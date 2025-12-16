import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * Single Digital I/O Channel Display
 * Used for both DI (read-only) and DO (interactive)
 */
Rectangle {
    id: root
    
    property int channel: 0
    property bool active: false
    property bool isOutput: false
    property bool enabled: true
    
    signal clicked()
    
    width: Style.dioChannelSize
    height: Style.dioChannelSize
    
    radius: Style.radiusM
    color: active ? (isOutput ? Style.doActive : Style.diActive) : Style.diInactive
    border.width: 2
    border.color: active ? Qt.darker(color, 1.2) : Style.border
    
    // Glow effect when active
    Rectangle {
        visible: active
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 4
        color: "transparent"
        border.width: 2
        border.color: parent.color
        opacity: 0.3
        
        SequentialAnimation on opacity {
            running: active
            loops: Animation.Infinite
            NumberAnimation { to: 0.1; duration: 1500; easing.type: Easing.InOutQuad }
            NumberAnimation { to: 0.3; duration: 1500; easing.type: Easing.InOutQuad }
        }
    }
    
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 2
        
        // Channel number
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: (channel + 1).toString().padStart(2, '0')
            font.pixelSize: Style.fontSizeL
            font.weight: Font.Bold
            font.family: "monospace"
            color: active ? "#000000" : Style.textSecondary
        }
        
        // State indicator
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: active ? "ON" : "OFF"
            font.pixelSize: Style.fontSizeXS
            font.weight: Font.Medium
            color: active ? "#000000" : Style.textMuted
            opacity: 0.8
        }
    }
    
    // Interaction (only for outputs)
    MouseArea {
        anchors.fill: parent
        enabled: isOutput && root.enabled
        cursorShape: isOutput ? Qt.PointingHandCursor : Qt.ArrowCursor
        
        onClicked: root.clicked()
        
        onPressed: {
            if (isOutput) {
                scaleAnim.to = 0.92
                scaleAnim.restart()
            }
        }
        
        onReleased: {
            if (isOutput) {
                scaleAnim.to = 1.0
                scaleAnim.restart()
            }
        }
    }
    
    // Press animation
    scale: 1.0
    NumberAnimation on scale {
        id: scaleAnim
        to: 1.0
        duration: Style.animFast
        easing.type: Easing.OutBack
    }
    
    // Color transition
    Behavior on color {
        ColorAnimation { duration: Style.animNormal }
    }
    
    Behavior on border.color {
        ColorAnimation { duration: Style.animNormal }
    }
    
    // Hover effect for outputs
    states: State {
        name: "hovered"
        when: isOutput && hoverHandler.hovered && !active
        PropertyChanges {
            target: root
            color: Style.doHover
        }
    }
    
    HoverHandler {
        id: hoverHandler
        enabled: isOutput
    }
}

