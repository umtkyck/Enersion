import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."
import "../components"

/**
 * Dashboard Page - System Overview
 */
Item {
    id: root
    
    property bool connected: false
    property int diActiveCount: 0
    property int doActiveCount: 0
    
    signal navigateTo(int pageIndex)
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: Style.spacingL
            
            // === Welcome Section ===
            RowLayout {
                Layout.fillWidth: true
                spacing: Style.spacingL
                
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Style.spacingXS
                    
                    Text {
                        text: "Welcome to Enersion"
                        font.pixelSize: Style.fontSizeHero
                        font.weight: Font.Bold
                        color: Style.textPrimary
                    }
                    
                    Text {
                        text: "Industrial Control System for Digital I/O Management"
                        font.pixelSize: Style.fontSizeL
                        color: Style.textSecondary
                    }
                }
                
                // Connection Status Card
                Rectangle {
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 80
                    radius: Style.radiusL
                    color: connected ? Qt.rgba(0, 0.9, 0.46, 0.1) : Qt.rgba(1, 0.32, 0.32, 0.1)
                    border.width: 1
                    border.color: connected ? Style.success : Style.error
                    
                    RowLayout {
                        anchors.centerIn: parent
                        spacing: Style.spacingM
                        
                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: connected ? Style.success : Style.error
                            
                            SequentialAnimation on scale {
                                running: connected
                                loops: Animation.Infinite
                                NumberAnimation { to: 1.2; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }
                        
                        ColumnLayout {
                            spacing: 2
                            
                            Text {
                                text: connected ? "ONLINE" : "OFFLINE"
                                font.pixelSize: Style.fontSizeL
                                font.weight: Font.Bold
                                color: connected ? Style.success : Style.error
                            }
                            
                            Text {
                                text: connected ? "System Active" : "Connect to start"
                                font.pixelSize: Style.fontSizeS
                                color: Style.textSecondary
                            }
                        }
                    }
                }
            }
            
            // === Stats Cards Row ===
            RowLayout {
                Layout.fillWidth: true
                spacing: Style.spacingL
                
                // DI Stats Card
                Card {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    title: "Digital Inputs"
                    icon: "▣"
                    hoverable: true
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Style.spacingM
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Text {
                                text: diActiveCount.toString()
                                font.pixelSize: 56
                                font.weight: Font.Bold
                                color: Style.diActive
                            }
                            
                            Text {
                                text: "/ 64"
                                font.pixelSize: Style.fontSizeXXL
                                color: Style.textMuted
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            // Mini bar chart
                            Rectangle {
                                width: 120
                                height: 40
                                color: "transparent"
                                
                                Row {
                                    anchors.bottom: parent.bottom
                                    spacing: 4
                                    
                                    Repeater {
                                        model: 8
                                        
                                        Rectangle {
                                            width: 10
                                            height: Math.max(4, Math.random() * 40)
                                            color: Style.diActive
                                            opacity: 0.3 + Math.random() * 0.7
                                            radius: 2
                                        }
                                    }
                                }
                            }
                        }
                        
                        Text {
                            text: "Active inputs detected"
                            font.pixelSize: Style.fontSizeM
                            color: Style.textSecondary
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: navigateTo(1)
                    }
                }
                
                // DO Stats Card
                Card {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    title: "Digital Outputs"
                    icon: "◧"
                    hoverable: true
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Style.spacingM
                        
                        RowLayout {
                            Layout.fillWidth: true
                            
                            Text {
                                text: doActiveCount.toString()
                                font.pixelSize: 56
                                font.weight: Font.Bold
                                color: Style.doActive
                            }
                            
                            Text {
                                text: "/ 64"
                                font.pixelSize: Style.fontSizeXXL
                                color: Style.textMuted
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            // Progress ring
                            Item {
                                width: 50
                                height: 50
                                
                                Canvas {
                                    anchors.fill: parent
                                    
                                    onPaint: {
                                        var ctx = getContext("2d")
                                        ctx.reset()
                                        
                                        var centerX = width / 2
                                        var centerY = height / 2
                                        var radius = 20
                                        var lineWidth = 6
                                        
                                        // Background circle
                                        ctx.beginPath()
                                        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2)
                                        ctx.strokeStyle = Style.border
                                        ctx.lineWidth = lineWidth
                                        ctx.stroke()
                                        
                                        // Progress arc
                                        var progress = doActiveCount / 64
                                        ctx.beginPath()
                                        ctx.arc(centerX, centerY, radius, -Math.PI/2, -Math.PI/2 + progress * Math.PI * 2)
                                        ctx.strokeStyle = Style.doActive
                                        ctx.lineWidth = lineWidth
                                        ctx.lineCap = "round"
                                        ctx.stroke()
                                    }
                                }
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: Math.round(doActiveCount / 64 * 100) + "%"
                                    font.pixelSize: Style.fontSizeXS
                                    font.weight: Font.Bold
                                    color: Style.textPrimary
                                }
                            }
                        }
                        
                        Text {
                            text: "Outputs currently active"
                            font.pixelSize: Style.fontSizeM
                            color: Style.textSecondary
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: navigateTo(2)
                    }
                }
            }
            
            // === Quick Actions ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                title: "Quick Actions"
                icon: "⚡"
                
                RowLayout {
                    anchors.fill: parent
                    spacing: Style.spacingM
                    
                    PrimaryButton {
                        text: "View Inputs"
                        icon: "▣"
                        onClicked: navigateTo(1)
                    }
                    
                    PrimaryButton {
                        text: "Control Outputs"
                        icon: "◧"
                        onClicked: navigateTo(2)
                    }
                    
                    PrimaryButton {
                        text: "All OFF"
                        icon: "○"
                        outline: true
                        enabled: connected && doActiveCount > 0
                        onClicked: {
                            if (appController) {
                                appController.setAllOutputs(false)
                            }
                        }
                    }
                    
                    PrimaryButton {
                        text: "All ON"
                        icon: "●"
                        outline: true
                        enabled: connected
                        onClicked: {
                            if (appController) {
                                appController.setAllOutputs(true)
                            }
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    PrimaryButton {
                        text: "Settings"
                        icon: "⚙"
                        outline: true
                        onClicked: navigateTo(3)
                    }
                }
            }
            
            // === System Info ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                title: "System Information"
                icon: "ℹ"
                
                GridLayout {
                    anchors.fill: parent
                    columns: 4
                    rowSpacing: Style.spacingM
                    columnSpacing: Style.spacingXL
                    
                    // Info items
                    Repeater {
                        model: [
                            { label: "Device", value: "MYIR STM32MP257" },
                            { label: "RS485 Port", value: "/dev/ttySTM9" },
                            { label: "Baud Rate", value: "115200" },
                            { label: "DI Controller", value: connected ? "0x02 ✓" : "0x02 ✗" },
                            { label: "DO Controller", value: connected ? "0x03 ✓" : "0x03 ✗" },
                            { label: "Protocol", value: "Enersion v1.0" },
                            { label: "Uptime", value: "00:00:00" },
                            { label: "Errors", value: "0" }
                        ]
                        
                        ColumnLayout {
                            spacing: 4
                            
                            Text {
                                text: modelData.label
                                font.pixelSize: Style.fontSizeS
                                color: Style.textMuted
                            }
                            
                            Text {
                                text: modelData.value
                                font.pixelSize: Style.fontSizeM
                                font.weight: Font.Medium
                                color: Style.textPrimary
                            }
                        }
                    }
                }
            }
            
            // Spacer
            Item { Layout.preferredHeight: Style.spacingL }
        }
    }
}

