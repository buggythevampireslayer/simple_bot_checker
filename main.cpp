#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <string.h>

using std::string, std::vector;

struct cheater { string id3; char tag; };
struct player { int id; string ign; string id3; };
HWND h_pWindow = nullptr;

HWND get_windowhandle()
{
    HWND h_pWindow = nullptr;
    std::cout << "Waiting for TF2 process..." << std::endl;
    while (h_pWindow == nullptr)
    {
        h_pWindow = FindWindowA("Valve001", 0);
        if (h_pWindow == nullptr)
        {
            Sleep(200); // prevent throttling
        }
    }
    std::cout << "Found TF2 process." << std::endl;
    return h_pWindow;
}

void set_color(int option){
    HANDLE color;
    color = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(color, option);
}

int get_col_id(string option)
{
    if (option == "black")
        return 0;
    else if (option == "blue")
        return 1;
    else if (option == "green") 
        return 2;
    else if (option == "teal")
        return 3;
    else if (option == "darkred")
        return 4;
    else if (option == "purple")
        return 5;
    else if (option == "olive")
        return 6;
    else if (option == "lightgray")
        return 7;
    else if (option == "darkgray")
        return 8;
    else if (option == "blue")
        return 9;
    else if (option == "lime")
        return 10;
    else if (option == "cyan")
        return 11;
    else if (option == "red")
        return 12;
    else if (option == "pink")
        return 13;
    else if (option == "yellow")
        return 14;
    else if (option == "white")
        return 15;
    return 0;
}

vector<cheater> get_cheater_list(string path)
{
    std::ifstream readfile(path);

    if (!readfile.is_open())
        readfile.open(path);

    vector<cheater> cl;

    string line;
    while (getline(readfile, line))
    {
        cheater c;
        c.tag = line[0];
        c.id3 = line.substr(2, line.length() - 2);
        cl.push_back(c);
    }
    readfile.close();
    return cl;
}

player handle_line(string line)
{
    int id3_s = 0, id3_e = 0, ign_s = 10, ign_e = 0, id_s = 0, id_e = 0;

    for (int i = 7; i < 45; i++){
        if (line[i] == '"')
            ign_e = i;
    }

    for (int i = 25; i < 65; i++){
        if (line[i] == '[')
            id3_s = i;
        else if (line[i] == ']')
            id3_e = i + 1;
    }

    for (int i = 1; i < 10; i++){
        if (line[i] != ' ' && id_s == 0)
            id_s = i;
        
        else if (line[i] == ' ' && id_s != 0)
            id_e = i;
    }
    // get id3 and ign
    player p;
    p.id3 = line.substr(id3_s, id3_e - id3_s);
    p.ign = line.substr(ign_s, ign_e - ign_s);
    int x;
    sscanf(line.substr(id_s, id_e - id_s).c_str(), "%d", &x);
    p.id = x;
    return p;
}

vector<player> get_ingame_playerlist(string path)
{
    std::ifstream readlogfile(path);

    if (!readlogfile.is_open())
        readlogfile.open(path);
    

    vector<player> pv;
    bool hitfirsthash = false;
    string line;

    while (getline(readlogfile, line))
    {
        if (line[0] != '#' || line[2] == 'u')
            continue;
        
        else {
            pv.push_back(handle_line(line));
            if (!hitfirsthash)
                hitfirsthash = true;
        }
    }
    readlogfile.close();
    return pv;
}

vector<int> kick_list;

void kick_loop(){
    if (kick_list.size() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    else
    {
        const char* kickcmd = ("callvote kick " + std::to_string(kick_list.at(0))).c_str();
        COPYDATASTRUCT data;
        data.cbData = strlen(kickcmd) + 1;
        data.dwData = 0;
        data.lpData = (void *)kickcmd;
        SendMessageA(h_pWindow, WM_COPYDATA, 0, (LPARAM)&data);
        std::this_thread::sleep_for(std::chrono::milliseconds(25000));
    }
}

int main()
{
    // read config file to get TF2 install path
    std::cout << "Reading config..." << std::endl;
    std::ifstream readconfig("config.cfg");
    string path, bgoption, txtoption, autokick;
    getline(readconfig, path);
    path += "\\tf\\console.log";

    getline(readconfig, bgoption);
    getline(readconfig, txtoption);
    getline(readconfig, autokick);
    readconfig.close();
    std::cout << "Done." << std::endl << std::endl;


    // set console color here
    int color_option; // bruh
    try {
        color_option = get_col_id(bgoption.substr(3, bgoption.length() - 3)) * 16 + get_col_id(txtoption.substr(5, txtoption.length() - 5));
    } catch (const std::exception &err) {
        color_option = 15;
    }
    if (color_option == 0) color_option = 15;
    set_color(color_option);

    // do autokick function
    bool b_autokick = (autokick.substr(9, autokick.length() - 9) == "true") ? true : false;

    // read cheater_list.txt to get current cheaters
    std::cout << "Parsing cheater list..." << std::endl;
    vector<cheater> cheater_list = get_cheater_list("cheater_list.txt");
    std::cout << "Done." << std::endl << std::endl;

    // vector variables
    vector<player> player_list;
    vector<player> temp_list;
    
    // Loop until tf2 process is found
    // credit to https://github.com/extremeblackliu who i got this idea off
    HWND h_pWindow = get_windowhandle();

    // define message data
    const char *statuscmd = "status";
    COPYDATASTRUCT data;
    data.cbData = strlen(statuscmd) + 1;
    data.dwData = 0;
    data.lpData = (void *)statuscmd;

    // truncate file before opening so it doesn't drop a whole bunch of players on startup
    std::ofstream clr_file;
    clr_file.open(path, std::ofstream::out | std::ofstream::trunc);
    clr_file.close();

    // wait for player input to start
    std::cout << std::endl;
    std::cout << "NOTE: Wait for home page to fully load before continuing" << std::endl;
    system("pause");

    while (true)
    {
        if (!IsWindow(h_pWindow)){
            std::cout << "Lost TF2 window, attempting to locate it." << std::endl;
            h_pWindow = get_windowhandle();
        }
        SendMessageA(h_pWindow, WM_COPYDATA, 0, (LPARAM)&data);
        Sleep(200);
        
        temp_list = get_ingame_playerlist(path);
        if (temp_list.size() > 1){
            player_list = temp_list;
            sort(player_list.begin(), player_list.end(), [](const player &lv, const player &rv)
                 { return lv.id < rv.id; });
        }

        system("cls");
        std::cout << "Current list of players in game:" << std::endl << std::endl;

        for (auto p = player_list.begin(); p != player_list.end(); p++)
        {
            string t;
            cheater match;
            for (auto c = cheater_list.begin(); c != cheater_list.end(); c++){
                if (c->id3 == p->id3){
                    match = *c;
                    break;
                }
            }
            if (!match.id3.empty()){
                t = (match.tag == 'C') ? "Cheater   " : 
                    (match.tag == 'S') ? "Suspicious" : 
                    (match.tag == 'W') ? "Watched   " : 
                    "Innocent  ";
            }
            else {
                t = "          ";
            }
            if (b_autokick && match.tag == 'C'){
                // autokick script
                kick_list.push_back(p->id);
            }
            string ign_padding = "";
            string id3_padding = "";
            for (int i = 0; i < (18 - p->id3.length()); i++){
                id3_padding += " ";
            }
            for (int i = 0; i < (34 - p->ign.length()); i++){
                ign_padding += " ";
            }
            // print player info to console
            std::cout << t << " :   " << p->ign << ign_padding << " - " << p->id3 << id3_padding << " - " << p->id << std::endl;
        }

        // clear console log file
        temp_list.clear();
        std::ofstream clr_file;
        clr_file.open(path, std::ofstream::out | std::ofstream::trunc);
        clr_file.close();

        // try to kick a player on another thread
        std::thread t(kick_loop);
        t.join();

        // Changes this from 1000 to 4800 since it will be running status on its own now and it's unnecessary cpu usage
        Sleep(4800);
    }
}