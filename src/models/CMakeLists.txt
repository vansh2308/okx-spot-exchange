set(MODELS_SOURCES
    simulator.cpp
    almgren_chriss.cpp
    slippage_model.cpp
    fee_model.cpp
    maker_taker_model.cpp
    regression_model.cpp
)

set(MODELS_HEADERS
    ${CMAKE_SOURCE_DIR}/include/models/simulator.h
    ${CMAKE_SOURCE_DIR}/include/models/almgren_chriss.h
    ${CMAKE_SOURCE_DIR}/include/models/slippage_model.h
    ${CMAKE_SOURCE_DIR}/include/models/fee_model.h
    ${CMAKE_SOURCE_DIR}/include/models/maker_taker_model.h
    ${CMAKE_SOURCE_DIR}/include/models/regression_model.h
)

add_library(models STATIC ${MODELS_SOURCES} ${MODELS_HEADERS})

target_include_directories(models PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(models
    PRIVATE
    core
) 