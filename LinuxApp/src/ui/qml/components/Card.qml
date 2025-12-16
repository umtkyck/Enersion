import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

/**
 * Reusable Card Component
 */
Rectangle {
    id: root
    
    property string title: ""
    property string icon: ""
    property bool hoverable: false
    
    default property alias content: contentContainer.data
    
    color: Style.bgCard
    radius: Style.radiusL
    border.width: 1
    border.color: Style.border
    
    Behavior on color {
        ColorAnimation { duration: Style.animFast }
    }
    
    Behavior on border.color {
        ColorAnimation { duration: Style.animFast }
    }
    
    states: State {
        name: "hovered"
        when: hoverable && hoverHandler.hovered
        PropertyChanges { 
            target: root
            color: Style.bgCardHover
            border.color: Style.primary
        }
    }
    
    HoverHandler {
        id: hoverHandler
        enabled: hoverable
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.spacingL
        spacing: Style.spacingM
        
        // === Header ===
        RowLayout {
            visible: title !== ""
            Layout.fillWidth: true
            spacing: Style.spacingS
            
            Text {
                visible: icon !== ""
                text: icon
                font.pixelSize: Style.fontSizeL
                color: Style.primary
            }
            
            Text {
                Layout.fillWidth: true
                text: title
                font.pixelSize: Style.fontSizeL
                font.weight: Font.Medium
                color: Style.textPrimary
            }
        }
        
        // === Content ===
        Item {
            id: contentContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}

