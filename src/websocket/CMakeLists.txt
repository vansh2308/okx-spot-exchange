set(WEBSOCKET_SOURCES
    websocket_client.cpp
    message_processor.cpp
)

set(WEBSOCKET_HEADERS
    ${CMAKE_SOURCE_DIR}/include/websocket/websocket_client.h
    ${CMAKE_SOURCE_DIR}/include/websocket/message_processor.h
)

# add_library(websocket STATIC ${WEBSOCKET_SOURCES} ${WEBSOCKET_HEADERS} ${CMAKE_SOURCE_DIR}/src/websocket/websocket_client.moc)
add_library(websocket STATIC ${WEBSOCKET_SOURCES} ${WEBSOCKET_HEADERS})

target_include_directories(websocket
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(websocket
    PUBLIC
    core
    # Qt6::Core
    # Qt6::Network
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    nlohmann_json::nlohmann_json
) 