/**
 * HomePage.qml - Main Dashboard
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
            
            // Connection Panel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                radius: 12
                color: "#2D2D2D"
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12
                    
                    Label {
                        text: "RS485 Connection"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#00BFA5"
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        
                        ComboBox {
                            id: portCombo
                            Layout.preferredWidth: 300
                            model: AppController.deviceManager.availablePorts()
                            enabled: !AppController.deviceManager.connected
                            
                            Material.background: "#3D3D3D"
                        }
                        
                        Button {
                            text: "🔄"
                            enabled: !AppController.deviceManager.connected
                            onClicked: portCombo.model = AppController.deviceManager.availablePorts()
                            
                            Material.background: "#3D3D3D"
                        }
                        
                        ComboBox {
                            id: baudCombo
                            Layout.preferredWidth: 120
                            model: ["9600", "19200", "38400", "57600", "115200"]
                            currentIndex: 4
                            enabled: !AppController.deviceManager.connected
                            
                            Material.background: "#3D3D3D"
                        }
                        
                        Button {
                            text: AppController.deviceManager.connected ? "Disconnect" : "Connect"
                            highlighted: !AppController.deviceManager.connected
                            Material.background: AppController.deviceManager.connected ? 
                                                "#F44336" : "#00897B"
                            
                            onClicked: {
                                if (AppController.deviceManager.connected) {
                                    AppController.deviceManager.disconnect()
                                } else {
                                    let port = portCombo.currentText.split(" - ")[0]
                                    let baud = parseInt(baudCombo.currentText)
                                    AppController.deviceManager.connect(port, baud)
                                }
                            }
                        }
                        
                        Button {
                            text: "Scan Devices"
                            enabled: AppController.deviceManager.connected
                            onClicked: AppController.deviceManager.scanDevices()
                            
                            Material.background: "#3D3D3D"
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            // Device Status Cards
            RowLayout {
                Layout.fillWidth: true
                spacing: 16
                
                // DI Controller Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    radius: 12
                    color: "#2D2D2D"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: "📥 Digital Input Controller"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: AppController.deviceManager.diOnline ? 
                                       "#4CAF50" : "#F44336"
                            }
                        }
                        
                        Label {
                            text: "Address: 0x02"
                            font.pixelSize: 12
                            opacity: 0.6
                        }
                        
                        Item { Layout.fillHeight: true }
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: "Health:"
                                font.pixelSize: 14
                            }
                            
                            ProgressBar {
                                Layout.fillWidth: true
                                value: AppController.deviceManager.diControllerHealth / 100.0
                                
                                Material.accent: value > 0.8 ? "#4CAF50" :
                                                value > 0.5 ? "#FF9800" : "#F44336"
                            }
                            
                            Label {
                                text: AppController.deviceManager.diControllerHealth + "%"
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }
                        
                        Label {
                            text: "Active Inputs: " + AppController.diService.activeCount + " / 64"
                            font.pixelSize: 14
                        }
                        
                        Button {
                            text: "Open Digital Inputs →"
                            Layout.fillWidth: true
                            enabled: AppController.deviceManager.diOnline
                            onClicked: stackView.currentIndex = 1
                            
                            Material.background: "#00897B"
                        }
                    }
                }
                
                // DO Controller Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    radius: 12
                    color: "#2D2D2D"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: "📤 Digital Output Controller"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: AppController.deviceManager.doOnline ? 
                                       "#4CAF50" : "#F44336"
                            }
                        }
                        
                        Label {
                            text: "Address: 0x03"
                            font.pixelSize: 12
                            opacity: 0.6
                        }
                        
                        Item { Layout.fillHeight: true }
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Label {
                                text: "Health:"
                                font.pixelSize: 14
                            }
                            
                            ProgressBar {
                                Layout.fillWidth: true
                                value: AppController.deviceManager.doControllerHealth / 100.0
                                
                                Material.accent: value > 0.8 ? "#4CAF50" :
                                                value > 0.5 ? "#FF9800" : "#F44336"
                            }
                            
                            Label {
                                text: AppController.deviceManager.doControllerHealth + "%"
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }
                        
                        Label {
                            text: "Active Outputs: " + AppController.doService.activeCount + " / 64"
                            font.pixelSize: 14
                        }
                        
                        Button {
                            text: "Open Digital Outputs →"
                            Layout.fillWidth: true
                            enabled: AppController.deviceManager.doOnline
                            onClicked: stackView.currentIndex = 2
                            
                            Material.background: "#00897B"
                        }
                    }
                }
            }
            
            // Quick Stats
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                radius: 12
                color: "#2D2D2D"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    
                    // DI Active
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Label {
                            text: AppController.diService.activeCount.toString()
                            font.pixelSize: 36
                            font.bold: true
                            color: "#26A69A"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Label {
                            text: "Active Inputs"
                            font.pixelSize: 12
                            opacity: 0.6
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                    
                    Rectangle {
                        width: 1
                        Layout.fillHeight: true
                        color: "#404040"
                    }
                    
                    // DO Active
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Label {
                            text: AppController.doService.activeCount.toString()
                            font.pixelSize: 36
                            font.bold: true
                            color: "#FF9800"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Label {
                            text: "Active Outputs"
                            font.pixelSize: 12
                            opacity: 0.6
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                    
                    Rectangle {
                        width: 1
                        Layout.fillHeight: true
                        color: "#404040"
                    }
                    
                    // Connection Status
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Label {
                            text: AppController.deviceManager.connected ? "●" : "○"
                            font.pixelSize: 36
                            color: AppController.deviceManager.connected ? "#4CAF50" : "#F44336"
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Label {
                            text: AppController.deviceManager.connected ? "Online" : "Offline"
                            font.pixelSize: 12
                            opacity: 0.6
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
        }
    }
}

