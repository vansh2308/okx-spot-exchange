cmake_minimum_required(VERSION 3.10)
project(crypto_spot_exchange)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt6 path for macOS
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
endif()

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets Network Charts REQUIRED)

# Find other dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${FMT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/third_party/concurrentqueue  # For concurrentqueue
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Charts_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/models)
add_subdirectory(src/ui)
add_subdirectory(src/websocket)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/main.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ui
    websocket
    models
    core
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    ${Boost_LIBRARIES}
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    fmt::fmt
    nlohmann_json::nlohmann_json
)

# Add tests
# enable_testing()
# add_subdirectory(tests) 