#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "lib/fifo_map.hpp"
#include "rton-json.h"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const std::string ver = std::to_string(VERSION_MAJOR) + '.' + std::to_string(VERSION_MINOR) + '.' + std::to_string(VERSION_PATCH);

//import from json2rton.cpp
int rton_encode();

//import from rton2json.cpp
json json_decode();

std::ifstream input;
std::ofstream output, debug;
json debug_js, rton_list, json_list;

std::string to_hex_string(uint64_t q){
    std::stringstream ss;
    ss << "0x" << std::hex << q;
    return ss.str();
}

std::string to_hex_string(std::vector <uint8_t> a){
    std::stringstream ss;
    ss << "0x";
    for (uint8_t i : a){
        ss << std::setfill('0') << std::setw(2) << std::hex << (int) i;
    }
    return ss.str();
}

int help(const char *argv[]){
    std::cerr << "Usage:" << std::endl
              << '\t' << argv[0] << " <file-or-folder-path>" << std::endl
              << '\t' << argv[0] << " [options]" << " <file-or-folder-path>" << std::endl << std::endl
              << "Options:" << std::endl
              << "\t--help\t\tShow help (the thing you're looking at)" << std::endl
              << "\t--rton2json\tForce covert RTON to JSON" << std::endl
              << "\t--json2rton\tForce covert JSON to RTON" << std::endl;
    std::cin.get();
    return 1;
}

int process_file(std::filesystem::path file_name, const int argc, const char *argv[]){
    debug_js.clear();
    std::clog << "Processing file " << file_name;

    //info
    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile time"] = std::string(__DATE__) + ' ' + __TIME__;

    //detect rton or json
    int file_type = -1;//0 json      1 rton
    if (argc == 3){
        debug_js["Info"]["Mode"] = "Manual";
        if (strcmp(argv[1], "--rton2json") == 0) file_type = 1;
        else if (strcmp(argv[1], "--json2rton") == 0) file_type = 0;
        else return help(argv);
        debug_js["Info"]["Option"] = argv[1];
    }
    else{
        debug_js["Info"]["Mode"] = "Auto";

        input.open(file_name, std::ifstream::binary);

        char header[5];
        input.read(header, 4);
        header[4] = 0;

        if (strcmp(header, "RTON") == 0) file_type = 1;
        else{
            try{
                input.seekg(0);
                json js = json::parse(input);
                file_type = 0;
            }
            catch(json::exception &e){
            }
        }

        input.close();
    }
    debug_js["Info"]["File"] = file_name.string();

    //init RTON Stats
    debug_js["RTON stats"]["RTON version"] = 1; //not sure if it ever higher than 1
    debug_js["RTON stats"]["List of bytecodes"]["Offset"] = "Bytecode";
    debug_js["RTON stats"]["0x91 stack"]["Unsigned RTON number"] = "String";
    debug_js["RTON stats"]["0x93 stack"]["Unsigned RTON number"] = "UTF-8 string";

    try{
        switch(file_type){
        case 0:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::clog << " - JSON Detected" << std::endl;
            json_list["JSON files"].push_back(file_name.string());

            input.open(file_name);
            std::filesystem::create_directory(file_name.parent_path() / "json2rton");
            output.open((file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton", std::ofstream::binary);
            rton_encode(); //write directly to file

            input.close();
            output.close();

            //log at the end
            debug << std::setw(4) << debug_js;
            debug.close();
            std::clog << "Done" << std::endl << std::endl;

            break;
        }
        case 1:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::clog << " - RTON Detected" << std::endl;
            rton_list["RTON files"].push_back(file_name.string());

            input.open(file_name, std::ifstream::binary);
            std::filesystem::create_directory(file_name.parent_path() / "rton2json");
            output.open((file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json");
            output << std::setw(4) << json_decode();

            input.close();
            output.close();

            //log at the end
            debug << std::setw(4) << debug_js;
            debug.close();
            std::clog << "Done" << std::endl << std::endl;

            break;
        }
        default:{
            std::clog << " - Unknown" << std::endl
                      << "Skipped" << std::endl << std::endl;
        }
        }
    }
    catch (int e){
        //reset fstream state
        input.close();
        output.close();
        debug.close();

        //remove unfinished file
        std::filesystem::path out_file;
        if (file_type) out_file = (file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json";
        else out_file = (file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton";
        std::filesystem::remove(out_file);

        std::clog << "Error code: " << e << std::endl << std::endl;
        return e;
    }
    return 0;
}

int main(const int argc, const char *argv[]){
    std::clog << std::endl << "rton-json made by H3x4n1um" << std::endl
              << "Version: " << ver << std::endl
              << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl
              << "Credits: nlohmann for his awesome JSON parser and fifo_map" << std::endl << std::endl;

    std::filesystem::path path;
    switch (argc){
    case 1:{
        std::string temp;
        std::clog << "Enter file or folder path: ";
        getline(std::cin, temp);
        path = temp;
        std::clog << std::endl;
        break;
    }
    case 2:{
        if (strcmp(argv[1], "--help") == 0) return help(argv);
        path = argv[1];
        break;
    }
    case 3:{
        path = argv[2];
        break;
    }
    default:{
        return help(argv);
    }
    }

    if (!std::filesystem::is_regular_file(path) && !std::filesystem::is_directory(path)){
        std::cerr << "Error! Can't find file or folder " << path << "!!!" << std::endl;
        std::cin.get();
        return 1;
    }

    if (std::filesystem::is_regular_file(path)){
        process_file(path, argc, argv);
        path = path.parent_path();
    }
    else{
        for (auto &file : std::filesystem::directory_iterator(path)){
            if (std::filesystem::is_regular_file(file)){
                process_file(file.path(), argc, argv);
            }
        }
    }

    rton_list["Total files"] = rton_list["RTON files"].size();
    json_list["Total files"] = json_list["JSON files"].size();

    //log processed json & rton
    debug.open(path / "json2rton" / "json2rton.json");
    debug << std::setw(4) << json_list;
    debug.close();
    debug.open(path / "rton2json" / "rton2json.json");
    debug << std::setw(4) << rton_list;
    debug.close();

    std::clog << "Finished" << std::endl;
    std::cin.get();
    return 0;
}
