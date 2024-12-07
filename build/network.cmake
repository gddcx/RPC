set(SRC_NET ${ROOT_DIR}/src/network)

aux_source_directory(${SRC_NET} SRC)

add_library(network STATIC ${SRC})