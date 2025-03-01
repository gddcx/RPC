set(SRC_NET ${ROOT_DIR}/src/utils)

aux_source_directory(${SRC_NET} SRC)

add_library(utils STATIC ${SRC})