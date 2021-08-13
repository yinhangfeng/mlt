set(FRAMEWORK_DIR ${SRC_DIR}/framework)

set(MLT_PUPLIC_HEADERS
  ${FRAMEWORK_DIR}/mlt.h
  ${FRAMEWORK_DIR}/mlt_animation.h
  ${FRAMEWORK_DIR}/mlt_audio.h
  ${FRAMEWORK_DIR}/mlt_cache.h
  ${FRAMEWORK_DIR}/mlt_chain.h
  ${FRAMEWORK_DIR}/mlt_consumer.h
  ${FRAMEWORK_DIR}/mlt_deque.h
  ${FRAMEWORK_DIR}/mlt_events.h
  ${FRAMEWORK_DIR}/mlt_factory.h
  ${FRAMEWORK_DIR}/mlt_field.h
  ${FRAMEWORK_DIR}/mlt_filter.h
  ${FRAMEWORK_DIR}/mlt_frame.h
  ${FRAMEWORK_DIR}/mlt_image.h
  ${FRAMEWORK_DIR}/mlt_link.h
  ${FRAMEWORK_DIR}/mlt_log.h
  ${FRAMEWORK_DIR}/mlt_luma_map.h
  ${FRAMEWORK_DIR}/mlt_multitrack.h
  ${FRAMEWORK_DIR}/mlt_parser.h
  ${FRAMEWORK_DIR}/mlt_playlist.h
  ${FRAMEWORK_DIR}/mlt_pool.h
  ${FRAMEWORK_DIR}/mlt_producer.h
  ${FRAMEWORK_DIR}/mlt_profile.h
  ${FRAMEWORK_DIR}/mlt_properties.h
  ${FRAMEWORK_DIR}/mlt_property.h
  ${FRAMEWORK_DIR}/mlt_repository.h
  ${FRAMEWORK_DIR}/mlt_service.h
  ${FRAMEWORK_DIR}/mlt_slices.h
  ${FRAMEWORK_DIR}/mlt_tokeniser.h
  ${FRAMEWORK_DIR}/mlt_tractor.h
  ${FRAMEWORK_DIR}/mlt_transition.h
  ${FRAMEWORK_DIR}/mlt_types.h
  ${FRAMEWORK_DIR}/mlt_version.h
  ${FRAMEWORK_DIR}/mlt_factory_register.h
)

add_library(mlt STATIC
  ${FRAMEWORK_DIR}/mlt_animation.c
  ${FRAMEWORK_DIR}/mlt_audio.c
  ${FRAMEWORK_DIR}/mlt_cache.c
  ${FRAMEWORK_DIR}/mlt_chain.c
  ${FRAMEWORK_DIR}/mlt_consumer.c
  ${FRAMEWORK_DIR}/mlt_deque.c
  ${FRAMEWORK_DIR}/mlt_events.c
  ${FRAMEWORK_DIR}/mlt_factory.c
  ${FRAMEWORK_DIR}/mlt_field.c
  ${FRAMEWORK_DIR}/mlt_filter.c
  ${FRAMEWORK_DIR}/mlt_frame.c
  ${FRAMEWORK_DIR}/mlt_image.c
  ${FRAMEWORK_DIR}/mlt_link.c
  ${FRAMEWORK_DIR}/mlt_log.c
  ${FRAMEWORK_DIR}/mlt_luma_map.c
  ${FRAMEWORK_DIR}/mlt_multitrack.c
  ${FRAMEWORK_DIR}/mlt_parser.c
  ${FRAMEWORK_DIR}/mlt_playlist.c
  ${FRAMEWORK_DIR}/mlt_pool.c
  ${FRAMEWORK_DIR}/mlt_producer.c
  ${FRAMEWORK_DIR}/mlt_profile.c
  ${FRAMEWORK_DIR}/mlt_properties.c
  ${FRAMEWORK_DIR}/mlt_property.c
  ${FRAMEWORK_DIR}/mlt_repository.c
  ${FRAMEWORK_DIR}/mlt_service.c
  ${FRAMEWORK_DIR}/mlt_slices.c
  ${FRAMEWORK_DIR}/mlt_tokeniser.c
  ${FRAMEWORK_DIR}/mlt_tractor.c
  ${FRAMEWORK_DIR}/mlt_transition.c
  ${FRAMEWORK_DIR}/mlt_version.c
)

# add_library(Mlt${MLT_VERSION_MAJOR}::mlt ALIAS mlt)

target_compile_options(mlt PRIVATE ${MLT_COMPILE_OPTIONS})

target_link_libraries(mlt PRIVATE m Threads::Threads mltcore mltxml ${CMAKE_DL_LIBS})

target_include_directories(mlt PUBLIC
  $<BUILD_INTERFACE:${SRC_DIR}>
  # $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mlt-${MLT_VERSION_MAJOR}>
)

set_target_properties(mlt PROPERTIES
  VERSION ${MLT_VERSION}
  SOVERSION ${MLT_VERSION_MAJOR}
  OUTPUT_NAME mlt
  PUBLIC_HEADER "${MLT_PUPLIC_HEADERS}"
)

# if(WIN32)
#   if(MINGW)
#     install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libmlt-${MLT_VERSION_MAJOR}.dll"
#       DESTINATION ${CMAKE_INSTALL_LIBDIR}
#       RENAME libmlt.dll
#     )
#     target_link_options(mlt PRIVATE -Wl,--output-def,libmlt.def)
#     install(FILES "${CMAKE_BINARY_DIR}/libmlt.def" DESTINATION ${CMAKE_INSTALL_LIBDIR})
#   endif()
#   target_sources(mlt PRIVATE ../win32/win32.c ../win32/strptime.c)
#   target_link_libraries(mlt PRIVATE Iconv::Iconv)
#   if(NOT WINDOWS_DEPLOY)
#     target_compile_definitions(mlt PRIVATE NODEPLOY)
#   endif()
# endif()

# if(NOT (WIN32 OR (APPLE AND RELOCATABLE)))
#   if(APPLE)
#     target_compile_definitions(mlt PRIVATE PREFIX_DATA="${CMAKE_INSTALL_FULL_DATADIR}/mlt" PREFIX_LIB="${CMAKE_INSTALL_FULL_LIBDIR}/mlt")
#   else()
#     target_compile_definitions(mlt PRIVATE PREFIX_DATA="${CMAKE_INSTALL_FULL_DATADIR}/mlt-${MLT_VERSION_MAJOR}" PREFIX_LIB="${CMAKE_INSTALL_FULL_LIBDIR}/mlt-${MLT_VERSION_MAJOR}")
#     target_link_options(mlt PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/mlt.vers)
#     set_target_properties(mlt PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mlt.vers)
#   endif()
# elseif(APPLE AND RELOCATABLE)
#   target_compile_definitions(mlt PRIVATE RELOCATABLE)
# endif()

# TODO
target_compile_definitions(mlt PRIVATE PREFIX_DATA="${CMAKE_INSTALL_FULL_DATADIR}/mlt" PREFIX_LIB="${CMAKE_INSTALL_FULL_LIBDIR}/mlt")

# install(TARGETS mlt
#   EXPORT MltTargets
#   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mlt-${MLT_VERSION_MAJOR}/framework
# )

# install(FILES metaschema.yaml DESTINATION ${MLT_INSTALL_DATA_DIR})

# configure_file(mlt-framework.pc.in mlt-framework-${MLT_VERSION_MAJOR}.pc @ONLY)
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mlt-framework-${MLT_VERSION_MAJOR}.pc
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
#   COMPONENT Development
# )
