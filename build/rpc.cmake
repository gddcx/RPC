set(SRC_NET ${ROOT_DIR}/src/rpc)

aux_source_directory(${SRC_NET} SRC)

add_library(rpc STATIC ${SRC})