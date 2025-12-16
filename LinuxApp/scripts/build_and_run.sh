#!/bin/bash
# Build and run Enersion test on MYIR board

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR/../src"
TEST_DIR="$SRC_DIR/test"

echo "=========================================="
echo " Building Enersion Test Application"
echo "=========================================="
echo ""

cd "$TEST_DIR"

# Compile
echo "Compiling..."
gcc -Wall -Wextra -O2 \
    -I../hal -I../protocol \
    -o enersion_test \
    enersion_test.c \
    ../hal/rs485_serial.c \
    ../hal/rs485_gpio.c \
    ../protocol/enersion_crc.c \
    ../protocol/enersion_protocol.c

echo ""
echo "[OK] Build successful!"
echo ""

# Run
echo "Starting application..."
echo ""
./enersion_test



