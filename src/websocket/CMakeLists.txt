set(WEBSOCKET_SOURCES
    websocket_client.cpp
    message_processor.cpp
)

set(WEBSOCKET_HEADERS
    ${CMAKE_SOURCE_DIR}/include/websocket/websocket_client.h
    ${CMAKE_SOURCE_DIR}/include/websocket/message_processor.h
)

add_library(websocket STATIC ${WEBSOCKET_SOURCES} ${WEBSOCKET_HEADERS})

target_include_directories(websocket PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(websocket
    PRIVATE
    Qt6::Core
    Qt6::Network
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
) 