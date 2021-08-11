set(XML_DIR ${SRC_DIR}/modules/xml)

add_library(mltxml STATIC
  ${XML_DIR}/common.c
  ${XML_DIR}/consumer_xml.c
  ${XML_DIR}/factory.c
  ${XML_DIR}/producer_xml.c
  ${XML_DIR}/gps_parser.c
  ${XML_DIR}/filter_gpstext.c
)

target_compile_options(mltxml PRIVATE ${MLT_COMPILE_OPTIONS})

# TODO
target_link_libraries(mltxml PRIVATE mlt m Threads::Threads PkgConfig::xml)

set_target_properties(mltxml PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

# install(TARGETS mltxml LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})

# install(FILES
#   consumer_xml.yml
#   producer_xml-nogl.yml
#   producer_xml-string.yml
#   producer_xml.yml
#   filter_gpstext.yml
#   mlt-xml.dtd
#   DESTINATION ${MLT_INSTALL_DATA_DIR}/xml
# )
