#!/usr/bin/python -u

import json
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Wnck', '3.0')

from gi.repository import Wnck
from gi.repository import Gtk

excludedApps = ['polydock', 'polybar', 'spacefm']

screen = Wnck.Screen.get_default()
theme = Gtk.IconTheme.get_default()

while Gtk.events_pending():
    Gtk.main_iteration()

# print("a")

# Initial Run
currentPID = screen.get_active_window().get_xid()

print(f"""[{",".join([
    json.dumps({
        "icon": theme.lookup_icon(x.get_class_instance_name(), 48, 0).get_filename(),
        "pid": x.get_xid(),
        "name": x.get_name(),
        "isActive": currentPID == x.get_xid()
    }) for x in screen.get_windows() 
        if theme.lookup_icon(x.get_class_instance_name(), 48, 0) and x.get_class_instance_name() not in excludedApps])}]""")
