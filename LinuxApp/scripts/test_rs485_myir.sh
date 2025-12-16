#!/bin/bash
# ==============================================================================
# RS485 Test Script for MYIR STM32MP257
# ==============================================================================
#
# Usage: ./test_rs485_myir.sh [rx|tx]
#
# RS485 Configuration:
#   Device: /dev/ttySTM9
#   GPIO: PI10 (138) for direction control
#   Baud: 115200
#
# ==============================================================================

set -e

GPIO_NUM=138
GPIO_PATH="/sys/class/gpio/PI10"
DEVICE="/dev/ttySTM9"
BAUD=115200

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN} MYIR STM32MP257 RS485 Test${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""
echo "Device: $DEVICE"
echo "GPIO: PI10 ($GPIO_NUM)"
echo "Baud: $BAUD"
echo ""

# Initialize GPIO
init_gpio() {
    echo -e "${YELLOW}Initializing GPIO...${NC}"
    
    # Export GPIO if not already
    if [ ! -d "$GPIO_PATH" ]; then
        echo $GPIO_NUM > /sys/class/gpio/export
        sleep 0.1
    fi
    
    # Set as output
    echo out > $GPIO_PATH/direction
    
    echo -e "${GREEN}GPIO initialized${NC}"
}

# Set RX mode (GPIO LOW)
rx_mode() {
    echo 0 > $GPIO_PATH/value
    echo -e "${GREEN}RS485 RX mode enabled${NC}"
}

# Set TX mode (GPIO HIGH)
tx_mode() {
    echo 1 > $GPIO_PATH/value
    echo -e "${GREEN}RS485 TX mode enabled${NC}"
}

# Test receive
test_rx() {
    echo -e "${YELLOW}Starting RX test...${NC}"
    echo "Waiting for data from PC..."
    echo "Press Ctrl+C to stop"
    echo ""
    
    rx_mode
    
    # Use uart_test if available, otherwise use cat
    if command -v uart_test &> /dev/null; then
        uart_test -d $DEVICE -b $BAUD -m 0
    else
        stty -F $DEVICE $BAUD raw -echo
        cat $DEVICE
    fi
}

# Test transmit
test_tx() {
    echo -e "${YELLOW}Starting TX test...${NC}"
    echo "Sending test data..."
    echo ""
    
    tx_mode
    
    # Use uart_test if available
    if command -v uart_test &> /dev/null; then
        uart_test -d $DEVICE -b $BAUD -m 1
    else
        stty -F $DEVICE $BAUD raw -echo
        for i in {1..5}; do
            echo "ENERSION_TEST_$i" > $DEVICE
            echo "Sent: ENERSION_TEST_$i"
            sleep 0.5
        done
    fi
    
    rx_mode
}

# Cleanup
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"
    rx_mode
    echo -e "${GREEN}Done${NC}"
}

trap cleanup EXIT

# Main
init_gpio

case "${1:-help}" in
    rx)
        test_rx
        ;;
    tx)
        test_tx
        ;;
    *)
        echo "Usage: $0 [rx|tx]"
        echo ""
        echo "  rx - Start receiving data"
        echo "  tx - Send test data"
        echo ""
        echo "Connect RS485 A/B to PC via USB-RS485 converter"
        echo "Use SSCOM or similar terminal on PC at 115200 baud"
        ;;
esac



