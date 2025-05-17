cmake_minimum_required(VERSION 3.10)
project(crypto_spot_exchange)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt6 path for macOS
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
endif()

# Find Qt packages
# find_package(Qt5 COMPONENTS Core Widgets Network Charts REQUIRED)


# Find other dependencies
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${FMT_INCLUDE_DIRS}
    # ${Qt6Core_INCLUDE_DIRS}
    # ${Qt6Widgets_INCLUDE_DIRS}
    # ${Qt6Network_INCLUDE_DIRS}
    # ${Qt6Charts_INCLUDE_DIRS}
)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    # Qt6::Core
    # Qt6::Widgets
    # Qt6::Network
    # Qt6::Charts
    ${Boost_LIBRARIES}
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    fmt::fmt
)

# Add tests
# enable_testing()
# add_subdirectory(tests) 