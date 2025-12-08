/**
 * SettingsPage.qml - Application Settings
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import EnersionApp 1.0

Page {
    id: root
    
    background: Rectangle {
        color: "#121212"
    }
    
    ScrollView {
        anchors.fill: parent
        anchors.margins: 16
        contentWidth: availableWidth
        
        ColumnLayout {
            width: parent.width
            spacing: 16
            
            // Header
            Label {
                text: "⚙️ Settings"
                font.pixelSize: 24
                font.bold: true
                color: "#00BFA5"
            }
            
            // About Section
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: aboutContent.height + 32
                radius: 12
                color: "#2D2D2D"
                
                ColumnLayout {
                    id: aboutContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Label {
                        text: "About"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#00BFA5"
                    }
                    
                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 8
                        
                        Label { text: "Application:"; opacity: 0.6 }
                        Label { text: "Enersion Controller"; font.bold: true }
                        
                        Label { text: "Version:"; opacity: 0.6 }
                        Label { text: "v" + AppController.appVersion; font.bold: true }
                        
                        Label { text: "Target:"; opacity: 0.6 }
                        Label { text: "STM32MP257 MYIR Board"; font.bold: true }
                        
                        Label { text: "Display:"; opacity: 0.6 }
                        Label { text: "HDMI Touchscreen"; font.bold: true }
                        
                        Label { text: "Protocol:"; opacity: 0.6 }
                        Label { text: "Enersion RS485"; font.bold: true }
                    }
                }
            }
            
            // Communication Settings
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: commContent.height + 32
                radius: 12
                color: "#2D2D2D"
                
                ColumnLayout {
                    id: commContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Label {
                        text: "Communication"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#00BFA5"
                    }
                    
                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 12
                        
                        Label { text: "Response Timeout:" }
                        SpinBox {
                            from: 100
                            to: 5000
                            stepSize: 100
                            value: 500
                            editable: true
                            
                            Material.background: "#3D3D3D"
                            
                            textFromValue: function(value) { return value + " ms" }
                        }
                        
                        Label { text: "Retry Count:" }
                        SpinBox {
                            from: 0
                            to: 10
                            value: 3
                            
                            Material.background: "#3D3D3D"
                        }
                        
                        Label { text: "Heartbeat Interval:" }
                        ComboBox {
                            model: ["1 second", "2 seconds", "5 seconds", "10 seconds"]
                            currentIndex: 1
                            
                            Material.background: "#3D3D3D"
                        }
                    }
                }
            }
            
            // Device Addresses
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: addrContent.height + 32
                radius: 12
                color: "#2D2D2D"
                
                ColumnLayout {
                    id: addrContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Label {
                        text: "Device Addresses"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#00BFA5"
                    }
                    
                    GridLayout {
                        columns: 3
                        columnSpacing: 16
                        rowSpacing: 8
                        
                        Label { text: "Device"; font.bold: true }
                        Label { text: "Address"; font.bold: true }
                        Label { text: "Status"; font.bold: true }
                        
                        Label { text: "Master (Linux)" }
                        Label { text: "0x10"; font.family: "monospace" }
                        Label { text: "—"; opacity: 0.6 }
                        
                        Label { text: "Controller DIO" }
                        Label { text: "0x02"; font.family: "monospace" }
                        Row {
                            spacing: 6
                            Rectangle {
                                width: 10; height: 10; radius: 5
                                color: AppController.deviceManager.diOnline ? "#4CAF50" : "#F44336"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Label { 
                                text: AppController.deviceManager.diOnline ? "Online" : "Offline"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        
                        Label { text: "Controller OUT" }
                        Label { text: "0x03"; font.family: "monospace" }
                        Row {
                            spacing: 6
                            Rectangle {
                                width: 10; height: 10; radius: 5
                                color: AppController.deviceManager.doOnline ? "#4CAF50" : "#F44336"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Label { 
                                text: AppController.deviceManager.doOnline ? "Online" : "Offline"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
            
            // System Info
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: sysContent.height + 32
                radius: 12
                color: "#2D2D2D"
                
                ColumnLayout {
                    id: sysContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Label {
                        text: "System Information"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#00BFA5"
                    }
                    
                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 8
                        
                        Label { text: "Qt Version:"; opacity: 0.6 }
                        Label { text: "6.x"; font.bold: true }
                        
                        Label { text: "Architecture:"; opacity: 0.6 }
                        Label { text: Qt.platform.os; font.bold: true }
                        
                        Label { text: "Screen Size:"; opacity: 0.6 }
                        Label { text: Screen.width + " x " + Screen.height; font.bold: true }
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
            
            // Copyright
            Label {
                text: "© 2024 Enersion. All rights reserved."
                font.pixelSize: 12
                opacity: 0.4
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}

