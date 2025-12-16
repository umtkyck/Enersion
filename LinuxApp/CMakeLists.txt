# ==============================================================================
# Enersion Control System - CMake Configuration
# Target: MYIR STM32MP257 (Linux/Qt)
# ==============================================================================

cmake_minimum_required(VERSION 3.16)

project(EnersionControlSystem
    VERSION 1.0.0
    DESCRIPTION "Industrial Digital I/O Control System"
    LANGUAGES C CXX
)

# ==============================================================================
# Build Options
# ==============================================================================

option(BUILD_GUI "Build Qt/QML GUI application" ON)
option(BUILD_CLI "Build command-line test application" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(MISRA_COMPLIANCE "Enable MISRA C:2012 warnings" ON)

# ==============================================================================
# Compiler Settings
# ==============================================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MISRA compliance flags
if(MISRA_COMPLIANCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wshadow")
endif()

# ==============================================================================
# HAL Library (C - MISRA Compliant)
# ==============================================================================

add_library(enersion_hal STATIC
    src/hal/rs485_serial.c
    src/hal/rs485_gpio.c
)

target_include_directories(enersion_hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)

# ==============================================================================
# Protocol Library (C - MISRA Compliant)
# ==============================================================================

add_library(enersion_protocol STATIC
    src/protocol/enersion_crc.c
    src/protocol/enersion_protocol.c
)

target_include_directories(enersion_protocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
)

target_link_libraries(enersion_protocol PUBLIC
    enersion_hal
)

# ==============================================================================
# CLI Test Application
# ==============================================================================

if(BUILD_CLI)
    add_executable(enersion_test
        src/test/enersion_test.c
    )
    
    target_link_libraries(enersion_test PRIVATE
        enersion_protocol
        enersion_hal
    )
    
    target_include_directories(enersion_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
        ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    )
endif()

# ==============================================================================
# Qt/QML GUI Application
# ==============================================================================

if(BUILD_GUI)
    # Find Qt packages
    find_package(Qt5 COMPONENTS Core Quick QuickControls2 Widgets QUIET)
    
    if(NOT Qt5_FOUND)
        find_package(Qt6 COMPONENTS Core Quick QuickControls2 Widgets QUIET)
        if(Qt6_FOUND)
            set(QT_VERSION 6)
        endif()
    else()
        set(QT_VERSION 5)
    endif()
    
    if(Qt5_FOUND OR Qt6_FOUND)
        message(STATUS "Qt${QT_VERSION} found - Building GUI application")
        
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        
        # QML resources
        set(QML_SOURCES
            src/ui/qml/main.qml
            src/ui/qml/Style.qml
            src/ui/qml/qmldir
            src/ui/qml/components/NavigationBar.qml
            src/ui/qml/components/TopBar.qml
            src/ui/qml/components/Card.qml
            src/ui/qml/components/DioChannel.qml
            src/ui/qml/components/DioGrid.qml
            src/ui/qml/components/PrimaryButton.qml
            src/ui/qml/pages/DashboardPage.qml
            src/ui/qml/pages/DigitalInputPage.qml
            src/ui/qml/pages/DigitalOutputPage.qml
            src/ui/qml/pages/SettingsPage.qml
        )
        
        # Create resource file
        set(QRC_FILE ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
        file(WRITE ${QRC_FILE} "<RCC>\n<qresource prefix=\"/\">\n")
        foreach(QML_FILE ${QML_SOURCES})
            get_filename_component(QML_NAME ${QML_FILE} NAME)
            file(APPEND ${QRC_FILE} "    <file alias=\"${QML_FILE}\">${CMAKE_CURRENT_SOURCE_DIR}/${QML_FILE}</file>\n")
        endforeach()
        file(APPEND ${QRC_FILE} "</qresource>\n</RCC>\n")
        
        # GUI executable
        add_executable(enersion_gui
            src/app/main.cpp
            src/app/app_controller.cpp
            src/app/models/digital_input_model.cpp
            src/app/models/digital_output_model.cpp
            ${QRC_FILE}
        )
        
        target_include_directories(enersion_gui PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/app
            ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
            ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
        )
        
        if(QT_VERSION EQUAL 6)
            target_link_libraries(enersion_gui PRIVATE
                Qt6::Core
                Qt6::Quick
                Qt6::QuickControls2
                Qt6::Widgets
                enersion_protocol
                enersion_hal
            )
        else()
            target_link_libraries(enersion_gui PRIVATE
                Qt5::Core
                Qt5::Quick
                Qt5::QuickControls2
                Qt5::Widgets
                enersion_protocol
                enersion_hal
            )
        endif()
        
        # Install rules
        install(TARGETS enersion_gui
            RUNTIME DESTINATION bin
        )
        
    else()
        message(WARNING "Qt not found - GUI application will not be built")
    endif()
endif()

# ==============================================================================
# Unit Tests
# ==============================================================================

if(BUILD_TESTS)
    enable_testing()
    
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        add_executable(test_protocol
            tests/test_protocol.cpp
        )
        
        target_link_libraries(test_protocol PRIVATE
            enersion_protocol
            enersion_hal
            GTest::gtest_main
        )
        
        add_test(NAME ProtocolTests COMMAND test_protocol)
    else()
        message(STATUS "GTest not found - skipping unit tests")
    endif()
endif()

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS enersion_hal enersion_protocol
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY src/hal/ DESTINATION include/enersion
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY src/protocol/ DESTINATION include/enersion
    FILES_MATCHING PATTERN "*.h"
)

# ==============================================================================
# Summary
# ==============================================================================

message(STATUS "")
message(STATUS "=== Enersion Control System Build Configuration ===")
message(STATUS "  Version:     ${PROJECT_VERSION}")
message(STATUS "  Build GUI:   ${BUILD_GUI}")
message(STATUS "  Build CLI:   ${BUILD_CLI}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  MISRA Mode:  ${MISRA_COMPLIANCE}")
message(STATUS "")
