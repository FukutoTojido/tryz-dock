#!/usr/bin/python -u

import json
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck, Gdk, GLib
from gi.repository import Gtk

excludedApps = ['polydock', 'polybar', 'spacefm', 'panel', 'eww-bar', 'eww-user', 'eww-timeWidget', 'eww']

def on_window_destroy(screen, window):
    get_window_list(screen)

def on_window_opened(screen, window):
    get_window_list(screen)

def get_window_list(screen):
    window_list = screen.get_windows()
    theme = Gtk.IconTheme.get_default()

    w = []

    for x in window_list:
        if not x.get_class_instance_name() or x.get_class_instance_name() in excludedApps:
            continue

        # icon = "/home/try-z/.icons/placeholder.svg"
        # if theme.lookup_icon(x.get_class_instance_name(), 48, 0):
        #     icon = theme.lookup_icon(x.get_class_instance_name(), 48, 0).get_filename()

        pid = x.get_xid()
        icon_path = f"""/tmp/{pid}.png"""
        x.get_mini_icon().savev(icon_path, "png")

        print(f"""{icon_path}""")

        # w.append(json.dumps({
        #     "icon": icon,
        #     "pid": x.get_xid(),
        #     "name": x.get_name(),
        #     "class": x.get_class_instance_name()
        # }))

    # print(f"""[{",".join(w)}]""")

def main():
    while Gtk.events_pending():
        Gtk.main_iteration()

    screen = Wnck.Screen.get_default()
    screen.connect("window-opened", on_window_opened)
    screen.connect("window-closed", on_window_destroy)

    get_window_list(screen)

    Gdk.init([])
    loop = GLib.MainLoop.new(None, False)

    loop.run()

main()