@echo off
REM Enersion Control System - Windows Run Script
echo ==============================================
echo  Enersion Control System
echo ==============================================
echo.

cd /d "%~dp0"

REM Check Python
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found
    pause
    exit /b 1
)

REM Install dependencies if needed
pip show pyserial >nul 2>&1
if errorlevel 1 (
    echo Installing dependencies...
    pip install -r requirements.txt
)

REM Run application
echo Starting application...
python app.py %*

pause

