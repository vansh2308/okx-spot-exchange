# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Find Qt6 packages
find_package(Qt6 COMPONENTS Core Widgets Test REQUIRED)

set(TEST_SOURCES
    bridge_test.cpp
    input_panel_test.cpp
    websocket_test.cpp
    models_test.cpp
)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    core
    models
    websocket
    ui
)

add_test(NAME run_tests COMMAND run_tests) 