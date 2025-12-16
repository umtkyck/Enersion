import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * 8x8 Grid of Digital I/O Channels (64 total)
 */
Item {
    id: root
    
    property var channelStates: []  // Array of 64 booleans
    property bool isOutput: false
    property bool enabled: true
    
    signal channelClicked(int channel)
    
    implicitWidth: gridLayout.implicitWidth
    implicitHeight: gridLayout.implicitHeight
    
    GridLayout {
        id: gridLayout
        anchors.centerIn: parent
        
        columns: 8
        rowSpacing: Style.dioChannelSpacing
        columnSpacing: Style.dioChannelSpacing
        
        Repeater {
            model: 64
            
            DioChannel {
                channel: index
                active: channelStates[index] || false
                isOutput: root.isOutput
                enabled: root.enabled
                
                onClicked: root.channelClicked(index)
            }
        }
    }
    
    // Row labels (1-8)
    Column {
        anchors.right: gridLayout.left
        anchors.top: gridLayout.top
        anchors.rightMargin: Style.spacingM
        spacing: Style.dioChannelSpacing
        
        Repeater {
            model: 8
            
            Item {
                width: 24
                height: Style.dioChannelSize
                
                Text {
                    anchors.centerIn: parent
                    text: "R" + (index + 1)
                    font.pixelSize: Style.fontSizeS
                    font.weight: Font.Medium
                    color: Style.textMuted
                }
            }
        }
    }
    
    // Column labels (A-H)
    Row {
        anchors.bottom: gridLayout.top
        anchors.left: gridLayout.left
        anchors.bottomMargin: Style.spacingS
        spacing: Style.dioChannelSpacing
        
        Repeater {
            model: ["A", "B", "C", "D", "E", "F", "G", "H"]
            
            Item {
                width: Style.dioChannelSize
                height: 20
                
                Text {
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: Style.fontSizeS
                    font.weight: Font.Medium
                    color: Style.textMuted
                }
            }
        }
    }
}

