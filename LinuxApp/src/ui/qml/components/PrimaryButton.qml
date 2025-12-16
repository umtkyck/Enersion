import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * Primary Action Button
 */
Rectangle {
    id: root
    
    property string text: "Button"
    property string icon: ""
    property bool enabled: true
    property bool destructive: false
    property bool outline: false
    
    signal clicked()
    
    implicitWidth: buttonContent.implicitWidth + Style.spacingL * 2
    implicitHeight: Style.buttonHeight
    
    radius: Style.radiusM
    color: {
        if (!enabled) return Style.bgInput
        if (outline) return "transparent"
        if (destructive) return Style.error
        return Style.primary
    }
    border.width: outline ? 2 : 0
    border.color: {
        if (destructive) return Style.error
        return Style.primary
    }
    
    opacity: enabled ? 1.0 : 0.5
    
    Behavior on color {
        ColorAnimation { duration: Style.animFast }
    }
    
    Behavior on scale {
        NumberAnimation { duration: Style.animFast; easing.type: Easing.OutBack }
    }
    
    RowLayout {
        id: buttonContent
        anchors.centerIn: parent
        spacing: Style.spacingS
        
        Text {
            visible: icon !== ""
            text: icon
            font.pixelSize: Style.fontSizeL
            color: {
                if (outline) {
                    if (destructive) return Style.error
                    return Style.primary
                }
                return destructive ? "#FFFFFF" : "#000000"
            }
        }
        
        Text {
            text: root.text
            font.pixelSize: Style.fontSizeM
            font.weight: Font.Medium
            color: {
                if (outline) {
                    if (destructive) return Style.error
                    return Style.primary
                }
                return destructive ? "#FFFFFF" : "#000000"
            }
        }
    }
    
    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        cursorShape: Qt.PointingHandCursor
        
        onClicked: root.clicked()
        
        onPressed: root.scale = 0.96
        onReleased: root.scale = 1.0
        onCanceled: root.scale = 1.0
    }
    
    states: State {
        name: "hovered"
        when: hoverHandler.hovered && enabled
        PropertyChanges {
            target: root
            color: {
                if (outline) return Qt.rgba(0, 0.83, 0.67, 0.1)
                if (destructive) return Qt.lighter(Style.error, 1.1)
                return Style.primaryLight
            }
        }
    }
    
    HoverHandler {
        id: hoverHandler
    }
}

