set(SRC_NET ${ROOT_DIR}/src/protocol)

aux_source_directory(${SRC_NET} SRC)

add_library(protocol STATIC ${SRC})