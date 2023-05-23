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

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#define NIL (0)

string exec(const char *cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

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

int pause(int milisec)
{
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = milisec * 1000000L;
    return nanosleep(&req, (struct timespec *)NULL);
}

int main(int argc, char *argv[])
{
    // Open connection to X server
    Display *dsp = XOpenDisplay(NIL);
    assert(dsp);

    // Start listening to root window, we handle potential errors
    // with custom error handler.
    XSelectInput(dsp, DefaultRootWindow(dsp), SubstructureNotifyMask);
    XSetErrorHandler(bad_window_handler);

    Window activeWindow;
    int revertTo;

    for (;;)
    {
        XEvent e;
        XNextEvent(dsp, &e);

        if (e.type == ConfigureNotify)
        {
            pause(100);
            Window temp = activeWindow;

            activeWindow = *(Window *)getProp(dsp, DefaultRootWindow(dsp), XA_WINDOW, (char *)"_NET_ACTIVE_WINDOW", NULL);
            // XGetInputFocus(dsp, &activeWindow, &revertTo);

            if (temp != activeWindow)
            {
                unsigned long *pid = NULL;
                pid = (unsigned long *)getProp(dsp, activeWindow, XA_CARDINAL, (char *)"_NET_WM_PID", NULL);
                if (pid != NULL)
                    cout << *pid << endl;
                g_free(pid);
            }
            // cout << "0x" << hex << activeWindow << endl;
        }
    }
}