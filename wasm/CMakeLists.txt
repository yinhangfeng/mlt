cmake_minimum_required(VERSION 3.14)

project(MLT
  VERSION 7.1.0
  DESCRIPTION "Multimedia Framework"
  HOMEPAGE_URL "https://www.mltframework.org"
  LANGUAGES C CXX
)

option(MOD_AVFORMAT "Enable avformat module" ON)
option(MOD_XML "Enable XML module" OFF)

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

list(APPEND MLT_COMPILE_OPTIONS "")

# MSVC cl doesn't support GNU inline assembly (but MSVC-compatible clang-cl does)
# if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#   message("xxxxx ${CMAKE_SYSTEM_PROCESSOR}")
#   if(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|x86|x86_64|AMD64")
#     set(CPU_MMX ON)
#     set(CPU_SSE ON)
#     set(CPU_SSE2 ON)
#     if(NOT MSVC) # also NOT clang-cl
#       list(APPEND MLT_COMPILE_OPTIONS "-mmmx;-msse;-msse2")
#     endif()
#   endif()
#   if(CMAKE_SYSTEM_PROCESSOR MATCHES "i686" OR (WIN32 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86"))
#     set(CPU_X86_32 ON)
#   endif()
#   if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
#     set(CPU_X86_64 ON)
#   endif()
# endif()

if(MSVC)
  list(APPEND MLT_COMPILE_OPTIONS "$<$<CONFIG:Release>:/fp:fast>")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  list(APPEND MLT_COMPILE_OPTIONS "$<$<CONFIG:Release>:-ffast-math>")
endif()

add_compile_definitions(
  # 静态依赖插件，不使用 dlopen
  STATIC_LINK_PLUGINS
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  include_directories(${BUILD_DIR}/include)
  link_directories(${BUILD_DIR}/lib)
  set(ENV{PKG_CONFIG_PATH} "${BUILD_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

  # 兼容 emsdk 编译需要定义的宏
  add_compile_definitions(
    MELT_NOSDL
    HAVE_LOCALE_H
  )

  # -gsource-map -pthread 不能放在 target_link_options 中
  # https://emscripten.org/docs/porting/Debugging.html#debug-information
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

if(MOD_AVFORMAT)
  add_compile_definitions(
    MOD_AVFORMAT_ENABLED
  )
endif()

if(MOD_XML)
  add_compile_definitions(
    MOD_XML_ENABLED
  )
endif()

add_subdirectory(framework)
add_subdirectory(modules)
add_subdirectory(melt)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  # https://emscripten.org/docs/getting_started/FAQ.html?highlight=cmake#how-do-i-specify-s-options-in-a-cmake-project
  # -sPROXY_TO_PTHREAD=1
  target_link_options(melt PRIVATE -sMODULARIZE=1 -sEXPORT_NAME=Melt -sEXPORTED_RUNTIME_METHODS=[FS,cwrap,ccall] -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency -sLLD_REPORT_UNDEFINED)
endif()
