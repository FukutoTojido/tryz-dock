#!/usr/bin/python -u

import json
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck, Gdk, GLib
from gi.repository import Gtk

root_win = None
excludedApps = ['polydock', 'polybar', 'spacefm', 'panel', 'eww-bar', 'eww-user', 'eww-timeWidget', 'eww']

class Node:
    def __init__(self, data):
        self.data = data
        self.next = None
        self.prev = None

def print_res():
    global root_win
    print(json.dumps(list(map(lambda x: x[1].data | { "idx": x[0] }, enumerate(serialize_linked_list(root_win))))))

def on_active_window_changed(screen, _):
    window = screen.get_active_window()
    if window is None:
        return
    
    global root_win
    if window.get_xid() == root_win.data["pid"]:
        return
    
    current_win = root_win

    while current_win is not None and current_win.data["pid"] != window.get_xid():
        current_win = current_win.next

    if current_win is None:
        return
    
    current_win.prev.next = current_win.next
    if current_win.next is not None:
        current_win.next.prev = current_win.prev

    root_win.prev = current_win
    current_win.next = root_win

    root_win = current_win

    # print(list(map(lambda x: x.data["class"], serialize_linked_list(root_win))))
    print_res()

def on_window_create(screen, window):
    global root_win
    theme = Gtk.IconTheme.get_default()

    icon = "/home/try-z/.icons/placeholder.svg"
    if theme.lookup_icon(window.get_class_instance_name(), 48, 0):
        icon = theme.lookup_icon(window.get_class_instance_name(), 48, 0).get_filename()

    current_win = Node({
        "icon": icon,
        "pid": window.get_xid(),
        "name": window.get_name(),
        "class": window.get_class_instance_name()
    })

    current_win.next = root_win

    if root_win is not None:
        root_win.prev = current_win
    root_win = current_win

    # print(list(map(lambda x: x.data["class"], serialize_linked_list(root_win))))
    print_res()

def on_window_destroy(screen, window):
    global root_win
    current_win = root_win

    if window.get_xid() == root_win.data["pid"]:
        root_win = current_win.next
        print_res()
        return
    
    while current_win is not None and current_win.data["pid"] != window.get_xid():
        current_win = current_win.next

    if current_win is None:
        return

    current_win.prev.next = current_win.next
    current_win.next.prev = current_win.prev   

    # print(list(map(lambda x: x.data["class"], serialize_linked_list(root_win))))
    print_res()

def get_windows_list(screen):
    window_list = screen.get_windows()
    theme = Gtk.IconTheme.get_default()

    last_win = None
    for x in window_list:
        icon = "/home/try-z/.icons/placeholder.svg"
        if theme.lookup_icon(x.get_class_instance_name(), 48, 0):
            icon = theme.lookup_icon(x.get_class_instance_name(), 48, 0).get_filename()

        current_win = Node({
            "icon": icon,
            "pid": x.get_xid(),
            "name": x.get_name(),
            "class": x.get_class_instance_name()
        })

        if root_win is None:
            root_win = current_win

        if last_win is not None:
            last_win.next = current_win
            current_win.prev = last_win

        last_win = current_win

def serialize_linked_list(root):
    windows = []
    current_win = root

    while current_win is not None:
        windows.append(current_win)
        current_win = current_win.next

    return windows

def main():
    while Gtk.events_pending():
        Gtk.main_iteration()

    screen = Wnck.Screen.get_default()
    screen.connect("window-opened", on_window_create)
    screen.connect("window-closed", on_window_destroy)
    screen.connect("active-window-changed", on_active_window_changed)
    get_windows_list(screen)

    Gdk.init([])
    loop = GLib.MainLoop.new(None, False)

    loop.run()

    # print(list(map(lambda x: x.data["class"], serialize_linked_list(root_win))))
    print_res()

main()

