set(UI_SOURCES
    # main_window.cpp
    # orderbook_widget.cpp
    orderbook_table_model.cpp 
    view.cpp
    bridge.cpp
    simulation_panel.cpp
)

set(UI_HEADERS
    # ${CMAKE_SOURCE_DIR}/include/ui/main_window.h
    # ${CMAKE_SOURCE_DIR}/include/ui/orderbook_widget.h
    ${CMAKE_SOURCE_DIR}/include/ui/orderbook_table_model.h
    ${CMAKE_SOURCE_DIR}/include/ui/view.h
    ${CMAKE_SOURCE_DIR}/include/ui/bridge.h
    ${CMAKE_SOURCE_DIR}/include/ui/simulation_panel.h

)

add_library(ui STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(ui
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
)

target_link_libraries(ui
    PUBLIC
        core
        websocket
        models
        Qt6::Core
        Qt6::Widgets
)

set_target_properties(ui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
) 