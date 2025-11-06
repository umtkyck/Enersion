@echo off
REM ******************************************************************************
REM @file           : run.bat
REM @brief          : Quick launch script for GUI application
REM ******************************************************************************

echo Starting PLC Controller GUI...
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Error: Python not found!
    echo Please install Python 3.8 or higher
    pause
    exit /b 1
)

REM Check if dependencies are installed
python -c "import PyQt5" >nul 2>&1
if errorlevel 1 (
    echo Installing dependencies...
    python -m pip install -r requirements.txt
    if errorlevel 1 (
        echo Error: Failed to install dependencies
        pause
        exit /b 1
    )
)

REM Run the application
python main_gui.py

pause


