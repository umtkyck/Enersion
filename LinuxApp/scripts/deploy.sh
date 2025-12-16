#!/bin/bash
# ==============================================================================
# Enersion Linux App - Deploy Script
# Target: STM32MP257 MYIR Board
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-arm"

# MYIR Board Configuration
TARGET_IP="${TARGET_IP:-192.168.0.10}"
TARGET_USER="${TARGET_USER:-root}"
TARGET_PASS="${TARGET_PASS:-123}"
TARGET_PATH="${TARGET_PATH:-/usr/bin}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN} Enersion Deploy Script${NC}"
echo -e "${CYAN}========================================${NC}"

# Parse arguments
RUN_AFTER=false
CHECK_ONLY=false

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
        --pass)
            TARGET_PASS="$2"
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
        --check)
            CHECK_ONLY=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --ip IP        Target IP address (default: 192.168.0.10)"
            echo "  --user USER    SSH user (default: root)"
            echo "  --pass PASS    SSH password (default: 123)"
            echo "  --path PATH    Installation path (default: /usr/bin)"
            echo "  --run          Run application after deploy"
            echo "  --check        Only check connection, don't deploy"
            echo "  --help         Show this help"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo -e "${YELLOW}Target: ${TARGET_USER}@${TARGET_IP}${NC}"
echo ""

# Check connection
echo -e "${GREEN}1. Checking network connectivity...${NC}"
if ping -c 2 "$TARGET_IP" > /dev/null 2>&1; then
    echo -e "   ${GREEN}✓ Host is reachable${NC}"
else
    echo -e "   ${RED}✗ Host is NOT reachable${NC}"
    echo ""
    echo "Troubleshooting:"
    echo "  - Check if MYIR board is powered on"
    echo "  - Verify network cable connection"
    echo "  - Ensure board and PC are on same network"
    exit 1
fi

# Check SSH
echo -e "${GREEN}2. Checking SSH connection...${NC}"
if command -v sshpass &> /dev/null; then
    if sshpass -p "$TARGET_PASS" ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no \
        "${TARGET_USER}@${TARGET_IP}" "echo 'SSH OK'" > /dev/null 2>&1; then
        echo -e "   ${GREEN}✓ SSH connection OK${NC}"
    else
        echo -e "   ${YELLOW}! SSH connection may require manual password${NC}"
    fi
else
    echo -e "   ${YELLOW}! sshpass not installed, will prompt for password${NC}"
fi

if [ "$CHECK_ONLY" = true ]; then
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN} Connection check complete!${NC}"
    echo -e "${GREEN}========================================${NC}"
    exit 0
fi

# Check build exists
BINARY="$BUILD_DIR/enersion_gui"
if [ ! -f "$BINARY" ]; then
    echo ""
    echo -e "${RED}Error: Binary not found at $BINARY${NC}"
    echo "Please run: ./scripts/build.sh --target stm32mp257"
    exit 1
fi

echo -e "${GREEN}3. Binary: $BINARY${NC}"

# Deploy
echo ""
echo -e "${GREEN}4. Deploying...${NC}"
if command -v sshpass &> /dev/null; then
    sshpass -p "$TARGET_PASS" scp -o StrictHostKeyChecking=no \
        "$BINARY" "${TARGET_USER}@${TARGET_IP}:${TARGET_PATH}/"
    sshpass -p "$TARGET_PASS" ssh -o StrictHostKeyChecking=no \
        "${TARGET_USER}@${TARGET_IP}" "chmod +x ${TARGET_PATH}/enersion_gui"
else
    echo "Enter password when prompted: $TARGET_PASS"
    scp -o StrictHostKeyChecking=no "$BINARY" "${TARGET_USER}@${TARGET_IP}:${TARGET_PATH}/"
    ssh -o StrictHostKeyChecking=no "${TARGET_USER}@${TARGET_IP}" "chmod +x ${TARGET_PATH}/enersion_gui"
fi

echo -e "${GREEN}   ✓ Deployment complete!${NC}"

# Run if requested
if [ "$RUN_AFTER" = true ]; then
    echo ""
    echo -e "${GREEN}5. Starting application...${NC}"
    if command -v sshpass &> /dev/null; then
        sshpass -p "$TARGET_PASS" ssh -o StrictHostKeyChecking=no \
            "${TARGET_USER}@${TARGET_IP}" "DISPLAY=:0 ${TARGET_PATH}/enersion_gui &"
    else
        ssh -o StrictHostKeyChecking=no "${TARGET_USER}@${TARGET_IP}" \
            "DISPLAY=:0 ${TARGET_PATH}/enersion_gui &"
    fi
    echo -e "   ${GREEN}✓ Application started${NC}"
fi

echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN} Deployment Successful!${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""
echo "To run manually on target:"
echo "  ssh ${TARGET_USER}@${TARGET_IP}  (password: ${TARGET_PASS})"
echo "  DISPLAY=:0 ${TARGET_PATH}/enersion_gui"
