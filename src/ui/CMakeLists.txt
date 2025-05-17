set(UI_SOURCES
    main_window.cpp
)

set(UI_HEADERS
    ${CMAKE_SOURCE_DIR}/include/ui/main_window.h
)

add_library(ui STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ui
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    websocket
) 