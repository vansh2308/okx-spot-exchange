# Set source files
set(MODELS_SOURCES
    simulator.cpp
    slippage_model.cpp
    maker_taker_model.cpp
    almgren_chriss.cpp
    fee_model.cpp
)

# Create library
add_library(models STATIC ${MODELS_SOURCES})

# Set include directories
target_include_directories(models
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(models
    PUBLIC
        core
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ${Boost_LIBRARIES}
) 