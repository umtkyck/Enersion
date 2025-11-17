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
APP_VERSION_MINOR = 1
APP_VERSION_PATCH = 0
APP_BUILD_NUMBER = 2

# Version aliases for compatibility
VERSION_MAJOR = APP_VERSION_MAJOR
VERSION_MINOR = APP_VERSION_MINOR
VERSION_PATCH = APP_VERSION_PATCH
VERSION_BUILD = APP_BUILD_NUMBER

# Application Information
APP_NAME = "Digital OUT Controller"
APP_COMPANY = "Enersion"
APP_DESCRIPTION = "RS485 Digital Output Controller (56 Channels)"

# Name alias for compatibility
VERSION_NAME = APP_NAME

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


