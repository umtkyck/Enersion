"""
Version information for Analog Input Controller GUI
"""

# Application version
APP_VERSION_MAJOR = 1
APP_VERSION_MINOR = 0
APP_VERSION_PATCH = 0
APP_BUILD_NUMBER = 1

# Application information
APP_NAME = "Analog Input Controller"
APP_DESCRIPTION = "RS485 GUI for monitoring 4-20mA, 0-10V, and NTC analog inputs"
APP_COMPANY = "Enersion"

# Compatibility aliases (for main_gui.py)
VERSION_MAJOR = APP_VERSION_MAJOR
VERSION_MINOR = APP_VERSION_MINOR
VERSION_PATCH = APP_VERSION_PATCH
VERSION_BUILD = APP_BUILD_NUMBER
VERSION_NAME = APP_NAME

def get_version_string():
    """Get formatted version string"""
    return f"{APP_NAME} v{APP_VERSION_MAJOR}.{APP_VERSION_MINOR}.{APP_VERSION_PATCH} Build:{APP_BUILD_NUMBER}"

def get_version_number():
    """Get version as tuple"""
    return (APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH, APP_BUILD_NUMBER)

if __name__ == "__main__":
    print(get_version_string())

