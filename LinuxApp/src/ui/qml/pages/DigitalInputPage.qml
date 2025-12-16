import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."
import "../components"

/**
 * Digital Input Page - Display 64 Digital Inputs
 */
Item {
    id: root
    
    property var model: null
    
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
                    text: "Digital Input Monitor"
                    font.pixelSize: Style.fontSizeXL
                    font.weight: Font.Medium
                    color: Style.textPrimary
                }
                
                Text {
                    text: "Real-time status of 64 digital input channels"
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
                        color: Style.diActive
                    }
                    
                    Text {
                        text: (model ? model.activeCount : 0) + " Active"
                        font.pixelSize: Style.fontSizeM
                        font.weight: Font.Medium
                        color: Style.textPrimary
                    }
                }
            }
            
            // Refresh indicator
            Rectangle {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: Style.radiusM
                color: Style.bgCard
                border.width: 1
                border.color: Style.border
                
                Text {
                    anchors.centerIn: parent
                    text: "⟳"
                    font.pixelSize: Style.fontSizeXL
                    color: Style.primary
                    
                    RotationAnimation on rotation {
                        running: model ? model.isPolling : false
                        loops: Animation.Infinite
                        from: 0
                        to: 360
                        duration: 1000
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
                    color: Style.diActive
                }
                Text {
                    text: "Input HIGH (Active)"
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
                    color: Style.diInactive
                    border.width: 1
                    border.color: Style.border
                }
                Text {
                    text: "Input LOW (Inactive)"
                    font.pixelSize: Style.fontSizeS
                    color: Style.textSecondary
                }
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
                    isOutput: false
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
                
                // Bank summaries
                Repeater {
                    model: 8
                    
                    ColumnLayout {
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
                                    
                                    property int channelIdx: parent.parent.parent.modelData * 8 + index
                                    color: channelStates[channelIdx] ? Style.diActive : Style.diInactive
                                }
                            }
                        }
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Poll rate
                ColumnLayout {
                    spacing: 4
                    
                    Text {
                        text: "Poll Rate"
                        font.pixelSize: Style.fontSizeS
                        color: Style.textMuted
                    }
                    
                    Text {
                        text: "100 ms"
                        font.pixelSize: Style.fontSizeM
                        font.weight: Font.Medium
                        color: Style.textPrimary
                    }
                }
            }
        }
    }
}
