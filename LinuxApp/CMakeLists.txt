# ==============================================================================
# Enersion Linux Application - Main CMake Configuration
# Target: STM32MP257 MYIR Board with HDMI Touchscreen
# ==============================================================================

cmake_minimum_required(VERSION 3.16)

project(EnersionGUI
    VERSION 1.0.0
    DESCRIPTION "Enersion Digital I/O Control Application"
    LANGUAGES C CXX
)

# ==============================================================================
# Build Options
# ==============================================================================
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_MISRA_CHECK "Enable MISRA C:2012 static analysis" OFF)

# ==============================================================================
# C/C++ Standards
# ==============================================================================
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Qt Configuration
# ==============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    SerialPort
)

# ==============================================================================
# Compiler Flags
# ==============================================================================
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -DDEBUG)
else()
    add_compile_options(-O2 -DNDEBUG)
endif()

# MISRA C compliance flags for C code
set(MISRA_C_FLAGS
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wcast-align
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wredundant-decls
    -Wnested-externs
    -Wshadow
    -Wundef
    -Wfloat-equal
    -Wswitch-default
    -Wswitch-enum
)

# ==============================================================================
# Source Files
# ==============================================================================

# HAL Layer (MISRA C compliant)
set(HAL_SOURCES
    src/hal/rs485_serial.c
)

# Protocol Layer (MISRA C compliant)
set(PROTOCOL_SOURCES
    src/protocol/enersion_crc.c
    src/protocol/enersion_protocol.c
)

# Service Layer (C++)
set(SERVICE_SOURCES
    src/service/device_manager.cpp
    src/service/di_service.cpp
    src/service/do_service.cpp
)

# Application Layer (C++)
set(APP_SOURCES
    src/app/main.cpp
    src/app/app_controller.cpp
    src/app/models/digital_input_model.cpp
    src/app/models/digital_output_model.cpp
    src/app/models/connection_model.cpp
)

# ==============================================================================
# HAL Library (C - MISRA compliant)
# ==============================================================================
add_library(enersion_hal STATIC ${HAL_SOURCES})
target_include_directories(enersion_hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)
target_compile_options(enersion_hal PRIVATE ${MISRA_C_FLAGS})

# ==============================================================================
# Protocol Library (C - MISRA compliant)
# ==============================================================================
add_library(enersion_protocol STATIC ${PROTOCOL_SOURCES})
target_include_directories(enersion_protocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)
target_link_libraries(enersion_protocol PRIVATE enersion_hal)
target_compile_options(enersion_protocol PRIVATE ${MISRA_C_FLAGS})

# ==============================================================================
# Service Library (C++)
# ==============================================================================
add_library(enersion_service STATIC ${SERVICE_SOURCES})
target_include_directories(enersion_service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)
target_link_libraries(enersion_service
    PUBLIC Qt6::Core
    PRIVATE enersion_protocol enersion_hal
)

# ==============================================================================
# Main Application
# ==============================================================================
qt_add_executable(enersion_gui ${APP_SOURCES})

target_include_directories(enersion_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)

target_link_libraries(enersion_gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
    enersion_service
    enersion_protocol
    enersion_hal
)

# QML Resources
qt_add_qml_module(enersion_gui
    URI EnersionApp
    VERSION 1.0
    QML_FILES
        src/ui/qml/main.qml
        src/ui/qml/pages/HomePage.qml
        src/ui/qml/pages/DigitalInputPage.qml
        src/ui/qml/pages/DigitalOutputPage.qml
        src/ui/qml/pages/SettingsPage.qml
        src/ui/qml/components/NavigationBar.qml
        src/ui/qml/components/StatusBar.qml
        src/ui/qml/components/ConnectionPanel.qml
        src/ui/qml/components/DigitalIOGrid.qml
        src/ui/qml/components/IOIndicator.qml
        src/ui/qml/components/ModernButton.qml
        src/ui/qml/components/DeviceStatusCard.qml
    RESOURCES
        src/ui/resources/fonts/Inter-Regular.ttf
        src/ui/resources/fonts/Inter-Bold.ttf
        src/ui/resources/icons/home.svg
        src/ui/resources/icons/input.svg
        src/ui/resources/icons/output.svg
        src/ui/resources/icons/settings.svg
        src/ui/resources/icons/connected.svg
        src/ui/resources/icons/disconnected.svg
)

# ==============================================================================
# Unit Tests
# ==============================================================================
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    # Protocol Tests
    add_executable(test_protocol
        tests/test_protocol.cpp
    )
    target_link_libraries(test_protocol PRIVATE
        Qt6::Test
        enersion_protocol
    )
    add_test(NAME ProtocolTests COMMAND test_protocol)
    
    # Service Tests
    add_executable(test_service
        tests/test_service.cpp
    )
    target_link_libraries(test_service PRIVATE
        Qt6::Test
        Qt6::Core
        enersion_service
    )
    add_test(NAME ServiceTests COMMAND test_service)
endif()

# ==============================================================================
# Installation
# ==============================================================================
install(TARGETS enersion_gui
    RUNTIME DESTINATION bin
)

# ==============================================================================
# Summary
# ==============================================================================
message(STATUS "")
message(STATUS "=== Enersion GUI Build Configuration ===")
message(STATUS "Version:       ${PROJECT_VERSION}")
message(STATUS "Build Type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt Version:    ${Qt6_VERSION}")
message(STATUS "Build Tests:   ${BUILD_TESTS}")
message(STATUS "=========================================")
message(STATUS "")

