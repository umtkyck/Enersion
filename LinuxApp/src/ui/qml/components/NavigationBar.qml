import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * Left Navigation Sidebar
 */
Rectangle {
    id: root
    
    property int currentIndex: 0
    property var pages: []
    property bool connected: false
    
    signal pageSelected(int index)
    
    color: Style.bgSidebar
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // === Logo Area ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "transparent"
            
            ColumnLayout {
                anchors.centerIn: parent
                spacing: Style.spacingXS
                
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "⚡"
                    font.pixelSize: 32
                    color: Style.primary
                }
                
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "ENERSION"
                    font.pixelSize: Style.fontSizeL
                    font.weight: Font.Bold
                    font.letterSpacing: 3
                    color: Style.textPrimary
                }
            }
        }
        
        // === Divider ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.leftMargin: Style.spacingL
            Layout.rightMargin: Style.spacingL
            color: Style.border
        }
        
        // === Navigation Items ===
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Style.spacingM
            
            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: Style.spacingXS
                
                Repeater {
                    model: pages
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 52
                        Layout.leftMargin: Style.spacingM
                        Layout.rightMargin: Style.spacingM
                        
                        color: currentIndex === index ? Style.bgCardHover : "transparent"
                        radius: Style.radiusM
                        
                        // Active indicator
                        Rectangle {
                            visible: currentIndex === index
                            width: 3
                            height: 24
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            color: Style.primary
                            radius: 2
                        }
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Style.spacingM
                            anchors.rightMargin: Style.spacingM
                            spacing: Style.spacingM
                            
                            Text {
                                text: modelData.icon
                                font.pixelSize: Style.fontSizeXL
                                color: currentIndex === index ? Style.primary : Style.textSecondary
                            }
                            
                            Text {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Style.fontSizeM
                                font.weight: currentIndex === index ? Font.Medium : Font.Normal
                                color: currentIndex === index ? Style.textPrimary : Style.textSecondary
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            
                            onClicked: pageSelected(index)
                        }
                        
                        // Hover effect
                        Behavior on color {
                            ColorAnimation { duration: Style.animFast }
                        }
                        
                        states: State {
                            name: "hovered"
                            when: hoverHandler.hovered && currentIndex !== index
                            PropertyChanges { target: parent; color: Style.bgCard }
                        }
                        
                        HoverHandler {
                            id: hoverHandler
                        }
                    }
                }
            }
        }
        
        // === Connection Status ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            Layout.leftMargin: Style.spacingM
            Layout.rightMargin: Style.spacingM
            Layout.bottomMargin: Style.spacingM
            
            color: Style.bgCard
            radius: Style.radiusM
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Style.spacingM
                spacing: Style.spacingS
                
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: connected ? Style.success : Style.error
                    
                    SequentialAnimation on opacity {
                        running: connected
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.5; duration: 1000 }
                        NumberAnimation { to: 1.0; duration: 1000 }
                    }
                }
                
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    
                    Text {
                        text: connected ? "Connected" : "Disconnected"
                        font.pixelSize: Style.fontSizeS
                        font.weight: Font.Medium
                        color: Style.textPrimary
                    }
                    
                    Text {
                        text: connected ? "/dev/ttySTM9" : "No device"
                        font.pixelSize: Style.fontSizeXS
                        color: Style.textMuted
                    }
                }
            }
        }
    }
}

