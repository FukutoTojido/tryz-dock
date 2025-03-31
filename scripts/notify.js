import Notifd from "gi://AstalNotifd";

const GLib = imports.gi.GLib;
const notifd = Notifd.get_default();

notifd.connect("notified", (_, id) => {
	const notification = notifd.get_notification(id);
	const { time, app_name, app_icon, summary, body } = notification;
	const notif = {
		id,
		time,
		app_name,
		app_icon: app_icon.replace("file://", ""),
		summary,
		body,
		actions: notification.get_actions().map(({ id, label }) => {
			return {
				id,
				label,
			};
		}),
	};

	GLib.spawn_command_line_async(`eww open flash_notif --arg notif=\'${JSON.stringify(notif)}\' --id notif-${notif.id}`)
	GLib.timeout_add(GLib.PRIORITY_DEFAULT, 10000, () => {
		GLib.spawn_command_line_async(`eww close notif-${notif.id}`)
		return GLib.SOURCE_REMOVE;
	});
});

const loop = GLib.MainLoop.new(null, false);
loop.run();
