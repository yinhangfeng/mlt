set(GDK_DIR ${SRC_DIR}/modules/gdk)

add_library(mltgdk MODULE
  ${GDK_DIR}/factory.c
  ${GDK_DIR}/filter_rescale.c
  ${GDK_DIR}/pixops.c
  ${GDK_DIR}/producer_pixbuf.c
)

target_compile_options(mltgdk PRIVATE ${MLT_COMPILE_OPTIONS})

pkg_check_modules(GdkPixbuf IMPORTED_TARGET gdk-pixbuf-2.0)

target_link_libraries(mltgdk PRIVATE mlt m Threads::Threads PkgConfig::GdkPixbuf)

target_compile_definitions(mltgdk PRIVATE USE_PIXBUF)

# if(TARGET PkgConfig::libexif)
#   target_link_libraries(mltgdk PRIVATE PkgConfig::libexif)
#   target_compile_definitions(mltgdk PRIVATE USE_EXIF)
# endif()

# if(TARGET PkgConfig::pango AND TARGET PkgConfig::fontconfig)
#   target_sources(mltgdk PRIVATE producer_pango.c)
#   target_link_libraries(mltgdk PRIVATE PkgConfig::pango PkgConfig::fontconfig PkgConfig::pangoft2)
#   if(APPLE)
#     target_link_libraries(mltgdk PRIVATE iconv)
#   endif()
#   target_compile_definitions(mltgdk PRIVATE USE_PANGO)
#   install(FILES producer_pango.yml DESTINATION ${MLT_INSTALL_DATA_DIR}/gdk)
# endif()

if(CPU_MMX)
  target_sources(mltgdk PRIVATE have_mmx.S scale_line_22_yuv_mmx.S)
  target_compile_definitions(mltgdk PRIVATE USE_MMX)
endif()

if(CPU_X86_64)
  target_compile_definitions(mltgdk PRIVATE ARCH_X86_64)
endif()

set_target_properties(mltgdk PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

# install(TARGETS mltgdk LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

# install(FILES filter_rescale.yml producer_pixbuf.yml DESTINATION ${MLT_INSTALL_DATA_DIR}/gdk)

if(NOT EXISTS "${MLT_DATA_OUTPUT_DIRECTORY}/avformat")
  file(COPY
    ${GDK_DIR}/filter_rescale.yml
    ${GDK_DIR}/producer_pixbuf.yml
    DESTINATION "${MLT_DATA_OUTPUT_DIRECTORY}/gdk"
  )
endif()
