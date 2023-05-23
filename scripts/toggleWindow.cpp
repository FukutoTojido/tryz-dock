#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <glib.h>
using namespace std;

#define NIL (0)

/**
 * Warn that the window does not exist but allow for continued execution. We
 * pretend that we know what we are doing.
 */
int bad_window_handler(Display *disp, XErrorEvent *err)
{
    char bad_id[20];
    char const *const window_id_format = "0x%lx";
    snprintf(bad_id, sizeof(bad_id), window_id_format, err->resourceid);
    cout << "WARNING: There is no window with ID(" << bad_id << ") any more." << endl;
    return 0;
}

gchar *getProp(Display *dsp, Window w, Atom xa_prop_type, char *prop_name, unsigned long *size)
{
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    gchar *ret;

    xa_prop_name = XInternAtom(dsp, prop_name, False);

    if (XGetWindowProperty(dsp, w, xa_prop_name, 0, 4096 / 4, False,
                           xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success)
    {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type)
    {
        XFree(ret_prop);
        return NULL;
    }

    tmp_size = (ret_format / 8) * ret_nitems;
    if (ret_format == 32)
        tmp_size *= sizeof(long) / 4;
    ret = (gchar *)g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size)
    {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

void minimizeWindow(Display *dsp, Window w) {
    XIconifyWindow(dsp, w, DefaultScreen(dsp));
}

void maximizeWindow(Display *dsp, Window w)
{
    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = w;
    ev.message_type = XInternAtom(dsp, "_NET_ACTIVE_WINDOW", True);
    ev.format = 32;
    ev.data.l[0] = 1;
    ev.data.l[1] = CurrentTime;
    ev.data.l[2] = ev.data.l[3] = ev.data.l[4] = 0;
    XSendEvent(dsp, DefaultRootWindow(dsp), False, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent *)&ev);
    XFlush(dsp);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        return 0;

    // Open connection to X server
    Display *dsp = XOpenDisplay(NIL);
    assert(dsp);

    // Start listening to root window, we handle potential errors
    // with custom error handler.
    XSelectInput(dsp, DefaultRootWindow(dsp), SubstructureNotifyMask);
    XSetErrorHandler(bad_window_handler);

    Window selectedWindow = (Window)stol(string(argv[1]), nullptr, 0);
    Window activeWindow = (Window)stol(string(argv[2]), nullptr, 0);

    unsigned long size;
    Atom *stateList = (Atom *)getProp(dsp, selectedWindow, XA_ATOM, (char *)"_NET_WM_STATE", &size);

    bool isHidden = false;
    for (size_t i = 0; i < size; i++)
    {
        if (stateList[i] == 375)
        {
            isHidden = true;
            break;
        }
    }

    if (isHidden)
    {
        maximizeWindow(dsp, selectedWindow);
    }
    else
    {
        minimizeWindow(dsp, selectedWindow);
    }

    // if (selectedWindow == activeWindow) {

    // }

    // cout << hex << selectedWindow << '\t' << hex << activeWindow << endl;
}