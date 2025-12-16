import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."
import "../components"

/**
 * Settings Page - Connection and System Configuration
 */
Item {
    id: root
    
    property var controller: null
    
    signal connect(string port, int baudrate)
    signal disconnect()
    
    property bool isConnected: controller ? controller.isConnected : false
    
    // Available ports (populated from controller)
    property var availablePorts: controller ? controller.availablePorts : [
        "/dev/ttySTM9 - RS485 (J2 Connector)",
        "/dev/ttySTM0 - STM32 UART",
        "/dev/ttyUSB0 - USB Serial"
    ]
    
    // Available baud rates
    property var baudRates: [9600, 19200, 38400, 57600, 115200, 230400, 460800]
    
    // Selected values
    property string selectedPort: "/dev/ttySTM9"
    property int selectedBaudrate: 115200
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: Style.spacingL
            
            // === Connection Section ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 280
                title: "Connection Settings"
                icon: "🔌"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: Style.spacingL
                    
                    // Status Banner
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        radius: Style.radiusM
                        color: isConnected ? Qt.rgba(0, 0.9, 0.46, 0.1) : Qt.rgba(1, 0.32, 0.32, 0.1)
                        border.width: 1
                        border.color: isConnected ? Style.success : Style.error
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: Style.spacingM
                            
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: isConnected ? Style.success : Style.error
                            }
                            
                            Text {
                                text: isConnected ? "Connected to RS485 Device" : "Not Connected"
                                font.pixelSize: Style.fontSizeM
                                font.weight: Font.Medium
                                color: isConnected ? Style.success : Style.error
                            }
                            
                            Item { Layout.fillWidth: true }
                            
                            Text {
                                visible: isConnected
                                text: selectedPort + " @ " + selectedBaudrate
                                font.pixelSize: Style.fontSizeS
                                color: Style.textSecondary
                            }
                        }
                    }
                    
                    // Port and Baudrate Row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.spacingL
                        
                        // Port Selection
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: Style.spacingS
                            
                            Text {
                                text: "Serial Port"
                                font.pixelSize: Style.fontSizeS
                                font.weight: Font.Medium
                                color: Style.textSecondary
                            }
                            
                            ComboBox {
                                id: portCombo
                                Layout.fillWidth: true
                                Layout.preferredHeight: Style.inputHeight
                                
                                model: availablePorts
                                currentIndex: 0
                                enabled: !isConnected
                                
                                background: Rectangle {
                                    color: Style.bgInput
                                    radius: Style.radiusM
                                    border.width: 1
                                    border.color: portCombo.activeFocus ? Style.primary : Style.border
                                }
                                
                                contentItem: Text {
                                    text: portCombo.displayText
                                    font.pixelSize: Style.fontSizeM
                                    color: Style.textPrimary
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: Style.spacingM
                                }
                                
                                onCurrentTextChanged: {
                                    selectedPort = currentText.split(" - ")[0]
                                }
                            }
                        }
                        
                        // Baudrate Selection
                        ColumnLayout {
                            Layout.preferredWidth: 200
                            spacing: Style.spacingS
                            
                            Text {
                                text: "Baud Rate"
                                font.pixelSize: Style.fontSizeS
                                font.weight: Font.Medium
                                color: Style.textSecondary
                            }
                            
                            ComboBox {
                                id: baudrateCombo
                                Layout.fillWidth: true
                                Layout.preferredHeight: Style.inputHeight
                                
                                model: baudRates
                                currentIndex: 4  // 115200
                                enabled: !isConnected
                                
                                background: Rectangle {
                                    color: Style.bgInput
                                    radius: Style.radiusM
                                    border.width: 1
                                    border.color: baudrateCombo.activeFocus ? Style.primary : Style.border
                                }
                                
                                contentItem: Text {
                                    text: baudrateCombo.displayText
                                    font.pixelSize: Style.fontSizeM
                                    color: Style.textPrimary
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: Style.spacingM
                                }
                                
                                onCurrentTextChanged: {
                                    selectedBaudrate = parseInt(currentText)
                                }
                            }
                        }
                    }
                    
                    // Connect/Disconnect Button
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Style.spacingM
                        
                        PrimaryButton {
                            Layout.preferredWidth: 160
                            text: isConnected ? "Disconnect" : "Connect"
                            icon: isConnected ? "✕" : "→"
                            destructive: isConnected
                            
                            onClicked: {
                                if (isConnected) {
                                    disconnect()
                                } else {
                                    connect(selectedPort, selectedBaudrate)
                                }
                            }
                        }
                        
                        PrimaryButton {
                            text: "Refresh Ports"
                            icon: "⟳"
                            outline: true
                            enabled: !isConnected
                            
                            onClicked: {
                                if (controller) {
                                    controller.refreshPorts()
                                }
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            // === Device Information ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                title: "Device Information"
                icon: "ℹ"
                
                GridLayout {
                    anchors.fill: parent
                    columns: 3
                    rowSpacing: Style.spacingM
                    columnSpacing: Style.spacingXL
                    
                    Repeater {
                        model: [
                            { label: "Board", value: "MYIR STM32MP257" },
                            { label: "RS485 Port", value: "/dev/ttySTM9" },
                            { label: "GPIO (Direction)", value: "PI10 (138)" },
                            { label: "DI Controller", value: "Address 0x02" },
                            { label: "DO Controller", value: "Address 0x03" },
                            { label: "Protocol", value: "Enersion v1.0" },
                            { label: "Firmware", value: controller && isConnected ? controller.firmwareVersion : "N/A" },
                            { label: "Uptime", value: controller && isConnected ? controller.uptime : "N/A" },
                            { label: "Errors", value: controller && isConnected ? controller.errorCount : "0" }
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
            
            // === Polling Settings ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                title: "Polling Settings"
                icon: "⏱"
                
                RowLayout {
                    anchors.fill: parent
                    spacing: Style.spacingXL
                    
                    ColumnLayout {
                        spacing: Style.spacingS
                        
                        Text {
                            text: "DI Poll Interval"
                            font.pixelSize: Style.fontSizeS
                            font.weight: Font.Medium
                            color: Style.textSecondary
                        }
                        
                        RowLayout {
                            spacing: Style.spacingS
                            
                            Slider {
                                id: diPollSlider
                                Layout.preferredWidth: 200
                                from: 50
                                to: 1000
                                stepSize: 50
                                value: 100
                                
                                background: Rectangle {
                                    x: diPollSlider.leftPadding
                                    y: diPollSlider.topPadding + diPollSlider.availableHeight / 2 - height / 2
                                    width: diPollSlider.availableWidth
                                    height: 4
                                    radius: 2
                                    color: Style.bgInput
                                    
                                    Rectangle {
                                        width: diPollSlider.visualPosition * parent.width
                                        height: parent.height
                                        color: Style.primary
                                        radius: 2
                                    }
                                }
                                
                                handle: Rectangle {
                                    x: diPollSlider.leftPadding + diPollSlider.visualPosition * (diPollSlider.availableWidth - width)
                                    y: diPollSlider.topPadding + diPollSlider.availableHeight / 2 - height / 2
                                    width: 20
                                    height: 20
                                    radius: 10
                                    color: Style.primary
                                }
                            }
                            
                            Text {
                                text: diPollSlider.value + " ms"
                                font.pixelSize: Style.fontSizeM
                                font.family: "monospace"
                                color: Style.textPrimary
                            }
                        }
                    }
                    
                    ColumnLayout {
                        spacing: Style.spacingS
                        
                        Text {
                            text: "DO Sync Interval"
                            font.pixelSize: Style.fontSizeS
                            font.weight: Font.Medium
                            color: Style.textSecondary
                        }
                        
                        RowLayout {
                            spacing: Style.spacingS
                            
                            Slider {
                                id: doPollSlider
                                Layout.preferredWidth: 200
                                from: 50
                                to: 1000
                                stepSize: 50
                                value: 200
                                
                                background: Rectangle {
                                    x: doPollSlider.leftPadding
                                    y: doPollSlider.topPadding + doPollSlider.availableHeight / 2 - height / 2
                                    width: doPollSlider.availableWidth
                                    height: 4
                                    radius: 2
                                    color: Style.bgInput
                                    
                                    Rectangle {
                                        width: doPollSlider.visualPosition * parent.width
                                        height: parent.height
                                        color: Style.doActive
                                        radius: 2
                                    }
                                }
                                
                                handle: Rectangle {
                                    x: doPollSlider.leftPadding + doPollSlider.visualPosition * (doPollSlider.availableWidth - width)
                                    y: doPollSlider.topPadding + doPollSlider.availableHeight / 2 - height / 2
                                    width: 20
                                    height: 20
                                    radius: 10
                                    color: Style.doActive
                                }
                            }
                            
                            Text {
                                text: doPollSlider.value + " ms"
                                font.pixelSize: Style.fontSizeM
                                font.family: "monospace"
                                color: Style.textPrimary
                            }
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    PrimaryButton {
                        text: "Apply"
                        icon: "✓"
                        
                        onClicked: {
                            if (controller) {
                                controller.setDiPollInterval(diPollSlider.value)
                                controller.setDoPollInterval(doPollSlider.value)
                            }
                        }
                    }
                }
            }
            
            // === About ===
            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                title: "About"
                icon: "⚡"
                
                RowLayout {
                    anchors.fill: parent
                    spacing: Style.spacingL
                    
                    Text {
                        text: "Enersion Control System"
                        font.pixelSize: Style.fontSizeL
                        font.weight: Font.Bold
                        color: Style.primary
                    }
                    
                    Text {
                        text: "v1.0.0"
                        font.pixelSize: Style.fontSizeM
                        color: Style.textSecondary
                    }
                    
                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 20
                        color: Style.border
                    }
                    
                    Text {
                        text: "Industrial Digital I/O Control for MYIR STM32MP257"
                        font.pixelSize: Style.fontSizeM
                        color: Style.textSecondary
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: "© 2024 Enersion"
                        font.pixelSize: Style.fontSizeS
                        color: Style.textMuted
                    }
                }
            }
            
            // Spacer
            Item { Layout.preferredHeight: Style.spacingL }
        }
    }
}
