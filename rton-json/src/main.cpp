#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "rton-json.h"
#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const std::string ver = std::to_string(VERSION_MAJOR) + '.' + std::to_string(VERSION_MINOR) + '.' + std::to_string(VERSION_PATCH);

json json_decode();
int rton_encode();

std::ifstream input;
std::ofstream output, debug;
json debug_js;

std::string to_hex_string(uint64_t q){
    if (q == 0) return "0x0";
    std::string s;
    std::stringstream ss;
    ss << std::hex << std::showbase << q;
    ss >> s;
    return s;
}

std::string to_hex_string(std::vector <uint8_t> a){
    std::string s;
    std::stringstream ss;
    ss << "0x";
    for (uint8_t i : a){
        if (i < 0x10) ss << 0;
        ss << std::hex << (int) i;
    }
    ss >> s;
    return s;
}

int help(const char* argv[]){
    std::cerr << "Usage:" << std::endl
              << '\t' << argv[0] << " <file-path>" << std::endl
              << '\t' << argv[0] << " [options]" << " <file-path>" << std::endl << std::endl
              << "Options:" << std::endl
              << "\t--help\t\tShow help (the thing you're looking at)" << std::endl
              << "\t--rton2json\tForce covert RTON to JSON" << std::endl
              << "\t--json2rton\tForce covert JSON to RTON" << std::endl;
    std::cin.get();
    return 1;
}

int main(const int argc, const char* argv[]){
    std::clog << std::endl << "rton-json made by H3x4n1um" << std::endl
              << "Version: " << ver << std::endl
              << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl
              << "Credits: nlohmann for his awesome JSON parser and fifo_map" << std::endl << std::endl;

    if (argc > 3) return help(argv);
    //get file_path
    std::filesystem::path file_path;
    if (argc == 1){
        std::string temp;
        std::clog << "Enter file path: ";
        getline(std::cin, temp);
        file_path = temp;
        std::clog << std::endl;
    }
    else if (argc == 2){
        if (strcmp(argv[1], "--help") == 0) return help(argv);
        file_path = argv[1];
    }
    else file_path = argv[2];

    //check file exist
    if (!std::filesystem::is_regular_file(file_path)){
        std::cerr << "ERROR! CAN'T FIND FILE " << file_path << "!!!" << std::endl;
        std::cin.get();
        return 1;
    }

    //info
    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile Time"] = std::string(__DATE__) + ' ' + __TIME__;

    //detect rton or json
    bool is_rton = false;
    if (argc == 3){
        debug_js["Info"]["Mode"] = "Manual";
        debug_js["Info"]["Option"] = argv[1];
        if (strcmp(argv[1], "--rton2json") == 0) is_rton = true;
        else if (strcmp(argv[1], "--json2rton") != 0) return help(argv);
    }
    //else just mark as json
    else{
        debug_js["Info"]["Mode"] = "Auto";
        input.open(file_path, std::ifstream::binary);
        //check header
        char header[5];
        input.read(header, 4);
        header[4] = 0;
        if (strcmp(header, "RTON") == 0) is_rton = true;
        input.close();
    }
    debug_js["Info"]["File"] = file_path.string();

    debug.open(file_path.string() + "_log.json");

    //init RTON Stats
    debug_js["RTON Stats"]["RTON Version"] = 1; //not sure if it ever higher than 1
    debug_js["RTON Stats"]["List of Bytecodes"]["Offset"] = "Bytecode";
    debug_js["RTON Stats"]["0x91 Stack"]["Unsigned RTON Number"] = "String";
    debug_js["RTON Stats"]["0x93 Stack"]["Unsigned RTON Number"] = "UTF-8 String";

    //rton2json
    if (is_rton){
        std::clog << "RTON DETECTED" << std::endl;
        //read
        input.open(file_path, std::ifstream::binary);
        //write
        output.open(file_path.string() + ".json");
        output << std::setw(4) << json_decode();
        //close
        input.close();
        output.close();
    }
    //json2rton
    else{
        std::clog << "JSON DETECTED" << std::endl;
        //read
        input.open(file_path);
        //write
        output.open(file_path.string() + ".rton", std::ofstream::binary);
        rton_encode(); //write directly to file
        //close
        input.close();
        output.close();
    }
    std::clog << std::endl << "DONE";
    //log at the end
    debug << std::setw(4) << debug_js;
    debug.close();
    std::cin.get();
    return 0;
}
