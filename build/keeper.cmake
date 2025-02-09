set(SRC_NET ${ROOT_DIR}/src/keeper)

aux_source_directory(${SRC_NET} SRC)

add_library(keeper STATIC ${SRC})