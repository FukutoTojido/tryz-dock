import Notifd from "gi://AstalNotifd";

const notifd = Notifd.get_default();
for (const notification of notifd.get_notifications()) {
	notification.dismiss();
}