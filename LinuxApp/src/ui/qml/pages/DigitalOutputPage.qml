import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."
import "../components"

/**
 * Digital Output Page - Control 64 Digital Outputs
 */
Item {
    id: root
    
    property var model: null
    
    signal toggleOutput(int channel)
    signal setAllOutputs(bool state)
    
    // Get channel states from model
    property var channelStates: {
        if (!model) return []
        var states = []
        for (var i = 0; i < 64; i++) {
            states.push(model.getChannelState(i))
        }
        return states
    }
    
    // Refresh states when model updates
    Connections {
        target: model
        function onDataChanged() {
            channelStates = Qt.binding(function() {
                if (!model) return []
                var states = []
                for (var i = 0; i < 64; i++) {
                    states.push(model.getChannelState(i))
                }
                return states
            })
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: Style.spacingL
        
        // === Header ===
        RowLayout {
            Layout.fillWidth: true
            spacing: Style.spacingL
            
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Style.spacingXS
                
                Text {
                    text: "Digital Output Control"
                    font.pixelSize: Style.fontSizeXL
                    font.weight: Font.Medium
                    color: Style.textPrimary
                }
                
                Text {
                    text: "Click on any output channel to toggle its state"
                    font.pixelSize: Style.fontSizeM
                    color: Style.textSecondary
                }
            }
            
            // Active count badge
            Rectangle {
                Layout.preferredWidth: activeRow.implicitWidth + Style.spacingL
                Layout.preferredHeight: 44
                radius: Style.radiusM
                color: Style.bgCard
                border.width: 1
                border.color: Style.border
                
                RowLayout {
                    id: activeRow
                    anchors.centerIn: parent
                    spacing: Style.spacingS
                    
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: Style.doActive
                    }
                    
                    Text {
                        text: (model ? model.activeCount : 0) + " Active"
                        font.pixelSize: Style.fontSizeM
                        font.weight: Font.Medium
                        color: Style.textPrimary
                    }
                }
            }
        }
        
        // === Control Bar ===
        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            
            RowLayout {
                anchors.fill: parent
                spacing: Style.spacingM
                
                Text {
                    text: "Quick Actions:"
                    font.pixelSize: Style.fontSizeM
                    font.weight: Font.Medium
                    color: Style.textSecondary
                }
                
                PrimaryButton {
                    text: "All ON"
                    icon: "●"
                    onClicked: setAllOutputs(true)
                }
                
                PrimaryButton {
                    text: "All OFF"
                    icon: "○"
                    destructive: true
                    onClicked: setAllOutputs(false)
                }
                
                // Separator
                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    Layout.topMargin: 8
                    Layout.bottomMargin: 8
                    color: Style.border
                }
                
                // Pattern buttons
                Text {
                    text: "Patterns:"
                    font.pixelSize: Style.fontSizeM
                    color: Style.textSecondary
                }
                
                PrimaryButton {
                    text: "Alternate"
                    outline: true
                    onClicked: {
                        // Set alternating pattern 0xAA
                        if (appController) {
                            appController.setPattern(0xAA)
                        }
                    }
                }
                
                PrimaryButton {
                    text: "First Half"
                    outline: true
                    onClicked: {
                        if (appController) {
                            appController.setFirstHalf(true)
                        }
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Sync indicator
                RowLayout {
                    spacing: Style.spacingXS
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: Style.success
                        
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }
                    
                    Text {
                        text: "Synced"
                        font.pixelSize: Style.fontSizeS
                        color: Style.textSecondary
                    }
                }
            }
        }
        
        // === Legend ===
        RowLayout {
            Layout.fillWidth: true
            spacing: Style.spacingL
            
            RowLayout {
                spacing: Style.spacingS
                
                Rectangle {
                    width: 20
                    height: 20
                    radius: 4
                    color: Style.doActive
                }
                Text {
                    text: "Output ON"
                    font.pixelSize: Style.fontSizeS
                    color: Style.textSecondary
                }
            }
            
            RowLayout {
                spacing: Style.spacingS
                
                Rectangle {
                    width: 20
                    height: 20
                    radius: 4
                    color: Style.doInactive
                    border.width: 1
                    border.color: Style.border
                }
                Text {
                    text: "Output OFF"
                    font.pixelSize: Style.fontSizeS
                    color: Style.textSecondary
                }
            }
            
            Text {
                text: "💡 Tip: Click any channel to toggle"
                font.pixelSize: Style.fontSizeS
                color: Style.textMuted
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // === Main Grid ===
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            Item {
                anchors.fill: parent
                
                DioGrid {
                    anchors.centerIn: parent
                    channelStates: root.channelStates
                    isOutput: true
                    
                    onChannelClicked: function(channel) {
                        toggleOutput(channel)
                    }
                }
            }
        }
        
        // === Channel Details Bar ===
        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            
            RowLayout {
                anchors.fill: parent
                spacing: Style.spacingXL
                
                // Bank summaries with controls
                Repeater {
                    model: 8
                    
                    Rectangle {
                        Layout.preferredWidth: bankCol.implicitWidth + Style.spacingM
                        Layout.fillHeight: true
                        color: "transparent"
                        radius: Style.radiusS
                        
                        ColumnLayout {
                            id: bankCol
                            anchors.centerIn: parent
                            spacing: 4
                            
                            Text {
                                text: "Bank " + (index + 1)
                                font.pixelSize: Style.fontSizeS
                                color: Style.textMuted
                            }
                            
                            RowLayout {
                                spacing: 2
                                
                                Repeater {
                                    model: 8
                                    
                                    Rectangle {
                                        width: 8
                                        height: 16
                                        radius: 2
                                        
                                        property int bankIndex: parent.parent.parent.parent.modelData
                                        property int channelIdx: bankIndex * 8 + index
                                        color: channelStates[channelIdx] ? Style.doActive : Style.doInactive
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: toggleOutput(channelIdx)
                                        }
                                    }
                                }
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // Toggle entire bank
                                if (appController) {
                                    appController.toggleBank(index)
                                }
                            }
                        }
                    }
                }
                
                Item { Layout.fillWidth: true }
            }
        }
    }
}
