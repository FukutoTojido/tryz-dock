#include <gtkmm-3.0/gtkmm.h>
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

auto app = Gtk::Application::create();
auto icon = Gtk::IconTheme::create();
auto theme = icon->get_default();

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#define NIL (0)

vector<string> excluded = {"polydock", "polybar", "rofi", "spacefm", "eww-bar"};

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

Window getActiveWindow(Display *dsp)
{
    Window *activeWin = NULL;
    unsigned long size;

    activeWin = (Window *)getProp(dsp, DefaultRootWindow(dsp), XA_WINDOW, (char *)"_NET_ACTIVE_WINDOW", &size);
    return activeWin[0];
}

gchar *getClass(Display *disp, Window win)
{
    gchar *class_utf8;
    gchar *wm_class;
    unsigned long size;

    wm_class = getProp(disp, win, XA_STRING, (char *)"WM_CLASS", &size);
    if (wm_class)
    {
        gchar *p_0 = strchr(wm_class, '\0');
        if (wm_class + size - 1 > p_0)
        {
            *(p_0) = '.';
        }
        class_utf8 = g_locale_to_utf8(wm_class, -1, NULL, NULL, NULL);
    }
    else
    {
        class_utf8 = NULL;
    }

    g_free(wm_class);

    return class_utf8;
}

gchar *getOutputStr(gchar *str, gboolean is_utf8)
{
    gchar *out;

    if (str == NULL)
    {
        return NULL;
    }

    out = g_strdup(str);
    return out;
}

gchar *getTitle(Display *dsp, Window w)
{
    gchar *title_utf8;
    gchar *wm_name;
    gchar *net_wm_name;

    wm_name = getProp(dsp, w, XA_STRING, (char *)"WM_NAME", NULL);
    net_wm_name = getProp(dsp, w, XInternAtom(dsp, "UTF8_STRING", False), (char *)"_NET_WM_NAME", NULL);

    if (net_wm_name)
    {
        title_utf8 = g_strdup(net_wm_name);
    }
    else
    {
        if (wm_name)
        {
            title_utf8 = g_locale_to_utf8(wm_name, -1, NULL, NULL, NULL);
        }
        else
        {
            title_utf8 = NULL;
        }
    }

    g_free(wm_name);
    g_free(net_wm_name);

    return title_utf8;
}

Window *getClientList(Display *dsp, unsigned long *size)
{
    Window *clientList = NULL;

    clientList = (Window *)getProp(dsp, DefaultRootWindow(dsp), XA_WINDOW, (char *)"_NET_CLIENT_LIST", size);
    return clientList;
}

void listWindows(Display *dsp)
{
    json apps = R"([])"_json;
    Window *clientList;
    unsigned long clientListSize;
    int i = 0;
    int maxClientLen = 0;

    clientList = getClientList(dsp, &clientListSize);

    // cout << hex << activeWindow << endl;
    // cout << getOutputStr(getTitle(dsp, activeWindow), TRUE) << endl;

    for (i = 0; i < clientListSize / sizeof(Window); i++)
    {
        gchar *clientMachine;
        if ((clientMachine = getProp(dsp, clientList[i], XA_STRING, (char *)"WM_CLIENT_MACHINE", NULL)))
        {
            maxClientLen = strlen(clientMachine);
        }
        g_free(clientMachine);
    }

    for (i = 0; i < clientListSize / sizeof(Window); i++)
    {
        json app = R"({
            "class": "",
            "pid": "",
            "icon": "",
            "name": "",
            "npid" : "",
            "window": ""
        })"_json;

        gchar *title_utf8 = getTitle(dsp, clientList[i]); /* UTF8 */
        gchar *title_out = getOutputStr(title_utf8, TRUE);
        gchar *class_out = getClass(dsp, clientList[i]); /* UTF8 */
        unsigned long *pid;

        stringstream pidStream;
        pidStream << "0x" << hex << clientList[i];
        app["window"] = pidStream.str();
        app["npid"] = clientList[i];

        string className = string(class_out);
        app["class"] = className.substr(0, className.find('.'));

        app["name"] = string(title_out);

        if (find(excluded.begin(), excluded.end(), className.substr(0, className.find('.'))) != excluded.end())
            continue;

        auto iconFile = theme->lookup_icon(className.substr(0, className.find('.')), 48, 1);
        if (iconFile.get_filename() != "")
            app["icon"] = iconFile.get_filename();
        else
        {
            app["icon"] = "/home/try-z/.icons/placeholder.svg";
        }

        pid = (unsigned long *)getProp(dsp, clientList[i], XA_CARDINAL, (char *)"_NET_WM_PID", NULL);
        app["pid"] = *pid;

        // cout << hex << clientList[i] << '\t' << class_out << '\t' << title_out << '\n';
        // cout << app << endl;
        apps.push_back(app);

        g_free(title_utf8);
        g_free(title_out);
        g_free(class_out);
        g_free(pid);
    }
    g_free(clientList);

    cout << apps << endl;
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

    // pause(1000);
    listWindows(dsp);

    for (;;)
    {
        XEvent e;
        XNextEvent(dsp, &e);
        // cout << e.type <<  endl;

        if (e.type == ReparentNotify)
        {
            // cout << "======================" << "Reparent Notify" << "======================\n";
            pause(100);
            listWindows(dsp);
            // cout << "======================" << "=============" << "======================\n";
        }
        else if (e.type == DestroyNotify)
        {
            // cout << "======================" << "Destroy Notify" << "=====================\n";
            pause(100);
            listWindows(dsp);
            // cout << "======================" << "==============" << "=====================\n";
        }
    }
}