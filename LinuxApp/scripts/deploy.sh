#!/bin/bash
# ==============================================================================
# Enersion Linux App - Deploy Script
# Target: STM32MP257 MYIR Board
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-arm"

# Default target configuration
TARGET_IP="${TARGET_IP:-192.168.1.100}"
TARGET_USER="${TARGET_USER:-root}"
TARGET_PATH="${TARGET_PATH:-/usr/bin}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN} Enersion Deploy Script${NC}"
echo -e "${GREEN}========================================${NC}"

# Parse arguments
RUN_AFTER=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --ip)
            TARGET_IP="$2"
            shift 2
            ;;
        --user)
            TARGET_USER="$2"
            shift 2
            ;;
        --path)
            TARGET_PATH="$2"
            shift 2
            ;;
        --run)
            RUN_AFTER=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --ip IP        Target IP address (default: 192.168.1.100)"
            echo "  --user USER    SSH user (default: root)"
            echo "  --path PATH    Installation path (default: /usr/bin)"
            echo "  --run          Run application after deploy"
            echo "  --help         Show this help"
            echo ""
            echo "Environment variables:"
            echo "  TARGET_IP      Target IP address"
            echo "  TARGET_USER    SSH user"
            echo "  TARGET_PATH    Installation path"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check build exists
BINARY="$BUILD_DIR/enersion_gui"
if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: Binary not found at $BINARY${NC}"
    echo "Please run: ./scripts/build.sh --target stm32mp257"
    exit 1
fi

echo -e "${YELLOW}Target: ${TARGET_USER}@${TARGET_IP}${NC}"
echo -e "${YELLOW}Binary: $BINARY${NC}"

# Deploy
echo -e "${GREEN}Deploying...${NC}"
scp "$BINARY" "${TARGET_USER}@${TARGET_IP}:${TARGET_PATH}/"

# Set permissions
echo -e "${GREEN}Setting permissions...${NC}"
ssh "${TARGET_USER}@${TARGET_IP}" "chmod +x ${TARGET_PATH}/enersion_gui"

echo -e "${GREEN}Deployment complete!${NC}"

# Run if requested
if [ "$RUN_AFTER" = true ]; then
    echo -e "${GREEN}Starting application...${NC}"
    ssh "${TARGET_USER}@${TARGET_IP}" "DISPLAY=:0 ${TARGET_PATH}/enersion_gui &"
fi

echo ""
echo "To run manually on target:"
echo "  ssh ${TARGET_USER}@${TARGET_IP}"
echo "  DISPLAY=:0 ${TARGET_PATH}/enersion_gui"

