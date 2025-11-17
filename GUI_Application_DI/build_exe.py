"""
******************************************************************************
@file           : build_exe.py
@brief          : Build script to create executable
******************************************************************************
@attention

Creates standalone executable using PyInstaller

Usage: python build_exe.py

******************************************************************************
"""

import PyInstaller.__main__
import os
import sys
from version import APP_NAME, get_version_string

def build_executable():
    """Build executable using PyInstaller"""
    
    print(f"Building {APP_NAME} executable...")
    print(f"Version: {get_version_string()}")
    
    # PyInstaller arguments
    args = [
        'main_gui.py',                  # Main script
        '--name=DigitalIN_Controller',  # Executable name
        '--onefile',                    # Single file
        '--windowed',                   # No console window
        '--clean',                      # Clean cache
        f'--icon=NONE',                 # Add icon if available
        '--add-data=version.py;.',      # Include version
        '--add-data=rs485_protocol.py;.',  # Include protocol
        '--hidden-import=serial.tools',
        '--hidden-import=serial.tools.list_ports',
    ]
    
    # Run PyInstaller
    try:
        PyInstaller.__main__.run(args)
        print("\n" + "="*60)
        print("Build completed successfully!")
        print("="*60)
        print(f"\nExecutable location: dist/DigitalIN_Controller.exe")
        print("\nYou can distribute the executable to users.")
        print("No Python installation required on target machine.")
        
    except Exception as e:
        print(f"\nBuild failed: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    build_executable()


