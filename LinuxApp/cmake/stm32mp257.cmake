# ==============================================================================
# STM32MP257 Cross-Compilation Toolchain File
# ==============================================================================
#
# Usage with STM32MP2 SDK:
#   source /opt/st/stm32mp2/environment-setup-cortexa35-ostl-linux
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/stm32mp257.cmake ..
#
# ==============================================================================

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross compiler (set by SDK environment)
if(DEFINED ENV{CC})
    set(CMAKE_C_COMPILER $ENV{CC})
endif()

if(DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
endif()

# Sysroot
if(DEFINED ENV{SDKTARGETSYSROOT})
    set(CMAKE_SYSROOT $ENV{SDKTARGETSYSROOT})
    set(CMAKE_FIND_ROOT_PATH $ENV{SDKTARGETSYSROOT})
endif()

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Target flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=cortex-a35")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a -mtune=cortex-a35")

# Qt6 configuration
if(DEFINED ENV{SDKTARGETSYSROOT})
    set(QT_HOST_PATH "/opt/st/stm32mp2/host")
    set(Qt6_DIR "$ENV{SDKTARGETSYSROOT}/usr/lib/cmake/Qt6")
endif()

message(STATUS "STM32MP257 Toolchain loaded")
message(STATUS "  Sysroot: ${CMAKE_SYSROOT}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")

