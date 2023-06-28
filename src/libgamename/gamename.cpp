#include "gamename.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

std::map<std::string, std::string> parse_file_into_map(const std::string& filename) {
    std::map<std::string, std::string> data_map;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        iss >> key;
        std::getline(iss >> std::ws, value);
        value = value.substr(1, value.size() - 2);
        data_map[key] = value;
    }
    file.close();
    return data_map;
}

const char * GetGameName(const char * langname, const char * key)
{
    //initialized only the first time
    static std::map<std::string, std::string> static_roms_name_map = parse_file_into_map("/mnt/SDCARD/BIOS/arcade_lists/arcade-rom-names.txt");
    if (!key)
        return NULL;
    auto iter = static_roms_name_map.find(key);
    if (iter != static_roms_name_map.end()) {
        return const_cast<char*>(iter->second.c_str()); // return the value associated with the key
    } else {
        return NULL; // return a default value if the key is not found
    }

}

// not mangled function that can be called froma a C program
const char * GetGameNameForC(const char * langname, const char * key){
    return GetGameName(langname, key);
}