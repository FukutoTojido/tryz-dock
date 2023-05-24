#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <assert.h>
#include <string>
#include <iostream>
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
    if (argc < 2)
        return 0;

    // Open connection to X server
    Display *dsp = XOpenDisplay(NIL);
    assert(dsp);

    // Start listening to root window, we handle potential errors
    // with custom error handler.
    XSelectInput(dsp, DefaultRootWindow(dsp), SubstructureNotifyMask);
    XSetErrorHandler(bad_window_handler);

    string::size_type sz;
    Window selectedWindow = (Window)stol(string(argv[1]), &sz);

    maximizeWindow(dsp, selectedWindow);
}