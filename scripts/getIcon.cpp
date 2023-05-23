#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <regex>
#include <vector>

#include <gtkmm-3.0/gtkmm.h>
auto app = Gtk::Application::create();
auto icon = Gtk::IconTheme::create();
// auto theme = icon->get_default();

#include "nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;

json apps = R"([])"_json;

vector<string> clientList;
string activeClient;

size_t length = 0;

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

void getInfo()
{
    for (string pid : clientList)
    {
        string homeDir = exec("echo $HOME");
        homeDir.pop_back();

        json app =
            R"(
            {
                "icon": "",
                "pid": "", 
                "class": "", 
                "name": "",
                "isActive": false
            })"_json;
        app["pid"] = pid;
        app["isActive"] = pid == activeClient;

        string WM_CLASS = " WM_CLASS";
        string WM_NAME = " WM_NAME";
        string windowClass = exec(((string) "xprop -id " + pid + WM_CLASS).c_str());
        string windowName = exec(((string) "xprop -id " + pid + WM_NAME).c_str());

        regex pttrn("\".*?\"");
        smatch match;

        if (regex_search(windowClass, match, pttrn))
        {
            windowClass = match[0];
            app["class"] = windowClass.substr(1, windowClass.length() - 2);

            if (find(excluded.begin(), excluded.end(), windowClass.substr(1, windowClass.length() - 2)) != excluded.end())
                continue;

            // auto iconFile = theme->lookup_icon(windowClass.substr(1, windowClass.length() - 2), 48, 1);
            // app["icon"] = iconFile.get_filename();
        }

        if (regex_search(windowName, match, pttrn))
        {
            windowName = match[0];
            app["name"] = windowName.substr(1, windowName.length() - 2);
        }

        apps.push_back(app);
    }
}

void getAppNameAndCount()
{
    // while (true)
    // {
    //     apps = R"([])"_json;
    //     string clientsRaw = exec("xprop -root _NET_CLIENT_LIST");
    //     string activeRaw = exec("xprop -root _NET_ACTIVE_WINDOW");

    //     regex pttrn("0x[0-9A-Fa-f]+");
    //     smatch match;

    //     clientsRaw.pop_back();
    //     activeRaw.pop_back();

    //     if (regex_search(activeRaw, match, pttrn))
    //         activeClient = match[0];

    //     while (regex_search(clientsRaw, match, pttrn))
    //     {
    //         clientList.push_back(match[0]);
    //         clientsRaw = match.suffix();
    //     }

    //     cout << length << ' ' << clientList.size();

    //     if (length != clientList.size())
    //     {
    //         cout << length << ' ' << clientList.size();
    //         length = clientList.size();
    //         getInfo();
    //         // cout << apps << '\n';
    //     }
    // }
}

void getAppIcon() {}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    getAppNameAndCount();
}