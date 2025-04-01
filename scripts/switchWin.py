#!/usr/bin/python -u

import json
import gi
import os
gi.require_version('Gtk', '3.0')
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck, Gdk, GLib
from gi.repository import Gtk

root_win = None
excludedApps = ['spacefm', 'panel', 'eww-bar', 'eww-user', 'eww-timeWidget', 'eww']
windows = {}

class Node:
    def __init__(self, data):
        self.data = data
        self.next = None
        self.prev = None

def print_res():
    global root_win
    print(json.dumps(list(map(lambda x: x[1].data | { "idx": x[0] }, enumerate(serialize_linked_list(root_win))))))

def on_workspace_change(window):
    xid = window.get_xid()
    workspace = window.get_workspace().get_number()

    global windows
    windows[xid].data["workspace"] = workspace

    print_res() 

def on_name_change(window):
    xid = window.get_xid()
    name = window.get_name()

    global windows
    windows[xid].data["name"] = name

    print_res() 

def on_icon_change(window):
    xid = window.get_xid()
    icon_path = f"""/tmp/{xid}.png"""
    window.get_icon().savev(icon_path, "png")
        
    global windows
    windows[xid].data["icon"] = icon_path

    print_res() 

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

    pid = window.get_xid()
    icon_path = f"""/tmp/{pid}.png"""
    window.get_icon().savev(icon_path, "png")

    current_win = Node({
        "icon": icon_path,
        "pid": window.get_xid(),
        "name": window.get_name(),
        "class": window.get_class_instance_name(),
        "workspace": window.get_workspace().get_number()
    })

    current_win.next = root_win

    if root_win is not None:
        root_win.prev = current_win
    root_win = current_win
    
    global windows
    windows[current_win.data["pid"]] = current_win
    window.connect("name-changed", on_name_change)
    window.connect("icon-changed", on_icon_change)
    window.connect("workspace-changed", on_workspace_change)

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

    global windows
    windows.pop(current_win.data["pid"], None)

    pid = window.get_xid()
    icon_path = f"""/tmp/{pid}.png"""
    os.remove(icon_path)

    # print(list(map(lambda x: x.data["class"], serialize_linked_list(root_win))))
    print_res()

def get_windows_list(screen):
    window_list = screen.get_windows()

    last_win = None
    for x in window_list:
        pid = x.get_xid()
        icon_path = f"""/tmp/{pid}.png"""
        x.get_icon().savev(icon_path, "png")

        current_win = Node({
            "icon": icon_path,
            "pid": x.get_xid(),
            "name": x.get_name(),
            "class": x.get_class_instance_name(),
            "workspace": x.get_workspace().get_number()
        })

        if root_win is None:
            root_win = current_win

        if last_win is not None:
            last_win.next = current_win
            current_win.prev = last_win

        last_win = current_win
        
        global windows
        windows[current_win.data["pid"]] = current_win
        x.connect("name-changed", on_name_change)
        x.connect("icon-changed", on_icon_change)
        x.connect("workspace-changed", on_workspace_change)

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

