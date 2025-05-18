set(WEBSOCKET_SOURCES
    websocket_client.cpp
    message_processor.cpp
)

set(WEBSOCKET_HEADERS
    ${CMAKE_SOURCE_DIR}/include/websocket/websocket_client.h
    ${CMAKE_SOURCE_DIR}/include/websocket/message_processor.h
    # ${CMAKE_SOURCE_DIR}/include/websocket/orderbook_bridge.h
)

# add_library(websocket STATIC ${WEBSOCKET_SOURCES} ${WEBSOCKET_HEADERS} ${CMAKE_SOURCE_DIR}/src/websocket/websocket_client.moc)
add_library(websocket STATIC ${WEBSOCKET_SOURCES} ${WEBSOCKET_HEADERS})

target_include_directories(websocket
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${Boost_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(websocket
    PUBLIC
    core
    Qt6::Core
    # Qt6::Network
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

set_target_properties(websocket PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
) 