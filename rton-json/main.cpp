#include <conio.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const std::string ver = "2.3.0";

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
    std::clog << "Usage:" << std::endl << std::endl;
    std::clog << argv[0] << " [file_path]\t(auto detect)" << std::endl;
    std::clog << argv[0] << " [option]\t(manual)" << std::endl;
    std::clog << "\n[option]:" << std::endl
              << "\t--help\t\t\tshow help (the thing you're looking)" << std::endl
              << "\t--rton2json [file_path]\tcovert [file_path] RTON to JSON" << std::endl
              << "\t--json2rton [file_path]\tcovert [file_path] JSON to RTON" << std::endl;
    getch();
    return 1;
}

int main(const int argc, const char* argv[]){
    std::clog << std::endl << "rton-json made by H3x4n1um version " << ver << std::endl;
    std::clog << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
    puts("Credits: nlohmann for his awesome JSON parser and fifo_map\n");

    if (argc > 3) return help(argv);

    //get file_path
    std::string file_path;
    if (argc == 1){
        std::clog << "Enter file path: ";
        getline(std::cin, file_path);
        puts("");
    }
    else if (argc == 2){
        if (strcmp(argv[1], "--help") == 0) return help(argv);
        file_path = argv[1];
    }
    else file_path = argv[2];

    //check file exist
    input.open(file_path);
    if (!input.good()){
        puts("ERROR! FILE NOT FOUND!!!");
        getch();
        return 1;
    }
    input.close();

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
        char header[4];
        input.read(header, 4);
        if (strcmp(header, "RTON") == 0) is_rton = true;
        input.close();
    }
    debug_js["Info"]["File"] = file_path;

    //remove extension
    std::string file_name;
    for (int i = file_path.size(); i > 0; --i){
        if (file_path[i] == '.'){
            file_name = file_path.substr(0, i);
            break;
        }
    }
    debug.open(file_name + "_log.json");

    //rton2json
    if (is_rton){
        std::clog << "RTON DETECTED" << std::endl;
        //read
        input.open(file_path, std::ifstream::binary);
        //write
        output.open(file_name + ".json");
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
        output.open(file_name + ".rton", std::ofstream::binary);
        rton_encode(); //write directly to file
        output.close();
    }
    puts("\nDONE!");
    //log at the end
    debug << std::setw(4) << debug_js;
    debug.close();
    getch();
    return 0;
}
