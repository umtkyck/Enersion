/**
 * main.qml - Enersion Controller Main Window
 * Modern touch-friendly UI for STM32MP257 HDMI Touchscreen
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import EnersionApp 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 720
    title: "Enersion Controller"
    
    // Material Design Theme - Industrial Dark
    Material.theme: Material.Dark
    Material.primary: "#00897B"      // Teal 600
    Material.accent: "#26A69A"       // Teal 400
    Material.background: "#121212"
    Material.foreground: "#FFFFFF"
    
    // Custom colors
    readonly property color accentColor: "#00BFA5"
    readonly property color successColor: "#4CAF50"
    readonly property color warningColor: "#FF9800"
    readonly property color errorColor: "#F44336"
    readonly property color surfaceColor: "#1E1E1E"
    readonly property color cardColor: "#2D2D2D"
    
    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Top Status Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: surfaceColor
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16
                
                // Logo/Title
                Label {
                    text: "⚡ ENERSION"
                    font.pixelSize: 18
                    font.bold: true
                    color: accentColor
                }
                
                // Connection indicator
                Row {
                    spacing: 8
                    
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: AppController.deviceManager.connected ? successColor : errorColor
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Label {
                        text: AppController.deviceManager.connected ? 
                              "Connected" : "Disconnected"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Device status indicators
                Row {
                    spacing: 16
                    visible: AppController.deviceManager.connected
                    
                    // DI Controller status
                    Row {
                        spacing: 6
                        
                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: AppController.deviceManager.diOnline ? successColor : errorColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                        Label {
                            text: "DI: " + (AppController.deviceManager.diOnline ? 
                                  AppController.deviceManager.diControllerHealth + "%" : "Offline")
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    
                    // DO Controller status
                    Row {
                        spacing: 6
                        
                        Rectangle {
                            width: 10
                            height: 10
                            radius: 5
                            color: AppController.deviceManager.doOnline ? successColor : errorColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                        Label {
                            text: "DO: " + (AppController.deviceManager.doOnline ? 
                                  AppController.deviceManager.doControllerHealth + "%" : "Offline")
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                
                // Version
                Label {
                    text: "v" + AppController.appVersion
                    font.pixelSize: 12
                    opacity: 0.6
                }
            }
        }
        
        // Main content area with navigation
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // Left Navigation
            Rectangle {
                Layout.preferredWidth: 80
                Layout.fillHeight: true
                color: surfaceColor
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.topMargin: 16
                    spacing: 8
                    
                    // Navigation buttons
                    Repeater {
                        model: [
                            { icon: "🏠", text: "Home", page: 0 },
                            { icon: "📥", text: "DI", page: 1 },
                            { icon: "📤", text: "DO", page: 2 },
                            { icon: "⚙️", text: "Settings", page: 3 }
                        ]
                        
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 64
                            Layout.leftMargin: 8
                            Layout.rightMargin: 8
                            radius: 8
                            color: stackView.currentIndex === modelData.page ? 
                                   accentColor : "transparent"
                            
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                
                                Label {
                                    text: modelData.icon
                                    font.pixelSize: 24
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                
                                Label {
                                    text: modelData.text
                                    font.pixelSize: 10
                                    font.bold: stackView.currentIndex === modelData.page
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: stackView.currentIndex = modelData.page
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Page content
            StackLayout {
                id: stackView
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                // Home Page
                HomePage {
                    id: homePage
                }
                
                // Digital Input Page
                DigitalInputPage {
                    id: diPage
                }
                
                // Digital Output Page
                DigitalOutputPage {
                    id: doPage
                }
                
                // Settings Page
                SettingsPage {
                    id: settingsPage
                }
            }
        }
        
        // Bottom Status Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            color: surfaceColor
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                
                Label {
                    text: AppController.statusMessage
                    font.pixelSize: 12
                    opacity: 0.8
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                
                Label {
                    text: Qt.formatDateTime(new Date(), "hh:mm:ss")
                    font.pixelSize: 12
                    opacity: 0.6
                }
            }
        }
    }
    
    // Update time every second
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {} // Just triggers UI update for time
    }
}

