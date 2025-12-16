#!/bin/bash
# =============================================================================
# Enersion Control System - Build Script for MYIR STM32MP257
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "=============================================="
echo " Enersion Control System - Build"
echo "=============================================="
echo ""
echo "Project: $PROJECT_DIR"
echo "Build:   $BUILD_DIR"
echo ""

# Parse arguments
BUILD_TYPE="Release"
BUILD_GUI=ON
BUILD_CLI=ON

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --no-gui)
            BUILD_GUI=OFF
            shift
            ;;
        --cli-only)
            BUILD_GUI=OFF
            BUILD_CLI=ON
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            rm -rf "$BUILD_DIR"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "[1/3] Configuring..."
cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_GUI=$BUILD_GUI \
    -DBUILD_CLI=$BUILD_CLI \
    -DBUILD_TESTS=OFF \
    -DMISRA_COMPLIANCE=ON

# Build
echo ""
echo "[2/3] Building..."
cmake --build . --parallel $(nproc)

echo ""
echo "[3/3] Build complete!"
echo ""

# List outputs
echo "Built executables:"
if [ -f "$BUILD_DIR/enersion_test" ]; then
    echo "  - enersion_test (CLI)"
fi
if [ -f "$BUILD_DIR/enersion_gui" ]; then
    echo "  - enersion_gui (Qt/QML)"
fi

echo ""
echo "To run:"
echo "  cd $BUILD_DIR"
if [ -f "$BUILD_DIR/enersion_test" ]; then
    echo "  ./enersion_test    # CLI test application"
fi
if [ -f "$BUILD_DIR/enersion_gui" ]; then
    echo "  ./enersion_gui     # Qt/QML GUI application"
fi
echo ""

