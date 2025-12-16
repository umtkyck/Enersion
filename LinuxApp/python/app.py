#!/usr/bin/env python3
"""
Enersion Control System - Main Application

Modern GUI for Digital I/O control on MYIR STM32MP257.

Author: Enersion
Version: 1.0.0
"""

import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time
import logging
from typing import Optional, Callable
from dataclasses import dataclass

# Local imports
from gui.theme import THEME, get_font
from gui.widgets import ModernButton, DIOGrid, StatusIndicator, Card
from hal.rs485 import RS485, RS485Config, RS485Error, list_serial_ports
from protocol.enersion import EnersionProtocol, Address, DIOState

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


class NavigationBar(tk.Frame):
    """Left sidebar navigation"""
    
    def __init__(self, parent, on_navigate: Callable, **kwargs):
        super().__init__(parent, bg=THEME.bg_sidebar, width=THEME.sidebar_width, **kwargs)
        
        self.on_navigate = on_navigate
        self.current_page = 0
        self.buttons = []
        
        self.pack_propagate(False)
        self._create_ui()
    
    def _create_ui(self):
        # Logo
        logo_frame = tk.Frame(self, bg=THEME.bg_sidebar)
        logo_frame.pack(fill='x', pady=THEME.spacing_l)
        
        logo_icon = tk.Label(logo_frame, text="⚡", font=get_font('hero'),
                            fg=THEME.primary, bg=THEME.bg_sidebar)
        logo_icon.pack()
        
        logo_text = tk.Label(logo_frame, text="ENERSION", font=get_font('l', bold=True),
                            fg=THEME.text_primary, bg=THEME.bg_sidebar)
        logo_text.pack()
        
        # Divider
        divider = tk.Frame(self, bg=THEME.border, height=1)
        divider.pack(fill='x', padx=THEME.spacing_l, pady=THEME.spacing_m)
        
        # Navigation items
        pages = [
            ("⌂", "Dashboard"),
            ("▣", "Digital Inputs"),
            ("◧", "Digital Outputs"),
            ("⚙", "Settings")
        ]
        
        for i, (icon, name) in enumerate(pages):
            btn = self._create_nav_button(icon, name, i)
            self.buttons.append(btn)
        
        # Spacer
        spacer = tk.Frame(self, bg=THEME.bg_sidebar)
        spacer.pack(fill='both', expand=True)
        
        # Connection status
        self.status_frame = tk.Frame(self, bg=THEME.bg_card)
        self.status_frame.pack(fill='x', padx=THEME.spacing_m, pady=THEME.spacing_m)
        
        status_inner = tk.Frame(self.status_frame, bg=THEME.bg_card)
        status_inner.pack(fill='x', padx=THEME.spacing_m, pady=THEME.spacing_m)
        
        self.status_indicator = StatusIndicator(status_inner)
        self.status_indicator.pack(side='left')
        
        status_text_frame = tk.Frame(status_inner, bg=THEME.bg_card)
        status_text_frame.pack(side='left', padx=(THEME.spacing_s, 0))
        
        self.status_label = tk.Label(status_text_frame, text="Disconnected",
                                    font=get_font('s', bold=True),
                                    fg=THEME.text_primary, bg=THEME.bg_card)
        self.status_label.pack(anchor='w')
        
        self.port_label = tk.Label(status_text_frame, text="No device",
                                  font=get_font('xs'),
                                  fg=THEME.text_muted, bg=THEME.bg_card)
        self.port_label.pack(anchor='w')
    
    def _create_nav_button(self, icon: str, name: str, index: int) -> tk.Frame:
        """Create a navigation button"""
        frame = tk.Frame(self, bg=THEME.bg_sidebar, cursor='hand2')
        frame.pack(fill='x', padx=THEME.spacing_m, pady=2)
        
        # Active indicator
        indicator = tk.Frame(frame, bg=THEME.primary if index == 0 else THEME.bg_sidebar,
                            width=3)
        indicator.pack(side='left', fill='y', padx=(0, THEME.spacing_s))
        
        # Content
        content = tk.Frame(frame, bg=THEME.bg_sidebar)
        content.pack(side='left', fill='x', expand=True, pady=THEME.spacing_m)
        
        icon_label = tk.Label(content, text=icon, font=get_font('l'),
                             fg=THEME.primary if index == 0 else THEME.text_secondary,
                             bg=THEME.bg_sidebar)
        icon_label.pack(side='left')
        
        text_label = tk.Label(content, text=name, font=get_font('m'),
                             fg=THEME.text_primary if index == 0 else THEME.text_secondary,
                             bg=THEME.bg_sidebar)
        text_label.pack(side='left', padx=(THEME.spacing_s, 0))
        
        # Store references
        frame.indicator = indicator
        frame.icon_label = icon_label
        frame.text_label = text_label
        frame.index = index
        
        # Bind click
        for widget in [frame, content, icon_label, text_label]:
            widget.bind('<Button-1>', lambda e, idx=index: self._on_click(idx))
        
        return frame
    
    def _on_click(self, index: int):
        """Handle navigation click"""
        self.set_active(index)
        self.on_navigate(index)
    
    def set_active(self, index: int):
        """Set active page"""
        self.current_page = index
        
        for i, btn in enumerate(self.buttons):
            is_active = i == index
            color = THEME.primary if is_active else THEME.bg_sidebar
            text_color = THEME.text_primary if is_active else THEME.text_secondary
            icon_color = THEME.primary if is_active else THEME.text_secondary
            
            btn.indicator.configure(bg=color)
            btn.text_label.configure(fg=text_color)
            btn.icon_label.configure(fg=icon_color)
    
    def set_connected(self, connected: bool, port: str = ""):
        """Update connection status"""
        self.status_indicator.set_connected(connected)
        self.status_label.configure(text="Connected" if connected else "Disconnected")
        self.port_label.configure(text=port if connected else "No device")


class TopBar(tk.Frame):
    """Top header bar"""
    
    def __init__(self, parent, **kwargs):
        super().__init__(parent, bg=THEME.bg_dark, height=THEME.header_height, **kwargs)
        
        self.pack_propagate(False)
        self._create_ui()
    
    def _create_ui(self):
        # Title
        self.title_label = tk.Label(self, text="Dashboard",
                                   font=get_font('xxl', bold=True),
                                   fg=THEME.text_primary, bg=THEME.bg_dark)
        self.title_label.pack(side='left', padx=THEME.spacing_l)
        
        # Right side - status pills
        right_frame = tk.Frame(self, bg=THEME.bg_dark)
        right_frame.pack(side='right', padx=THEME.spacing_l)
        
        # Time
        self.time_label = tk.Label(right_frame, text="00:00:00",
                                  font=get_font('m', mono=True),
                                  fg=THEME.text_secondary, bg=THEME.bg_dark)
        self.time_label.pack(side='right')
        
        # DO count
        self.do_pill = self._create_pill(right_frame, "DO", "0/64", THEME.do_active)
        self.do_pill.pack(side='right', padx=THEME.spacing_m)
        
        # DI count
        self.di_pill = self._create_pill(right_frame, "DI", "0/64", THEME.di_active)
        self.di_pill.pack(side='right', padx=THEME.spacing_m)
        
        # Update time
        self._update_time()
    
    def _create_pill(self, parent, label: str, value: str, color: str) -> tk.Frame:
        """Create a status pill"""
        frame = tk.Frame(parent, bg=THEME.bg_card)
        frame.configure(highlightbackground=THEME.border, highlightthickness=1)
        
        inner = tk.Frame(frame, bg=THEME.bg_card)
        inner.pack(padx=THEME.spacing_m, pady=THEME.spacing_xs)
        
        dot = tk.Canvas(inner, width=8, height=8, bg=THEME.bg_card, highlightthickness=0)
        dot.create_oval(0, 0, 8, 8, fill=color)
        dot.pack(side='left')
        
        label_widget = tk.Label(inner, text=label, font=get_font('s'),
                               fg=THEME.text_secondary, bg=THEME.bg_card)
        label_widget.pack(side='left', padx=(THEME.spacing_xs, 0))
        
        value_label = tk.Label(inner, text=value, font=get_font('s', bold=True),
                              fg=THEME.text_primary, bg=THEME.bg_card)
        value_label.pack(side='left', padx=(THEME.spacing_xs, 0))
        
        frame.value_label = value_label
        frame.dot = dot
        frame.color = color
        
        return frame
    
    def _update_time(self):
        """Update time display"""
        current_time = time.strftime("%H:%M:%S")
        self.time_label.configure(text=current_time)
        self.after(1000, self._update_time)
    
    def set_title(self, title: str):
        """Set page title"""
        self.title_label.configure(text=title)
    
    def set_di_count(self, count: int):
        """Set DI active count"""
        self.di_pill.value_label.configure(text=f"{count}/64")
    
    def set_do_count(self, count: int):
        """Set DO active count"""
        self.do_pill.value_label.configure(text=f"{count}/64")


class DashboardPage(tk.Frame):
    """Dashboard overview page"""
    
    def __init__(self, parent, app, **kwargs):
        super().__init__(parent, bg=THEME.bg_dark, **kwargs)
        
        self.app = app
        self._create_ui()
    
    def _create_ui(self):
        # Welcome section
        welcome_frame = tk.Frame(self, bg=THEME.bg_dark)
        welcome_frame.pack(fill='x', pady=THEME.spacing_l)
        
        welcome_title = tk.Label(welcome_frame, text="Welcome to Enersion",
                                font=get_font('hero', bold=True),
                                fg=THEME.text_primary, bg=THEME.bg_dark)
        welcome_title.pack(anchor='w')
        
        welcome_sub = tk.Label(welcome_frame,
                              text="Industrial Control System for Digital I/O Management",
                              font=get_font('l'), fg=THEME.text_secondary, bg=THEME.bg_dark)
        welcome_sub.pack(anchor='w')
        
        # Stats row
        stats_frame = tk.Frame(self, bg=THEME.bg_dark)
        stats_frame.pack(fill='x', pady=THEME.spacing_l)
        
        # DI Card
        di_card = Card(stats_frame, title="Digital Inputs", icon="▣")
        di_card.pack(side='left', fill='both', expand=True, padx=(0, THEME.spacing_m))
        
        self.di_count_label = tk.Label(di_card.content, text="0",
                                       font=("Segoe UI", 48, "bold"),
                                       fg=THEME.di_active, bg=THEME.bg_card)
        self.di_count_label.pack(side='left')
        
        tk.Label(di_card.content, text="/ 64", font=get_font('xxl'),
                fg=THEME.text_muted, bg=THEME.bg_card).pack(side='left')
        
        # DO Card
        do_card = Card(stats_frame, title="Digital Outputs", icon="◧")
        do_card.pack(side='left', fill='both', expand=True)
        
        self.do_count_label = tk.Label(do_card.content, text="0",
                                       font=("Segoe UI", 48, "bold"),
                                       fg=THEME.do_active, bg=THEME.bg_card)
        self.do_count_label.pack(side='left')
        
        tk.Label(do_card.content, text="/ 64", font=get_font('xxl'),
                fg=THEME.text_muted, bg=THEME.bg_card).pack(side='left')
        
        # Quick actions
        actions_card = Card(self, title="Quick Actions", icon="⚡")
        actions_card.pack(fill='x', pady=THEME.spacing_l)
        
        btn_frame = tk.Frame(actions_card.content, bg=THEME.bg_card)
        btn_frame.pack(fill='x')
        
        ModernButton(btn_frame, text="View Inputs", icon="▣",
                    command=lambda: self.app.navigate(1)).pack(side='left', padx=THEME.spacing_xs)
        ModernButton(btn_frame, text="Control Outputs", icon="◧",
                    command=lambda: self.app.navigate(2)).pack(side='left', padx=THEME.spacing_xs)
        ModernButton(btn_frame, text="All OFF", icon="○", outline=True,
                    command=self.app.all_outputs_off).pack(side='left', padx=THEME.spacing_xs)
        ModernButton(btn_frame, text="All ON", icon="●", outline=True,
                    command=self.app.all_outputs_on).pack(side='left', padx=THEME.spacing_xs)
    
    def update_counts(self, di_count: int, do_count: int):
        """Update DI/DO counts"""
        self.di_count_label.configure(text=str(di_count))
        self.do_count_label.configure(text=str(do_count))


class DigitalInputPage(tk.Frame):
    """Digital Input monitoring page"""
    
    def __init__(self, parent, app, **kwargs):
        super().__init__(parent, bg=THEME.bg_dark, **kwargs)
        
        self.app = app
        self._create_ui()
    
    def _create_ui(self):
        # Header
        header = tk.Frame(self, bg=THEME.bg_dark)
        header.pack(fill='x', pady=THEME.spacing_m)
        
        tk.Label(header, text="Digital Input Monitor",
                font=get_font('xl', bold=True),
                fg=THEME.text_primary, bg=THEME.bg_dark).pack(side='left')
        
        self.active_label = tk.Label(header, text="0 Active",
                                    font=get_font('m'),
                                    fg=THEME.di_active, bg=THEME.bg_dark)
        self.active_label.pack(side='right')
        
        # Legend
        legend = tk.Frame(self, bg=THEME.bg_dark)
        legend.pack(fill='x', pady=THEME.spacing_s)
        
        self._create_legend_item(legend, THEME.di_active, "Input HIGH (Active)")
        self._create_legend_item(legend, THEME.di_inactive, "Input LOW (Inactive)")
        
        # Grid
        grid_card = Card(self)
        grid_card.pack(fill='both', expand=True, pady=THEME.spacing_m)
        
        self.grid = DIOGrid(grid_card.content, is_output=False)
        self.grid.pack(expand=True)
    
    def _create_legend_item(self, parent, color: str, text: str):
        """Create legend item"""
        frame = tk.Frame(parent, bg=THEME.bg_dark)
        frame.pack(side='left', padx=THEME.spacing_m)
        
        box = tk.Canvas(frame, width=16, height=16, bg=THEME.bg_dark, highlightthickness=0)
        box.create_rectangle(0, 0, 16, 16, fill=color, outline=THEME.border)
        box.pack(side='left')
        
        tk.Label(frame, text=text, font=get_font('s'),
                fg=THEME.text_secondary, bg=THEME.bg_dark).pack(side='left', padx=(THEME.spacing_xs, 0))
    
    def update_states(self, states: list):
        """Update DI states"""
        self.grid.set_states(states)
        active = sum(states)
        self.active_label.configure(text=f"{active} Active")


class DigitalOutputPage(tk.Frame):
    """Digital Output control page"""
    
    def __init__(self, parent, app, **kwargs):
        super().__init__(parent, bg=THEME.bg_dark, **kwargs)
        
        self.app = app
        self._create_ui()
    
    def _create_ui(self):
        # Header
        header = tk.Frame(self, bg=THEME.bg_dark)
        header.pack(fill='x', pady=THEME.spacing_m)
        
        tk.Label(header, text="Digital Output Control",
                font=get_font('xl', bold=True),
                fg=THEME.text_primary, bg=THEME.bg_dark).pack(side='left')
        
        self.active_label = tk.Label(header, text="0 Active",
                                    font=get_font('m'),
                                    fg=THEME.do_active, bg=THEME.bg_dark)
        self.active_label.pack(side='right')
        
        # Control bar
        control_card = Card(self)
        control_card.pack(fill='x', pady=THEME.spacing_m)
        
        btn_frame = tk.Frame(control_card.content, bg=THEME.bg_card)
        btn_frame.pack(fill='x')
        
        tk.Label(btn_frame, text="Quick Actions:", font=get_font('m'),
                fg=THEME.text_secondary, bg=THEME.bg_card).pack(side='left')
        
        ModernButton(btn_frame, text="All ON", icon="●", width=100,
                    command=self.app.all_outputs_on).pack(side='left', padx=THEME.spacing_xs)
        ModernButton(btn_frame, text="All OFF", icon="○", width=100,
                    destructive=True, command=self.app.all_outputs_off).pack(side='left', padx=THEME.spacing_xs)
        ModernButton(btn_frame, text="Alternate", width=100, outline=True,
                    command=lambda: self.app.set_pattern(0xAA)).pack(side='left', padx=THEME.spacing_xs)
        
        # Legend
        legend = tk.Frame(self, bg=THEME.bg_dark)
        legend.pack(fill='x', pady=THEME.spacing_s)
        
        self._create_legend_item(legend, THEME.do_active, "Output ON")
        self._create_legend_item(legend, THEME.do_inactive, "Output OFF")
        
        tk.Label(legend, text="💡 Click any channel to toggle",
                font=get_font('s'), fg=THEME.text_muted,
                bg=THEME.bg_dark).pack(side='left', padx=THEME.spacing_l)
        
        # Grid
        grid_card = Card(self)
        grid_card.pack(fill='both', expand=True, pady=THEME.spacing_m)
        
        self.grid = DIOGrid(grid_card.content, is_output=True,
                           on_channel_click=self._on_channel_click)
        self.grid.pack(expand=True)
    
    def _create_legend_item(self, parent, color: str, text: str):
        """Create legend item"""
        frame = tk.Frame(parent, bg=THEME.bg_dark)
        frame.pack(side='left', padx=THEME.spacing_m)
        
        box = tk.Canvas(frame, width=16, height=16, bg=THEME.bg_dark, highlightthickness=0)
        box.create_rectangle(0, 0, 16, 16, fill=color, outline=THEME.border)
        box.pack(side='left')
        
        tk.Label(frame, text=text, font=get_font('s'),
                fg=THEME.text_secondary, bg=THEME.bg_dark).pack(side='left', padx=(THEME.spacing_xs, 0))
    
    def _on_channel_click(self, channel: int):
        """Handle channel toggle"""
        self.app.toggle_output(channel)
    
    def update_states(self, states: list):
        """Update DO states"""
        self.grid.set_states(states)
        active = sum(states)
        self.active_label.configure(text=f"{active} Active")


class SettingsPage(tk.Frame):
    """Settings and connection page"""
    
    def __init__(self, parent, app, **kwargs):
        super().__init__(parent, bg=THEME.bg_dark, **kwargs)
        
        self.app = app
        self._create_ui()
    
    def _create_ui(self):
        # Connection card
        conn_card = Card(self, title="Connection Settings", icon="🔌")
        conn_card.pack(fill='x', pady=THEME.spacing_m)
        
        # Port selection
        port_frame = tk.Frame(conn_card.content, bg=THEME.bg_card)
        port_frame.pack(fill='x', pady=THEME.spacing_s)
        
        tk.Label(port_frame, text="Serial Port:", font=get_font('s'),
                fg=THEME.text_secondary, bg=THEME.bg_card).pack(side='left')
        
        self.port_var = tk.StringVar(value="/dev/ttySTM9")
        self.port_combo = ttk.Combobox(port_frame, textvariable=self.port_var,
                                       values=list_serial_ports(), width=30)
        self.port_combo.pack(side='left', padx=THEME.spacing_m)
        
        tk.Label(port_frame, text="Baud Rate:", font=get_font('s'),
                fg=THEME.text_secondary, bg=THEME.bg_card).pack(side='left', padx=(THEME.spacing_l, 0))
        
        self.baud_var = tk.StringVar(value="115200")
        self.baud_combo = ttk.Combobox(port_frame, textvariable=self.baud_var,
                                       values=["9600", "19200", "38400", "57600", "115200", "230400"],
                                       width=10)
        self.baud_combo.pack(side='left', padx=THEME.spacing_m)
        
        # Buttons
        btn_frame = tk.Frame(conn_card.content, bg=THEME.bg_card)
        btn_frame.pack(fill='x', pady=THEME.spacing_m)
        
        self.connect_btn = ModernButton(btn_frame, text="Connect", icon="→",
                                       command=self._on_connect, width=140)
        self.connect_btn.pack(side='left')
        
        ModernButton(btn_frame, text="Refresh Ports", icon="⟳", outline=True,
                    command=self._refresh_ports, width=140).pack(side='left', padx=THEME.spacing_m)
        
        # Device info
        info_card = Card(self, title="Device Information", icon="ℹ")
        info_card.pack(fill='x', pady=THEME.spacing_m)
        
        info_grid = tk.Frame(info_card.content, bg=THEME.bg_card)
        info_grid.pack(fill='x')
        
        info_items = [
            ("Board", "MYIR STM32MP257"),
            ("RS485 Port", "/dev/ttySTM9"),
            ("GPIO (Direction)", "PI10 (138)"),
            ("DI Controller", "Address 0x02"),
            ("DO Controller", "Address 0x03"),
            ("Protocol", "Enersion v1.0"),
        ]
        
        for i, (label, value) in enumerate(info_items):
            row = i // 3
            col = i % 3
            
            frame = tk.Frame(info_grid, bg=THEME.bg_card)
            frame.grid(row=row, column=col, padx=THEME.spacing_l, pady=THEME.spacing_s, sticky='w')
            
            tk.Label(frame, text=label, font=get_font('s'),
                    fg=THEME.text_muted, bg=THEME.bg_card).pack(anchor='w')
            tk.Label(frame, text=value, font=get_font('m', bold=True),
                    fg=THEME.text_primary, bg=THEME.bg_card).pack(anchor='w')
        
        # About
        about_card = Card(self, title="About", icon="⚡")
        about_card.pack(fill='x', pady=THEME.spacing_m)
        
        about_frame = tk.Frame(about_card.content, bg=THEME.bg_card)
        about_frame.pack(fill='x')
        
        tk.Label(about_frame, text="Enersion Control System",
                font=get_font('l', bold=True), fg=THEME.primary,
                bg=THEME.bg_card).pack(side='left')
        tk.Label(about_frame, text="v1.0.0", font=get_font('m'),
                fg=THEME.text_secondary, bg=THEME.bg_card).pack(side='left', padx=THEME.spacing_m)
        tk.Label(about_frame, text="© 2024 Enersion", font=get_font('s'),
                fg=THEME.text_muted, bg=THEME.bg_card).pack(side='right')
    
    def _on_connect(self):
        """Handle connect/disconnect"""
        if self.app.is_connected:
            self.app.disconnect()
            self.connect_btn.text = "Connect"
            self.connect_btn.destructive = False
            self.connect_btn._draw()
        else:
            port = self.port_var.get()
            baudrate = int(self.baud_var.get())
            if self.app.connect(port, baudrate):
                self.connect_btn.text = "Disconnect"
                self.connect_btn.destructive = True
                self.connect_btn._draw()
    
    def _refresh_ports(self):
        """Refresh port list"""
        ports = list_serial_ports()
        self.port_combo['values'] = ports


class EnersionApp:
    """Main Application"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Enersion Control System")
        self.root.geometry("1280x800")
        self.root.minsize(1024, 600)
        self.root.configure(bg=THEME.bg_dark)
        
        # State
        self.is_connected = False
        self.rs485: Optional[RS485] = None
        self.protocol: Optional[EnersionProtocol] = None
        self.polling = False
        self.poll_thread: Optional[threading.Thread] = None
        
        # DIO states
        self.di_states = [False] * 64
        self.do_states = [False] * 64
        
        # Create UI
        self._create_ui()
        
        # Page names
        self.page_titles = ["Dashboard", "Digital Inputs", "Digital Outputs", "Settings"]
    
    def _create_ui(self):
        # Main container
        main_container = tk.Frame(self.root, bg=THEME.bg_dark)
        main_container.pack(fill='both', expand=True)
        
        # Navigation
        self.nav = NavigationBar(main_container, on_navigate=self.navigate)
        self.nav.pack(side='left', fill='y')
        
        # Content area
        content_area = tk.Frame(main_container, bg=THEME.bg_dark)
        content_area.pack(side='left', fill='both', expand=True)
        
        # Top bar
        self.top_bar = TopBar(content_area)
        self.top_bar.pack(fill='x')
        
        # Divider
        tk.Frame(content_area, bg=THEME.border, height=1).pack(fill='x')
        
        # Page container
        self.page_container = tk.Frame(content_area, bg=THEME.bg_dark)
        self.page_container.pack(fill='both', expand=True, padx=THEME.spacing_l,
                                pady=THEME.spacing_l)
        
        # Create pages
        self.pages = [
            DashboardPage(self.page_container, self),
            DigitalInputPage(self.page_container, self),
            DigitalOutputPage(self.page_container, self),
            SettingsPage(self.page_container, self),
        ]
        
        # Show dashboard
        self.current_page = 0
        self._show_page(0)
    
    def _show_page(self, index: int):
        """Show a page"""
        for page in self.pages:
            page.pack_forget()
        
        self.pages[index].pack(fill='both', expand=True)
        self.top_bar.set_title(self.page_titles[index])
    
    def navigate(self, index: int):
        """Navigate to a page"""
        self.current_page = index
        self.nav.set_active(index)
        self._show_page(index)
    
    def connect(self, port: str, baudrate: int) -> bool:
        """Connect to RS485 device"""
        try:
            config = RS485Config(device=port, baudrate=baudrate)
            self.rs485 = RS485(config)
            
            if not self.rs485.open():
                messagebox.showerror("Error", f"Failed to open {port}")
                return False
            
            self.protocol = EnersionProtocol(self.rs485)
            
            # Test connection
            if not self.protocol.ping(Address.CTRL_DIO) and not self.protocol.ping(Address.CTRL_OUT):
                messagebox.showwarning("Warning", "No controllers responded to ping")
            
            self.is_connected = True
            self.nav.set_connected(True, port)
            
            # Start polling
            self._start_polling()
            
            logger.info(f"Connected to {port} @ {baudrate}")
            return True
            
        except Exception as e:
            messagebox.showerror("Error", str(e))
            logger.error(f"Connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from RS485 device"""
        self._stop_polling()
        
        if self.rs485:
            self.rs485.close()
            self.rs485 = None
        
        self.protocol = None
        self.is_connected = False
        self.nav.set_connected(False)
        
        logger.info("Disconnected")
    
    def _start_polling(self):
        """Start polling thread"""
        self.polling = True
        self.poll_thread = threading.Thread(target=self._poll_loop, daemon=True)
        self.poll_thread.start()
    
    def _stop_polling(self):
        """Stop polling thread"""
        self.polling = False
        if self.poll_thread:
            self.poll_thread.join(timeout=1.0)
            self.poll_thread = None
    
    def _poll_loop(self):
        """Polling loop (runs in separate thread)"""
        while self.polling and self.is_connected:
            try:
                # Read DI
                di_state = self.protocol.read_digital_inputs()
                if di_state:
                    self.di_states = di_state.to_list()
                    self.root.after(0, self._update_di_display)
                
                # Read DO
                do_state = self.protocol.read_digital_outputs()
                if do_state:
                    self.do_states = do_state.to_list()
                    self.root.after(0, self._update_do_display)
                
                time.sleep(0.1)  # 100ms poll interval
                
            except Exception as e:
                logger.error(f"Poll error: {e}")
                time.sleep(1.0)
    
    def _update_di_display(self):
        """Update DI display (called from main thread)"""
        self.pages[1].update_states(self.di_states)
        di_count = sum(self.di_states)
        self.top_bar.set_di_count(di_count)
        self.pages[0].update_counts(di_count, sum(self.do_states))
    
    def _update_do_display(self):
        """Update DO display (called from main thread)"""
        self.pages[2].update_states(self.do_states)
        do_count = sum(self.do_states)
        self.top_bar.set_do_count(do_count)
        self.pages[0].update_counts(sum(self.di_states), do_count)
    
    def toggle_output(self, channel: int):
        """Toggle a single output"""
        if not self.is_connected or not self.protocol:
            return
        
        self.do_states[channel] = not self.do_states[channel]
        self._write_outputs()
    
    def all_outputs_on(self):
        """Turn all outputs ON"""
        if not self.is_connected or not self.protocol:
            return
        
        self.do_states = [True] * 64
        self._write_outputs()
    
    def all_outputs_off(self):
        """Turn all outputs OFF"""
        if not self.is_connected or not self.protocol:
            return
        
        self.do_states = [False] * 64
        self._write_outputs()
    
    def set_pattern(self, pattern: int):
        """Set alternating pattern"""
        if not self.is_connected or not self.protocol:
            return
        
        for i in range(64):
            byte_idx = i // 8
            bit_idx = i % 8
            self.do_states[i] = bool(pattern & (1 << bit_idx))
        
        self._write_outputs()
    
    def _write_outputs(self):
        """Write current DO states to hardware"""
        if not self.protocol:
            return
        
        state = DIOState()
        state.from_list(self.do_states)
        
        try:
            self.protocol.write_digital_outputs(state)
            self._update_do_display()
        except Exception as e:
            logger.error(f"Write outputs failed: {e}")
    
    def run(self):
        """Run the application"""
        self.root.mainloop()


def main():
    """Main entry point"""
    app = EnersionApp()
    app.run()


if __name__ == "__main__":
    main()

