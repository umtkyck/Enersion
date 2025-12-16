#!/bin/bash
# =============================================================================
# Enersion Control System - Run Script
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=============================================="
echo " Enersion Control System"
echo " MYIR STM32MP257"
echo "=============================================="
echo ""

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 not found"
    exit 1
fi

# Check dependencies
if ! python3 -c "import serial" 2>/dev/null; then
    echo "Installing dependencies..."
    pip3 install -r requirements.txt
fi

# Run web application
echo "Starting web application..."
echo "Open browser: http://localhost:8080"
echo ""
exec python3 web_app.py "$@"

