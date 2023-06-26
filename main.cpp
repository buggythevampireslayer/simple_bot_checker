#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <Windows.h>

using std::string, std::vector;

struct cheater { string id3; char tag; };
struct player { string ign; string id3; char tag; };


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
    int id3_s = 0, id3_e = 0, ign_s = 10, ign_e = 0;

    for (int i = 7; i < 45; i++){
        if (line[i] == '"')
            ign_e = i;
    }

    for (int i = 25; i < 65; i++){
        if (line[i] == '[')
            id3_s = i;
        else if (line[i] == ']')
            id3_e = i;
    }
    // get id3 and ign
    player p;
    p.id3 = line.substr(id3_s, id3_e - id3_s + 1);
    p.ign = line.substr(ign_s, ign_e - ign_s);
    return p;
}

// read log file
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

int main()
{
    // read config file to get TF2 install path
    std::cout << "Reading config" << std::endl;
    std::ifstream readconfig("config.cfg");
    string path, bgoption, txtoption;
    getline(readconfig, path);
    path += "\\tf\\console.log";

    getline(readconfig, bgoption);
    getline(readconfig, txtoption);
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

    // read cheater_list.txt to get current cheaters
    std::cout << "Parsing cheater list" << std::endl;
    vector<cheater> cheater_list = get_cheater_list("cheater_list.txt");
    std::cout << "Done." << std::endl << std::endl;

    // Start loop 
    std::cout << "Press enter to begin." << std::endl;
    system("pause");
    vector<player> player_list;
    vector<player> temp_list;
    std::cout << "Starting loop." << std::endl;
    system("cls");
    while (true)
    {
        system("cls");
        temp_list = get_ingame_playerlist(path);
        if (temp_list.size() > 1)
            player_list = temp_list;
            
        std::cout << "Current list of cheaters in game:" << std::endl << std::endl;
        for (player p : player_list)
        {
            for (cheater c : cheater_list){
                if (c.id3 == p.id3){
                    string tag_name = (c.tag == 'C') ? "Cheater   " : ((c.tag == 'S') ? "Suspicious" : ((c.tag == 'W') ? "Watched   " : "Innocent  "));
                    std::cout << tag_name << ": " << p.ign << " " << c.tag << std::endl;
                }
            }
        }

        // clear console log file, sleep
        temp_list.clear();
        std::ofstream clr_file;
        clr_file.open(path, std::ofstream::out | std::ofstream::trunc);
        clr_file.close();
        
        Sleep(1000); // Optional to make this shorter or longer, at 1000ms it likely wont ever use more than 1% CPU
    }
}