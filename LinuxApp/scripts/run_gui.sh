#!/bin/bash
# =============================================================================
# Enersion Control System - Run GUI Application
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
GUI_APP="$BUILD_DIR/enersion_gui"

# Check if built
if [ ! -f "$GUI_APP" ]; then
    echo "GUI application not found. Building..."
    "$SCRIPT_DIR/build_gui.sh"
fi

# Set Qt environment for embedded
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_HIDECURSOR=1
export QT_QPA_EGLFS_PHYSICAL_WIDTH=300
export QT_QPA_EGLFS_PHYSICAL_HEIGHT=200

# For development/testing with X11
if [ -n "$DISPLAY" ]; then
    export QT_QPA_PLATFORM=xcb
fi

# Run
echo "Starting Enersion Control System..."
exec "$GUI_APP" "$@"

