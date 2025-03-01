set(SRC_NET ${ROOT_DIR}/src/monitoring)

aux_source_directory(${SRC_NET} SRC)

add_library(monitor STATIC ${SRC})