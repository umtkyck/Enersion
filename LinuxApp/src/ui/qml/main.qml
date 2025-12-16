import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "."
import "components"
import "pages"

/**
 * Enersion Control System - Main Application Window
 * Modern Industrial UI for STM32MP257 MYIR Touchscreen
 */
ApplicationWindow {
    id: mainWindow
    
    visible: true
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 600
    
    title: qsTr("Enersion Control System")
    color: Style.bgDark
    
    // Current page index
    property int currentPage: 0
    
    // Connection state (bound to C++ backend)
    property bool isConnected: appController ? appController.isConnected : false
    property int diActiveCount: diModel ? diModel.activeCount : 0
    property int doActiveCount: doModel ? doModel.activeCount : 0
    
    // Page names for navigation
    readonly property var pageNames: [
        { name: "Dashboard", icon: "⌂" },
        { name: "Digital Inputs", icon: "▣" },
        { name: "Digital Outputs", icon: "◧" },
        { name: "Settings", icon: "⚙" }
    ]
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // === Left Navigation Bar ===
        NavigationBar {
            id: navBar
            Layout.fillHeight: true
            Layout.preferredWidth: Style.sidebarWidth
            
            currentIndex: currentPage
            pages: pageNames
            connected: isConnected
            
            onPageSelected: function(index) {
                currentPage = index
            }
        }
        
        // === Main Content Area ===
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Style.bgDark
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // === Top Bar ===
                TopBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Style.headerHeight
                    
                    title: pageNames[currentPage].name
                    connected: isConnected
                    diCount: diActiveCount
                    doCount: doActiveCount
                }
                
                // === Page Stack ===
                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: Style.spacingL
                    
                    currentIndex: currentPage
                    
                    // Page 0: Dashboard
                    DashboardPage {
                        connected: isConnected
                        diActiveCount: mainWindow.diActiveCount
                        doActiveCount: mainWindow.doActiveCount
                        
                        onNavigateTo: function(pageIndex) {
                            currentPage = pageIndex
                        }
                    }
                    
                    // Page 1: Digital Inputs
                    DigitalInputPage {
                        model: diModel
                    }
                    
                    // Page 2: Digital Outputs
                    DigitalOutputPage {
                        model: doModel
                        
                        onToggleOutput: function(channel) {
                            if (appController) {
                                appController.toggleOutput(channel)
                            }
                        }
                        
                        onSetAllOutputs: function(state) {
                            if (appController) {
                                appController.setAllOutputs(state)
                            }
                        }
                    }
                    
                    // Page 3: Settings
                    SettingsPage {
                        controller: appController
                        
                        onConnect: function(port, baudrate) {
                            if (appController) {
                                appController.connectDevice(port, baudrate)
                            }
                        }
                        
                        onDisconnect: {
                            if (appController) {
                                appController.disconnectDevice()
                            }
                        }
                    }
                }
            }
        }
    }
    
    // === Startup Animation ===
    Component.onCompleted: {
        console.log("Enersion Control System started")
        
        // Auto-connect on startup (optional)
        if (appController) {
            appController.autoConnect()
        }
    }
    
    // === Keyboard Shortcuts ===
    Shortcut {
        sequence: "Ctrl+1"
        onActivated: currentPage = 0
    }
    Shortcut {
        sequence: "Ctrl+2"
        onActivated: currentPage = 1
    }
    Shortcut {
        sequence: "Ctrl+3"
        onActivated: currentPage = 2
    }
    Shortcut {
        sequence: "Ctrl+4"
        onActivated: currentPage = 3
    }
    Shortcut {
        sequence: "Ctrl+Q"
        onActivated: Qt.quit()
    }
}
