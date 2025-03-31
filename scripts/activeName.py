#!/usr/bin/python -u

import gi
import json
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck, Gdk, GLib

activeWin = None

def on_active_window_changed(screen, window):
    global activeWin
    active_window = screen.get_active_window()
    if not active_window:
        print(json.dumps({
            "name": "welcome home",
            "application": "desktop",
            "xid": -1
        }))
        return
    activeWin = active_window
    activeWin.connect("name-changed", on_name_change)
    
    print(json.dumps({
        "name": activeWin.get_name(),
        "application": activeWin.get_application().get_name(),
        "xid": active_window.get_xid(),
    }))

def on_name_change(window):
    global activeWin
    if not activeWin:
        return
    
    if activeWin.get_xid() != window.get_xid():
        return

    print(json.dumps({
        "name": window.get_name(),
        "application": window.get_application().get_name(),
        "xid": window.get_xid(),
    }))

def main():
    screen = Wnck.Screen.get_default()
    screen.connect("active-window-changed", on_active_window_changed)

    Gdk.init([])
    loop = GLib.MainLoop.new(None, False)\

    loop.run()

    global activeWin
    activeWin = screen.get_active_window()
    activeWin.connect("name-changed", on_name_change)

main()