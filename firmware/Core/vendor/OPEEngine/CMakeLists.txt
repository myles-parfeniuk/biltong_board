add_library(OPEEngine INTERFACE)

target_include_directories(OPEEngine INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/callback
    ${CMAKE_CURRENT_SOURCE_DIR}/pool
)
