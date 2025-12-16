pragma Singleton
import QtQuick 2.15

/**
 * Enersion Design System - Modern Industrial UI
 * Color palette inspired by high-end SCADA systems
 */
QtObject {
    // === Primary Colors ===
    readonly property color primary: "#00D4AA"          // Teal accent
    readonly property color primaryDark: "#00A88A"
    readonly property color primaryLight: "#33DDBB"
    
    // === Background Colors ===
    readonly property color bgDark: "#0A0E14"           // Deep navy
    readonly property color bgCard: "#141B22"           // Card background
    readonly property color bgCardHover: "#1A2530"
    readonly property color bgSidebar: "#0D1218"
    readonly property color bgInput: "#1C252E"
    
    // === Text Colors ===
    readonly property color textPrimary: "#FFFFFF"
    readonly property color textSecondary: "#8B9CAF"
    readonly property color textMuted: "#5A6B7D"
    
    // === Status Colors ===
    readonly property color success: "#00E676"
    readonly property color warning: "#FFB300"
    readonly property color error: "#FF5252"
    readonly property color info: "#40C4FF"
    
    // === DI/DO Colors ===
    readonly property color diActive: "#00E676"         // Green - Input ON
    readonly property color diInactive: "#2D3A47"       // Dark - Input OFF
    readonly property color doActive: "#FF9100"         // Orange - Output ON
    readonly property color doInactive: "#2D3A47"       // Dark - Output OFF
    readonly property color doHover: "#3D4A57"
    
    // === Border & Shadows ===
    readonly property color border: "#2D3A47"
    readonly property color borderActive: "#00D4AA"
    readonly property color shadow: "#000000"
    
    // === Typography ===
    readonly property string fontFamily: "Segoe UI, Roboto, Ubuntu, sans-serif"
    readonly property int fontSizeXS: 11
    readonly property int fontSizeS: 12
    readonly property int fontSizeM: 14
    readonly property int fontSizeL: 16
    readonly property int fontSizeXL: 20
    readonly property int fontSizeXXL: 28
    readonly property int fontSizeHero: 48
    
    // === Spacing ===
    readonly property int spacingXS: 4
    readonly property int spacingS: 8
    readonly property int spacingM: 16
    readonly property int spacingL: 24
    readonly property int spacingXL: 32
    readonly property int spacingXXL: 48
    
    // === Border Radius ===
    readonly property int radiusS: 4
    readonly property int radiusM: 8
    readonly property int radiusL: 12
    readonly property int radiusXL: 16
    readonly property int radiusRound: 999
    
    // === Animation ===
    readonly property int animFast: 150
    readonly property int animNormal: 250
    readonly property int animSlow: 400
    
    // === Sidebar ===
    readonly property int sidebarWidth: 240
    readonly property int sidebarCollapsedWidth: 72
    
    // === Header ===
    readonly property int headerHeight: 64
    
    // === Touch targets ===
    readonly property int touchMinSize: 48
    readonly property int buttonHeight: 44
    readonly property int inputHeight: 48
    
    // === Grid ===
    readonly property int dioGridColumns: 8
    readonly property int dioChannelSize: 64
    readonly property int dioChannelSpacing: 8
}

