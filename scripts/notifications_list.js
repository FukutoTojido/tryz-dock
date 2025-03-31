import Notifd from "gi://AstalNotifd";

const GLib = imports.gi.GLib;
const notifd = Notifd.get_default();

print(
	JSON.stringify(
		notifd.get_notifications().map((notification) => {
			const { id, time, app_name, app_icon, summary, body } = notification;
			return {
				id,
				time,
				app_name,
				app_icon,
				summary,
				body,
				actions: notification.get_actions().map(({ id, label }) => {
					return {
						id,
						label,
					};
				}),
			};
		}),
	),
);

const handleNotificationsChange = () => {
	const notifications = notifd.get_notifications();
	print(
		JSON.stringify(
			notifications.map((notification) => {
				const { id, time, app_name, app_icon, summary, body } = notification;
				return {
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
			}),
		),
	);
};

notifd.connect("notified", handleNotificationsChange);
notifd.connect("resolved", handleNotificationsChange);

const loop = GLib.MainLoop.new(null, false);
loop.run();
