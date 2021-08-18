cmake_minimum_required(VERSION 3.14)

project(MLT
  VERSION 7.1.0
  DESCRIPTION "Multimedia Framework"
  HOMEPAGE_URL "https://www.mltframework.org"
  LANGUAGES C CXX
)

option(MOD_AVFORMAT "Enable avformat module" ON)
option(MOD_XML "Enable XML module" ON)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR}/..)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/../src)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/../build)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/wasm")
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/${CMAKE_INSTALL_BINDIR}")
endif()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib")
set(MLT_MODULE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/lib/mlt")
set(MLT_DATA_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/share/mlt")

# 拷贝 headers presets profiles
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

# 拷贝 presets profiles
if(NOT EXISTS ${MLT_DATA_OUTPUT_DIRECTORY})
  # file(COPY "${SRC_DIR}/modules/core" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}/core" FILES_MATCHING REGEX yml|txt)
  # file(REMOVE "${MLT_DATA_OUTPUT_DIRECTORY}/core/CMakeLists.txt")
  # file(COPY "${SRC_DIR}/modules/avformat" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}/avformat" FILES_MATCHING REGEX yml|txt)
  # file(REMOVE "${MLT_DATA_OUTPUT_DIRECTORY}/avformat/CMakeLists.txt")
  file(COPY "${PROJECT_DIR}/presets" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}")
  file(COPY "${PROJECT_DIR}/profiles" DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}")
endif()

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
  MELT_NOSDL
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  # include_directories(${BUILD_DIR}/include)
  # TODO PkgConfig 为什么不会自动添加 link_directories?
  link_directories(${BUILD_DIR}/lib ${PROJECT_DIR}/third_party/ffmpeg/build/lib)
  set(ENV{PKG_CONFIG_PATH} "${BUILD_DIR}/lib/pkgconfig:${PROJECT_DIR}/third_party/ffmpeg/build/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

  # 兼容 emsdk 编译需要定义的宏
  add_compile_definitions(
    HAVE_LOCALE_H
  )

  # -gsource-map -pthread 不能放在 target_link_options 中
  # https://emscripten.org/docs/porting/Debugging.html#debug-information
  # --embed-file=${MLT_INSTALL_DATA_DIR}@/mlt_data
  set(EMSCRIPTEN_CFLAGS "-pthread -gsource-map")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_CFLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_CFLAGS}")
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
  # https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
  target_link_options(melt PRIVATE
    -sMODULARIZE=1
    -sEXPORT_NAME=createMelt
    -sINVOKE_RUN=0 # 关闭默认运行 main() 函数
    # -sEXIT_RUNTIME=1
    # -sEXPORTED_FUNCTIONS=[]  # 导出函数
    -sEXPORTED_RUNTIME_METHODS=[FS,cwrap,ccall,setValue,writeAsciiToMemory]
    -sPROXY_TO_PTHREAD=1 # 在 worker 中运行 main 函数 https://github.com/emscripten-core/emscripten/blob/main/system/lib/pthread/emscripten_proxy_main.c
    -sPTHREAD_POOL_SIZE=4 # 初始线程池大小 navigator.hardwareConcurrency 表示根据机器cpu核心数
    -sINITIAL_MEMORY=2146435072 # 1024 * 1024 * 2047 = 2146435072 bytes ~= 2 GB
    # -sALLOW_MEMORY_GROWTH=1 # 允许内存扩大
    -sLLD_REPORT_UNDEFINED
    --embed-file=${MLT_DATA_OUTPUT_DIRECTORY}@/mlt_data
    # --source-map-base=./ # 控制 sourcemap 相对于 .wasm 文件的相对路径
  )
endif()
