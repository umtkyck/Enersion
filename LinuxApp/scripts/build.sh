#!/bin/bash
# ==============================================================================
# Enersion Linux App - Build Script
# Target: Local development and STM32MP257 cross-compilation
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN} Enersion Linux App Build Script${NC}"
echo -e "${GREEN}========================================${NC}"

# Parse arguments
BUILD_TYPE="Release"
TARGET="local"
CLEAN=false
RUN_TESTS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --target)
            TARGET="$2"
            shift 2
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug        Build with debug symbols"
            echo "  --target TYPE  Target platform: local, stm32mp257"
            echo "  --clean        Clean build directory first"
            echo "  --test         Run unit tests after build"
            echo "  --help         Show this help"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Set build directory based on target
if [ "$TARGET" == "stm32mp257" ]; then
    BUILD_DIR="$PROJECT_DIR/build-arm"
    echo -e "${YELLOW}Cross-compiling for STM32MP257...${NC}"
    
    # Check for SDK environment
    if [ -z "$SDKTARGETSYSROOT" ]; then
        echo -e "${RED}Error: STM32MP2 SDK not sourced!${NC}"
        echo "Please run: source /opt/st/stm32mp2/environment-setup-cortexa35-ostl-linux"
        exit 1
    fi
else
    BUILD_DIR="$PROJECT_DIR/build"
    echo -e "${YELLOW}Building for local platform...${NC}"
fi

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring...${NC}"
if [ "$TARGET" == "stm32mp257" ]; then
    cmake "$PROJECT_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/cmake/stm32mp257.cmake"
else
    cmake "$PROJECT_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . --parallel $(nproc)

# Run tests if requested
if [ "$RUN_TESTS" = true ] && [ "$TARGET" == "local" ]; then
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN} Build Complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Binary location: $BUILD_DIR/enersion_gui"

if [ "$TARGET" == "stm32mp257" ]; then
    echo ""
    echo "To deploy to target:"
    echo "  scp $BUILD_DIR/enersion_gui root@<target-ip>:/usr/bin/"
fi

