#!/usr/bin/env python3
"""
Enersion UI Theme - Modern Industrial Design

Color palette and styling for the application.
"""

from dataclasses import dataclass


@dataclass
class Theme:
    """Application theme colors and sizes"""
    
    # === Primary Colors ===
    primary: str = "#00D4AA"
    primary_dark: str = "#00A88A"
    primary_light: str = "#33DDBB"
    
    # === Background Colors ===
    bg_dark: str = "#0A0E14"
    bg_card: str = "#141B22"
    bg_card_hover: str = "#1A2530"
    bg_sidebar: str = "#0D1218"
    bg_input: str = "#1C252E"
    
    # === Text Colors ===
    text_primary: str = "#FFFFFF"
    text_secondary: str = "#8B9CAF"
    text_muted: str = "#5A6B7D"
    
    # === Status Colors ===
    success: str = "#00E676"
    warning: str = "#FFB300"
    error: str = "#FF5252"
    info: str = "#40C4FF"
    
    # === DI/DO Colors ===
    di_active: str = "#00E676"
    di_inactive: str = "#2D3A47"
    do_active: str = "#FF9100"
    do_inactive: str = "#2D3A47"
    do_hover: str = "#3D4A57"
    
    # === Border Colors ===
    border: str = "#2D3A47"
    border_active: str = "#00D4AA"
    
    # === Fonts ===
    font_family: str = "Segoe UI"
    font_family_mono: str = "Consolas"
    font_size_xs: int = 10
    font_size_s: int = 11
    font_size_m: int = 12
    font_size_l: int = 14
    font_size_xl: int = 18
    font_size_xxl: int = 24
    font_size_hero: int = 36
    
    # === Spacing ===
    spacing_xs: int = 4
    spacing_s: int = 8
    spacing_m: int = 16
    spacing_l: int = 24
    spacing_xl: int = 32
    
    # === Sizes ===
    sidebar_width: int = 220
    header_height: int = 60
    button_height: int = 40
    input_height: int = 40
    dio_channel_size: int = 56
    dio_spacing: int = 6
    
    # === Border Radius ===
    radius_s: int = 4
    radius_m: int = 8
    radius_l: int = 12


# Global theme instance
THEME = Theme()


def get_font(size: str = 'm', bold: bool = False, mono: bool = False) -> tuple:
    """Get font tuple for tkinter"""
    family = THEME.font_family_mono if mono else THEME.font_family
    
    sizes = {
        'xs': THEME.font_size_xs,
        's': THEME.font_size_s,
        'm': THEME.font_size_m,
        'l': THEME.font_size_l,
        'xl': THEME.font_size_xl,
        'xxl': THEME.font_size_xxl,
        'hero': THEME.font_size_hero,
    }
    
    font_size = sizes.get(size, THEME.font_size_m)
    weight = 'bold' if bold else 'normal'
    
    return (family, font_size, weight)

