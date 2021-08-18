set(CORE_DIR ${SRC_DIR}/modules/core)

add_library(mltcore STATIC
  ${CORE_DIR}/consumer_multi.c
  ${CORE_DIR}/consumer_null.c
  ${CORE_DIR}/factory.c
  ${CORE_DIR}/filter_audiochannels.c
  ${CORE_DIR}/filter_audioconvert.c
  ${CORE_DIR}/filter_audiomap.c
  ${CORE_DIR}/filter_audiowave.c
  ${CORE_DIR}/filter_brightness.c
  ${CORE_DIR}/filter_channelcopy.c
  ${CORE_DIR}/filter_choppy.c
  ${CORE_DIR}/filter_crop.c
  ${CORE_DIR}/filter_fieldorder.c
  ${CORE_DIR}/filter_gamma.c
  ${CORE_DIR}/filter_greyscale.c
  ${CORE_DIR}/filter_imageconvert.c
  ${CORE_DIR}/filter_luma.c
  ${CORE_DIR}/filter_mask_apply.c
  ${CORE_DIR}/filter_mask_start.c
  ${CORE_DIR}/filter_mirror.c
  ${CORE_DIR}/filter_mono.c
  ${CORE_DIR}/filter_obscure.c
  ${CORE_DIR}/filter_panner.c
  ${CORE_DIR}/filter_rescale.c
  ${CORE_DIR}/filter_resize.c
  ${CORE_DIR}/filter_transition.c
  ${CORE_DIR}/filter_watermark.c
  ${CORE_DIR}/link_timeremap.c
  ${CORE_DIR}/producer_colour.c
  ${CORE_DIR}/producer_consumer.c
  ${CORE_DIR}/producer_hold.c
  ${CORE_DIR}/producer_loader.c
  ${CORE_DIR}/producer_melt.c
  ${CORE_DIR}/producer_noise.c
  ${CORE_DIR}/producer_timewarp.c
  ${CORE_DIR}/producer_tone.c
  ${CORE_DIR}/transition_composite.c
  ${CORE_DIR}/transition_luma.c
  ${CORE_DIR}/transition_matte.c
  ${CORE_DIR}/transition_mix.c
)

target_compile_options(mltcore PRIVATE ${MLT_COMPILE_OPTIONS})

target_link_libraries(mltcore PRIVATE m mlt Threads::Threads)

# if(WIN32)
#   target_sources(mltcore PRIVATE ../../win32/fnmatch.c)
#   target_include_directories(mltcore PRIVATE ../../win32)
# endif()

# if(CPU_SSE)
#   target_compile_definitions(mltcore PRIVATE USE_SSE)
# endif()

# if(CPU_X86_64)
#   target_sources(mltcore PRIVATE composite_line_yuv_sse2_simple.c)
#   target_compile_definitions(mltcore PRIVATE ARCH_X86_64)
# endif()

set_target_properties(mltcore PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

# install(TARGETS mltcore LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

# install(FILES
#   consumer_multi.yml
#   filter_audiomap.yml
#   filter_audiowave.yml
#   filter_brightness.yml
#   filter_channelcopy.yml
#   filter_choppy.yml
#   filter_crop.yml
#   filter_fieldorder.yml
#   filter_gamma.yml
#   filter_greyscale.yml
#   filter_luma.yml
#   filter_mask_apply.yml
#   filter_mask_start.yml
#   filter_mirror.yml
#   filter_mono.yml
#   filter_obscure.yml
#   filter_panner.yml
#   filter_rescale.yml
#   filter_resize.yml
#   filter_transition.yml
#   filter_watermark.yml
#   link_timeremap.yml
#   producer_colour.yml
#   producer_consumer.yml
#   producer_hold.yml
#   producer_loader.yml
#   producer_melt_file.yml
#   producer_melt.yml
#   producer_noise.yml
#   producer_timewarp.yml
#   producer_tone.yml
#   transition_composite.yml
#   transition_luma.yml
#   transition_matte.yml
#   transition_mix.yml
#   loader.dict
#   loader.ini
#   DESTINATION ${MLT_INSTALL_DATA_DIR}/core
# )

if(NOT EXISTS "${MLT_DATA_OUTPUT_DIRECTORY}/core")
  file(COPY
    ${CORE_DIR}/consumer_multi.yml
    ${CORE_DIR}/filter_audiomap.yml
    ${CORE_DIR}/filter_audiowave.yml
    ${CORE_DIR}/filter_brightness.yml
    ${CORE_DIR}/filter_channelcopy.yml
    ${CORE_DIR}/filter_choppy.yml
    ${CORE_DIR}/filter_crop.yml
    ${CORE_DIR}/filter_fieldorder.yml
    ${CORE_DIR}/filter_gamma.yml
    ${CORE_DIR}/filter_greyscale.yml
    ${CORE_DIR}/filter_luma.yml
    ${CORE_DIR}/filter_mask_apply.yml
    ${CORE_DIR}/filter_mask_start.yml
    ${CORE_DIR}/filter_mirror.yml
    ${CORE_DIR}/filter_mono.yml
    ${CORE_DIR}/filter_obscure.yml
    ${CORE_DIR}/filter_panner.yml
    ${CORE_DIR}/filter_rescale.yml
    ${CORE_DIR}/filter_resize.yml
    ${CORE_DIR}/filter_transition.yml
    ${CORE_DIR}/filter_watermark.yml
    ${CORE_DIR}/link_timeremap.yml
    ${CORE_DIR}/producer_colour.yml
    ${CORE_DIR}/producer_consumer.yml
    ${CORE_DIR}/producer_hold.yml
    ${CORE_DIR}/producer_loader.yml
    ${CORE_DIR}/producer_melt_file.yml
    ${CORE_DIR}/producer_melt.yml
    ${CORE_DIR}/producer_noise.yml
    ${CORE_DIR}/producer_timewarp.yml
    ${CORE_DIR}/producer_tone.yml
    ${CORE_DIR}/transition_composite.yml
    ${CORE_DIR}/transition_luma.yml
    ${CORE_DIR}/transition_matte.yml
    ${CORE_DIR}/transition_mix.yml
    ${CORE_DIR}/loader.dict
    ${CORE_DIR}/loader.ini
    DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}/core"
  )
endif()
