cmake_minimum_required(VERSION 3.15)
project(crypto_simulator VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automoc, autorcc, and autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenSSL REQUIRED)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/websocket)
add_subdirectory(src/models)
add_subdirectory(src/ui)

# Create executable
add_executable(crypto_simulator src/main.cpp)

# Link libraries
target_link_libraries(crypto_simulator
    PRIVATE
        core
        websocket
        models
        ui
        ${Boost_LIBRARIES}
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Qt6::Core
        Qt6::Widgets
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Include directories
target_include_directories(crypto_simulator
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party
        ${Boost_INCLUDE_DIRS}
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

# Add tests
# enable_testing()
# add_subdirectory(tests) 