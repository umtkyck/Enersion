"""
******************************************************************************
@file           : version.py
@brief          : GUI Application Version Management
******************************************************************************
@attention

All code and comments in English language

******************************************************************************
"""

# Application Version
APP_VERSION_MAJOR = 1
APP_VERSION_MINOR = 0
APP_VERSION_PATCH = 0
APP_BUILD_NUMBER = 1

# Application Information
APP_NAME = "PLC Controller GUI"
APP_COMPANY = "Enersion"
APP_DESCRIPTION = "RS485 PLC Controller Interface"

def get_version_string():
    """
    Get formatted version string
    
    Returns:
        str: Version string
    """
    return f"{APP_NAME} v{APP_VERSION_MAJOR}.{APP_VERSION_MINOR}.{APP_VERSION_PATCH}.{APP_BUILD_NUMBER}"

def get_version_number():
    """
    Get version as integer
    
    Returns:
        int: Version encoded as [Major:8][Minor:8][Patch:8][Build:8]
    """
    return (APP_VERSION_MAJOR << 24) | \
           (APP_VERSION_MINOR << 16) | \
           (APP_VERSION_PATCH << 8) | \
           APP_BUILD_NUMBER


