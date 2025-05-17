set(CORE_SOURCES
    logger.cpp
    config.cpp
    utils.cpp
    orderbook.cpp
)

set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/core/logger.h
    ${CMAKE_SOURCE_DIR}/include/core/config.h
    ${CMAKE_SOURCE_DIR}/include/core/utils.h
    ${CMAKE_SOURCE_DIR}/include/core/orderbook.h
)

add_library(core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(core
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(core
    PUBLIC
    nlohmann_json::nlohmann_json
) 