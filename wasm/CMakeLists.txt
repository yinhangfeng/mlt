cmake_minimum_required(VERSION 3.14)

project(MLT
  VERSION 7.1.0
  DESCRIPTION "Multimedia Framework"
  HOMEPAGE_URL "https://www.mltframework.org"
  LANGUAGES C CXX
)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/..)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/../src)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/../build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib")
set(MLT_MODULE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib/mlt")
set(MLT_DATA_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/share/mlt")

# TODO 拷贝 headers presets profiles
# if(NOT EXISTS ${MLT_DATA_OUTPUT_DIRECTORY})
#   if(WIN32) # symlinks require admin rights on Windows
#     file(COPY "${CMAKE_SOURCE_DIR}/src/modules" DESTINATION "${CMAKE_BINARY_DIR}/out/share" FILES_MATCHING REGEX yml|txt)
#     file(RENAME "${CMAKE_BINARY_DIR}/out/share/modules" "${MLT_DATA_OUTPUT_DIRECTORY}")
#     file(COPY "${CMAKE_SOURCE_DIR}/presets" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}")
#     file(COPY "${CMAKE_SOURCE_DIR}/profiles" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}")
#   else()
#     file(MAKE_DIRECTORY "${MLT_DATA_OUTPUT_DIRECTORY}")
#     file(GLOB MOD_SUBDIRS "${CMAKE_SOURCE_DIR}/src/modules/*")
#     foreach(MOD_SUBDIR ${MOD_SUBDIRS})
#       file(RELATIVE_PATH MOD_NAME "${CMAKE_SOURCE_DIR}/src/modules" ${MOD_SUBDIR})
#       file(CREATE_LINK "${CMAKE_SOURCE_DIR}/src/modules/${MOD_NAME}" "${MLT_DATA_OUTPUT_DIRECTORY}/${MOD_NAME}" SYMBOLIC)
#     endforeach()
#     file(CREATE_LINK "${CMAKE_SOURCE_DIR}/presets" "${MLT_DATA_OUTPUT_DIRECTORY}/presets" SYMBOLIC)
#     file(CREATE_LINK "${CMAKE_SOURCE_DIR}/profiles" "${MLT_DATA_OUTPUT_DIRECTORY}/profiles" SYMBOLIC)
#   endif()
# endif()

set(MLT_INSTALL_MODULE_DIR ${CMAKE_INSTALL_LIBDIR}/mlt)
set(MLT_INSTALL_DATA_DIR ${CMAKE_INSTALL_DATADIR}/mlt)
# if(NOT (WIN32 OR APPLE))
#   set(MLT_INSTALL_MODULE_DIR ${CMAKE_INSTALL_LIBDIR}/mlt-${MLT_VERSION_MAJOR})
#   set(MLT_INSTALL_DATA_DIR ${CMAKE_INSTALL_DATADIR}/mlt-${MLT_VERSION_MAJOR})
# endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  include_directories(${BUILD_DIR}/include)
  link_directories(${BUILD_DIR}/lib)

  # 兼容 emsdk 编译需要定义的宏
  add_compile_definitions(
    MELT_NOSDL
    HAVE_LOCALE_H
  )
else()
  add_compile_definitions(
    # 模拟 __EMSCRIPTEN__ 环境
    __EMSCRIPTEN__
  )
endif()

add_subdirectory(framework)
add_subdirectory(modules)
# 可执行文件模块直接使用 emcc 编译可以方便控制相关参数
add_subdirectory(melt)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  # https://emscripten.org/docs/getting_started/FAQ.html?highlight=cmake#how-do-i-specify-s-options-in-a-cmake-project
  # -sPROXY_TO_PTHREAD=1
  target_link_options(melt PRIVATE -sMODULARIZE=1 -sEXPORT_NAME=Melt -sEXPORTED_RUNTIME_METHODS=[FS,cwrap,ccall] -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency)
  # -gsource-map 不能放在 target_link_options 中
  # https://emscripten.org/docs/porting/Debugging.html#debug-information
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
