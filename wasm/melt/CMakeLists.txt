set(MELT_DIR ${SRC_DIR}/melt)

add_executable(melt ${MELT_DIR}/melt.c ${MELT_DIR}/io.c)

target_compile_options(melt PRIVATE ${MLT_COMPILE_OPTIONS})

target_link_libraries(melt PRIVATE mlt Threads::Threads)

target_compile_definitions(melt PRIVATE VERSION="${MLT_VERSION}")
