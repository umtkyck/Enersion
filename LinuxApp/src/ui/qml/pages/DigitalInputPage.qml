/**
 * DigitalInputPage.qml - Digital Input Monitor (64 channels)
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
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            
            Label {
                text: "📥 Digital Input Monitor"
                font.pixelSize: 24
                font.bold: true
                color: "#00BFA5"
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: "Controller DIO (0x02)"
                font.pixelSize: 14
                opacity: 0.6
            }
            
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: AppController.deviceManager.diOnline ? "#4CAF50" : "#F44336"
            }
        }
        
        // Control Panel
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            radius: 8
            color: "#2D2D2D"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 16
                
                Button {
                    text: "🔄 Read Now"
                    enabled: AppController.deviceManager.diOnline
                    highlighted: true
                    
                    Material.background: "#2196F3"
                    
                    onClicked: AppController.diService.readInputs()
                }
                
                Button {
                    text: AppController.diService.autoRefresh ? 
                          "⏹ Stop Auto" : "▶ Auto Refresh"
                    enabled: AppController.deviceManager.diOnline
                    
                    Material.background: AppController.diService.autoRefresh ? 
                                        "#F44336" : "#4CAF50"
                    
                    onClicked: {
                        AppController.diService.autoRefresh = !AppController.diService.autoRefresh
                    }
                }
                
                ComboBox {
                    id: refreshInterval
                    model: ["500ms", "1s", "2s", "5s"]
                    currentIndex: 1
                    
                    Material.background: "#3D3D3D"
                    
                    onCurrentIndexChanged: {
                        let intervals = [500, 1000, 2000, 5000]
                        AppController.diService.refreshInterval = intervals[currentIndex]
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                Label {
                    text: "Active: " + AppController.diService.activeCount + " / 64"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#26A69A"
                }
            }
        }
        
        // Input Grid
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#2D2D2D"
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 12
                contentWidth: availableWidth
                
                GridLayout {
                    columns: 8
                    rowSpacing: 8
                    columnSpacing: 8
                    width: parent.width
                    
                    Repeater {
                        model: 64
                        
                        delegate: Rectangle {
                            width: 100
                            height: 48
                            radius: 6
                            
                            property bool isActive: index < AppController.diService.inputStates.length ?
                                                   AppController.diService.inputStates[index] : false
                            
                            color: isActive ? "#1B5E20" : "#37474F"
                            border.color: isActive ? "#4CAF50" : "#546E7A"
                            border.width: isActive ? 2 : 1
                            
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 2
                                
                                Label {
                                    text: "DI" + index.toString().padStart(2, '0')
                                    font.pixelSize: 12
                                    font.bold: true
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                
                                Label {
                                    text: isActive ? "HIGH" : "LOW"
                                    font.pixelSize: 10
                                    color: isActive ? "#A5D6A7" : "#90A4AE"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                            
                            // Pulse animation when state changes
                            Behavior on color {
                                ColorAnimation { duration: 200 }
                            }
                        }
                    }
                }
            }
        }
        
        // Status Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 8
            color: "#2D2D2D"
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                
                Label {
                    text: AppController.diService.autoRefresh ? 
                          "🟢 Auto-refresh active (" + refreshInterval.currentText + ")" :
                          "Manual refresh mode"
                    font.pixelSize: 12
                    opacity: 0.8
                }
                
                Item { Layout.fillWidth: true }
                
                Label {
                    text: "Health: " + AppController.deviceManager.diControllerHealth + "%"
                    font.pixelSize: 12
                    color: AppController.deviceManager.diControllerHealth > 80 ? "#4CAF50" :
                           AppController.deviceManager.diControllerHealth > 50 ? "#FF9800" : "#F44336"
                }
            }
        }
    }
}

