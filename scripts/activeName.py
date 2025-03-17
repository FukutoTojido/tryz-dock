#!/usr/bin/python -u

import gi
import json
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck, Gdk, GLib

activeWin = None

def on_active_window_changed(screen, window):
    active_window = screen.get_active_window()
    if not active_window:
        return
    activeWin = active_window
    activeWin.connect("name-changed", on_name_change)
    
    print(json.dumps({
        "name": activeWin.get_name(),
        "application": activeWin.get_application().get_name(),
        "xid": active_window.get_xid(),
    }))

def on_name_change(window):
    # if not activeWin:
    #     return
    print(json.dumps({
        "name": window.get_name(),
        "application": window.get_application().get_name(),
        "xid": window.get_xid(),
    }))

def main():
    screen = Wnck.Screen.get_default()
    screen.connect("active-window-changed", on_active_window_changed)

    Gdk.init([])
    loop = GLib.MainLoop.new(None, False)

    loop.run()

    activeWin = screen.get_active_window()
    activeWin.connect("name-changed", on_name_change)

main()