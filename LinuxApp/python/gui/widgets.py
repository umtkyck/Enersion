#!/usr/bin/env python3
"""
Custom Widgets for Enersion GUI

Modern styled widgets for tkinter.
"""

import tkinter as tk
from tkinter import ttk
from typing import Callable, Optional, List
from .theme import THEME, get_font


class ModernButton(tk.Canvas):
    """Modern styled button with hover effects"""
    
    def __init__(self, parent, text: str = "", icon: str = "",
                 command: Optional[Callable] = None,
                 primary: bool = True, destructive: bool = False,
                 outline: bool = False, width: int = 120, **kwargs):
        
        height = THEME.button_height
        super().__init__(parent, width=width, height=height,
                        bg=THEME.bg_card, highlightthickness=0, **kwargs)
        
        self.text = text
        self.icon = icon
        self.command = command
        self.primary = primary
        self.destructive = destructive
        self.outline = outline
        self._width = width
        self._height = height
        self._enabled = True
        self._hovered = False
        
        self._draw()
        
        self.bind('<Enter>', self._on_enter)
        self.bind('<Leave>', self._on_leave)
        self.bind('<Button-1>', self._on_click)
        self.bind('<ButtonRelease-1>', self._on_release)
    
    def _get_colors(self) -> tuple:
        """Get background and text colors based on state"""
        if not self._enabled:
            return THEME.bg_input, THEME.text_muted
        
        if self.outline:
            color = THEME.error if self.destructive else THEME.primary
            bg = THEME.bg_card_hover if self._hovered else THEME.bg_card
            return bg, color
        
        if self.destructive:
            bg = "#FF6B6B" if self._hovered else THEME.error
            return bg, "#FFFFFF"
        
        bg = THEME.primary_light if self._hovered else THEME.primary
        return bg, "#000000"
    
    def _draw(self):
        """Draw the button"""
        self.delete('all')
        
        bg_color, text_color = self._get_colors()
        border_color = THEME.error if self.destructive else THEME.primary
        
        # Background
        r = THEME.radius_m
        self.create_rounded_rect(2, 2, self._width - 2, self._height - 2,
                                 r, fill=bg_color,
                                 outline=border_color if self.outline else bg_color,
                                 width=2)
        
        # Text with icon
        display_text = f"{self.icon} {self.text}".strip()
        self.create_text(self._width // 2, self._height // 2,
                        text=display_text, fill=text_color,
                        font=get_font('m', bold=True))
    
    def create_rounded_rect(self, x1, y1, x2, y2, r, **kwargs):
        """Draw a rounded rectangle"""
        points = [
            x1 + r, y1,
            x2 - r, y1,
            x2, y1,
            x2, y1 + r,
            x2, y2 - r,
            x2, y2,
            x2 - r, y2,
            x1 + r, y2,
            x1, y2,
            x1, y2 - r,
            x1, y1 + r,
            x1, y1,
            x1 + r, y1,
        ]
        return self.create_polygon(points, smooth=True, **kwargs)
    
    def _on_enter(self, event):
        self._hovered = True
        self._draw()
    
    def _on_leave(self, event):
        self._hovered = False
        self._draw()
    
    def _on_click(self, event):
        if self._enabled:
            self.configure(cursor='hand2')
    
    def _on_release(self, event):
        if self._enabled and self.command:
            self.command()
    
    def set_enabled(self, enabled: bool):
        self._enabled = enabled
        self._draw()


class DIOChannel(tk.Canvas):
    """Single Digital I/O channel display/control"""
    
    def __init__(self, parent, channel: int, is_output: bool = False,
                 command: Optional[Callable] = None, **kwargs):
        
        size = THEME.dio_channel_size
        super().__init__(parent, width=size, height=size,
                        bg=THEME.bg_card, highlightthickness=0, **kwargs)
        
        self.channel = channel
        self.is_output = is_output
        self.command = command
        self._active = False
        self._hovered = False
        self._size = size
        
        self._draw()
        
        if is_output:
            self.bind('<Enter>', self._on_enter)
            self.bind('<Leave>', self._on_leave)
            self.bind('<Button-1>', self._on_click)
    
    def _get_color(self) -> str:
        """Get channel color based on state"""
        if self._active:
            return THEME.do_active if self.is_output else THEME.di_active
        elif self._hovered and self.is_output:
            return THEME.do_hover
        else:
            return THEME.di_inactive
    
    def _draw(self):
        """Draw the channel"""
        self.delete('all')
        
        color = self._get_color()
        r = THEME.radius_m
        
        # Background
        self.create_rounded_rect(2, 2, self._size - 2, self._size - 2,
                                 r, fill=color, outline=THEME.border, width=1)
        
        # Channel number
        text_color = "#000000" if self._active else THEME.text_secondary
        self.create_text(self._size // 2, self._size // 2 - 6,
                        text=f"{self.channel + 1:02d}",
                        fill=text_color,
                        font=get_font('l', bold=True, mono=True))
        
        # State text
        state_text = "ON" if self._active else "OFF"
        self.create_text(self._size // 2, self._size // 2 + 12,
                        text=state_text,
                        fill=text_color,
                        font=get_font('xs'))
    
    def create_rounded_rect(self, x1, y1, x2, y2, r, **kwargs):
        """Draw a rounded rectangle"""
        points = [
            x1 + r, y1, x2 - r, y1, x2, y1, x2, y1 + r,
            x2, y2 - r, x2, y2, x2 - r, y2, x1 + r, y2,
            x1, y2, x1, y2 - r, x1, y1 + r, x1, y1, x1 + r, y1,
        ]
        return self.create_polygon(points, smooth=True, **kwargs)
    
    def set_active(self, active: bool):
        """Set channel state"""
        if self._active != active:
            self._active = active
            self._draw()
    
    def get_active(self) -> bool:
        """Get channel state"""
        return self._active
    
    def _on_enter(self, event):
        self._hovered = True
        self.configure(cursor='hand2')
        self._draw()
    
    def _on_leave(self, event):
        self._hovered = False
        self._draw()
    
    def _on_click(self, event):
        if self.is_output and self.command:
            self.command(self.channel)


class DIOGrid(tk.Frame):
    """8x8 Grid of DIO channels (64 total)"""
    
    def __init__(self, parent, is_output: bool = False,
                 on_channel_click: Optional[Callable] = None, **kwargs):
        
        super().__init__(parent, bg=THEME.bg_card, **kwargs)
        
        self.is_output = is_output
        self.on_channel_click = on_channel_click
        self.channels: List[DIOChannel] = []
        
        self._create_grid()
    
    def _create_grid(self):
        """Create 8x8 grid of channels"""
        spacing = THEME.dio_spacing
        
        # Column labels
        label_frame = tk.Frame(self, bg=THEME.bg_card)
        label_frame.grid(row=0, column=1, sticky='w', padx=(30, 0))
        
        for col, label in enumerate(['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H']):
            lbl = tk.Label(label_frame, text=label,
                          font=get_font('s'), fg=THEME.text_muted,
                          bg=THEME.bg_card,
                          width=THEME.dio_channel_size // 8)
            lbl.grid(row=0, column=col, padx=(spacing, 0))
        
        # Grid
        grid_frame = tk.Frame(self, bg=THEME.bg_card)
        grid_frame.grid(row=1, column=0, columnspan=2)
        
        for row in range(8):
            # Row label
            row_label = tk.Label(grid_frame, text=f"R{row + 1}",
                                font=get_font('s'), fg=THEME.text_muted,
                                bg=THEME.bg_card, width=3)
            row_label.grid(row=row, column=0, padx=(0, spacing))
            
            for col in range(8):
                channel_idx = row * 8 + col
                
                channel = DIOChannel(
                    grid_frame,
                    channel=channel_idx,
                    is_output=self.is_output,
                    command=self._on_click if self.is_output else None
                )
                channel.grid(row=row, column=col + 1,
                            padx=spacing // 2, pady=spacing // 2)
                
                self.channels.append(channel)
    
    def _on_click(self, channel: int):
        """Handle channel click"""
        if self.on_channel_click:
            self.on_channel_click(channel)
    
    def set_states(self, states: List[bool]):
        """Set all channel states"""
        for i, state in enumerate(states[:64]):
            self.channels[i].set_active(state)
    
    def get_states(self) -> List[bool]:
        """Get all channel states"""
        return [ch.get_active() for ch in self.channels]
    
    def set_channel(self, channel: int, state: bool):
        """Set single channel state"""
        if 0 <= channel < 64:
            self.channels[channel].set_active(state)
    
    def toggle_channel(self, channel: int):
        """Toggle single channel"""
        if 0 <= channel < 64:
            current = self.channels[channel].get_active()
            self.channels[channel].set_active(not current)


class StatusIndicator(tk.Canvas):
    """Connection status indicator with pulse animation"""
    
    def __init__(self, parent, size: int = 12, **kwargs):
        super().__init__(parent, width=size, height=size,
                        bg=THEME.bg_sidebar, highlightthickness=0, **kwargs)
        
        self._size = size
        self._connected = False
        self._pulse = False
        
        self._draw()
    
    def _draw(self):
        """Draw the indicator"""
        self.delete('all')
        
        color = THEME.success if self._connected else THEME.error
        r = self._size // 2
        
        self.create_oval(1, 1, self._size - 1, self._size - 1,
                        fill=color, outline='')
    
    def set_connected(self, connected: bool):
        """Set connection state"""
        self._connected = connected
        self._draw()


class Card(tk.Frame):
    """Card container with title"""
    
    def __init__(self, parent, title: str = "", icon: str = "", **kwargs):
        super().__init__(parent, bg=THEME.bg_card, **kwargs)
        
        self.title = title
        self.icon = icon
        
        self.configure(
            highlightbackground=THEME.border,
            highlightcolor=THEME.border,
            highlightthickness=1
        )
        
        if title:
            # Header
            header = tk.Frame(self, bg=THEME.bg_card)
            header.pack(fill='x', padx=THEME.spacing_m, pady=(THEME.spacing_m, THEME.spacing_s))
            
            if icon:
                icon_label = tk.Label(header, text=icon, font=get_font('l'),
                                     fg=THEME.primary, bg=THEME.bg_card)
                icon_label.pack(side='left')
            
            title_label = tk.Label(header, text=title, font=get_font('l', bold=True),
                                  fg=THEME.text_primary, bg=THEME.bg_card)
            title_label.pack(side='left', padx=(THEME.spacing_s if icon else 0, 0))
        
        # Content frame
        self.content = tk.Frame(self, bg=THEME.bg_card)
        self.content.pack(fill='both', expand=True, padx=THEME.spacing_m,
                         pady=(0, THEME.spacing_m))

