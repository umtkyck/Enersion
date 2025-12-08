/**
 * DigitalOutputPage.qml - Digital Output Control (64 channels)
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
                text: "📤 Digital Output Control"
                font.pixelSize: 24
                font.bold: true
                color: "#FF9800"
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: "Controller OUT (0x03)"
                font.pixelSize: 14
                opacity: 0.6
            }
            
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: AppController.deviceManager.doOnline ? "#4CAF50" : "#F44336"
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
                    text: "✓ Write Outputs"
                    enabled: AppController.deviceManager.doOnline
                    highlighted: true
                    
                    Material.background: AppController.doService.pendingChanges ? 
                                        "#FF9800" : "#4CAF50"
                    
                    onClicked: AppController.doService.writeOutputs()
                }
                
                Button {
                    text: "↻ Read Current"
                    enabled: AppController.deviceManager.doOnline
                    
                    Material.background: "#2196F3"
                    
                    onClicked: AppController.doService.readOutputs()
                }
                
                Rectangle {
                    width: 1
                    height: 30
                    color: "#404040"
                }
                
                Button {
                    text: "All ON"
                    enabled: AppController.deviceManager.doOnline
                    
                    Material.background: "#3D3D3D"
                    
                    onClicked: AppController.doService.setAllOutputs(true)
                }
                
                Button {
                    text: "All OFF"
                    enabled: AppController.deviceManager.doOnline
                    
                    Material.background: "#3D3D3D"
                    
                    onClicked: AppController.doService.clearAllOutputs()
                }
                
                Item { Layout.fillWidth: true }
                
                Label {
                    text: "Active: " + AppController.doService.activeCount + " / 64"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#FF9800"
                }
                
                Label {
                    visible: AppController.doService.pendingChanges
                    text: "⚠ Unsaved changes"
                    font.pixelSize: 12
                    color: "#FF9800"
                }
            }
        }
        
        // Output Grid
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
                            height: 56
                            radius: 6
                            
                            property bool isActive: index < AppController.doService.outputStates.length ?
                                                   AppController.doService.outputStates[index] : false
                            
                            color: isActive ? "#E65100" : "#37474F"
                            border.color: isActive ? "#FF9800" : "#546E7A"
                            border.width: isActive ? 2 : 1
                            
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                
                                Label {
                                    text: "DO" + index.toString().padStart(2, '0')
                                    font.pixelSize: 12
                                    font.bold: true
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                
                                Switch {
                                    id: outputSwitch
                                    checked: isActive
                                    enabled: AppController.deviceManager.doOnline
                                    Layout.alignment: Qt.AlignHCenter
                                    scale: 0.7
                                    
                                    Material.accent: "#FF9800"
                                    
                                    onToggled: {
                                        AppController.doService.setOutput(index, checked)
                                    }
                                }
                            }
                            
                            // Touch area for entire card
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (AppController.deviceManager.doOnline) {
                                        AppController.doService.toggleOutput(index)
                                    }
                                }
                            }
                            
                            // Animation
                            Behavior on color {
                                ColorAnimation { duration: 150 }
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
            color: AppController.doService.pendingChanges ? "#3E2723" : "#2D2D2D"
            border.color: AppController.doService.pendingChanges ? "#FF9800" : "transparent"
            border.width: AppController.doService.pendingChanges ? 1 : 0
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                
                Label {
                    text: AppController.doService.pendingChanges ? 
                          "⚠ Changes not written - Click 'Write Outputs' to apply" :
                          "✓ Outputs synchronized with device"
                    font.pixelSize: 12
                    color: AppController.doService.pendingChanges ? "#FF9800" : "#4CAF50"
                }
                
                Item { Layout.fillWidth: true }
                
                Label {
                    text: "Health: " + AppController.deviceManager.doControllerHealth + "%"
                    font.pixelSize: 12
                    color: AppController.deviceManager.doControllerHealth > 80 ? "#4CAF50" :
                           AppController.deviceManager.doControllerHealth > 50 ? "#FF9800" : "#F44336"
                }
            }
        }
    }
}

