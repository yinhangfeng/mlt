set(AVFORMAT_DIR ${SRC_DIR}/modules/avformat)

add_library(mltavformat STATIC
  ${AVFORMAT_DIR}/common.c
  ${AVFORMAT_DIR}/factory.c
  ${AVFORMAT_DIR}/filter_avcolour_space.c
  ${AVFORMAT_DIR}/filter_avdeinterlace.c
  ${AVFORMAT_DIR}/filter_swscale.c
)

target_compile_options(mltavformat PRIVATE ${MLT_COMPILE_OPTIONS})

pkg_check_modules(libavformat IMPORTED_TARGET libavformat)
pkg_check_modules(libswscale IMPORTED_TARGET libswscale)
pkg_check_modules(libavutil IMPORTED_TARGET libavutil)
pkg_check_modules(libavcodec IMPORTED_TARGET libavcodec)
pkg_check_modules(libavfilter IMPORTED_TARGET libavfilter)
pkg_check_modules(libavdevice IMPORTED_TARGET libavdevice)
pkg_check_modules(libswresample IMPORTED_TARGET libswresample)

target_link_libraries(mltavformat PRIVATE
  mlt
  m
  Threads::Threads
  PkgConfig::libavformat
  PkgConfig::libswscale
  PkgConfig::libavutil
)

target_compile_definitions(mltavformat PRIVATE FILTERS)

# if(WIN32)
#   target_compile_definitions(mltavformat PRIVATE AVDATADIR="share/ffmpeg/")
# endif()

if(TARGET PkgConfig::libavcodec)
  target_sources(mltavformat PRIVATE ${AVFORMAT_DIR}/producer_avformat.c ${AVFORMAT_DIR}/consumer_avformat.c)
  target_link_libraries(mltavformat PRIVATE PkgConfig::libavcodec)
  target_compile_definitions(mltavformat PRIVATE CODECS)
endif()

if(TARGET PkgConfig::libavfilter)
  target_sources(mltavformat PRIVATE ${AVFORMAT_DIR}/filter_avfilter.c)
  target_link_libraries(mltavformat PRIVATE PkgConfig::libavfilter)
  target_compile_definitions(mltavformat PRIVATE AVFILTER)
endif()

if(TARGET PkgConfig::libavdevice)
  target_link_libraries(mltavformat PRIVATE PkgConfig::libavdevice)
  target_compile_definitions(mltavformat PRIVATE AVDEVICE)
endif()

if(TARGET PkgConfig::libswresample)
  target_sources(mltavformat PRIVATE ${AVFORMAT_DIR}/filter_swresample.c)
  target_link_libraries(mltavformat PRIVATE PkgConfig::libswresample)
  target_compile_definitions(mltavformat PRIVATE SWRESAMPLE)
endif()

if(CPU_MMX)
  target_compile_definitions(mltavformat PRIVATE USE_MMX)
endif()

set_target_properties(mltavformat PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

# install(TARGETS mltavformat LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

# install(FILES
#   consumer_avformat.yml
#   producer_avformat.yml
#   resolution_scale.yml
#   blacklist.txt
#   yuv_only.txt
#   DESTINATION ${MLT_INSTALL_DATA_DIR}/avformat
# )

if(NOT EXISTS "${MLT_DATA_OUTPUT_DIRECTORY}/avformat")
  file(COPY
    ${AVFORMAT_DIR}/consumer_avformat.yml
    ${AVFORMAT_DIR}/producer_avformat.yml
    ${AVFORMAT_DIR}/resolution_scale.yml
    ${AVFORMAT_DIR}/blacklist.txt
    ${AVFORMAT_DIR}/yuv_only.txt
    DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}/avformat"
  )
endif()
