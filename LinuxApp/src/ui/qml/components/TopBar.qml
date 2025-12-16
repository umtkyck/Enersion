import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * Top Header Bar with page title and status indicators
 */
Rectangle {
    id: root
    
    property string title: "Dashboard"
    property bool connected: false
    property int diCount: 0
    property int doCount: 0
    
    color: Style.bgDark
    
    // Bottom border
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Style.border
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Style.spacingL
        anchors.rightMargin: Style.spacingL
        spacing: Style.spacingL
        
        // === Page Title ===
        Text {
            text: title
            font.pixelSize: Style.fontSizeXXL
            font.weight: Font.Bold
            color: Style.textPrimary
        }
        
        Item { Layout.fillWidth: true }
        
        // === Status Pills ===
        RowLayout {
            spacing: Style.spacingM
            
            // DI Status
            Rectangle {
                Layout.preferredWidth: diPill.implicitWidth + Style.spacingL
                Layout.preferredHeight: 36
                color: Style.bgCard
                radius: 18
                border.width: 1
                border.color: Style.border
                
                RowLayout {
                    id: diPill
                    anchors.centerIn: parent
                    spacing: Style.spacingS
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: diCount > 0 ? Style.diActive : Style.textMuted
                    }
                    
                    Text {
                        text: "DI"
                        font.pixelSize: Style.fontSizeS
                        font.weight: Font.Medium
                        color: Style.textSecondary
                    }
                    
                    Text {
                        text: diCount + "/64"
                        font.pixelSize: Style.fontSizeS
                        font.weight: Font.Bold
                        color: Style.textPrimary
                    }
                }
            }
            
            // DO Status
            Rectangle {
                Layout.preferredWidth: doPill.implicitWidth + Style.spacingL
                Layout.preferredHeight: 36
                color: Style.bgCard
                radius: 18
                border.width: 1
                border.color: Style.border
                
                RowLayout {
                    id: doPill
                    anchors.centerIn: parent
                    spacing: Style.spacingS
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: doCount > 0 ? Style.doActive : Style.textMuted
                    }
                    
                    Text {
                        text: "DO"
                        font.pixelSize: Style.fontSizeS
                        font.weight: Font.Medium
                        color: Style.textSecondary
                    }
                    
                    Text {
                        text: doCount + "/64"
                        font.pixelSize: Style.fontSizeS
                        font.weight: Font.Bold
                        color: Style.textPrimary
                    }
                }
            }
        }
        
        // === Time Display ===
        Rectangle {
            Layout.preferredWidth: timeText.implicitWidth + Style.spacingL
            Layout.preferredHeight: 36
            color: "transparent"
            
            Text {
                id: timeText
                anchors.centerIn: parent
                text: Qt.formatDateTime(new Date(), "hh:mm:ss")
                font.pixelSize: Style.fontSizeM
                font.family: "monospace"
                color: Style.textSecondary
                
                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: timeText.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
                }
            }
        }
    }
}

