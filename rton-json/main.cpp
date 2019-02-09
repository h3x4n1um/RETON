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

const std::string ver = "2.0.0";

extern json init_json_decode();
extern int rton_encode();

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
    std::cout << "Usage: " << argv[0] << " [option] [file_path]\n"
    "\n[option]:\n"
    "\t-rton2json\tcovert RTON to JSON\n"
    "\t-json2rton\tcovert JSON to RTON\n";
    getch();
    return 1;
}

int main(const int argc, const char* argv[]){
    std::cout << "\nrton-json made by H3x4n1um version " + ver << std::endl;
    std::cout << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
    puts("Credits: nlohmann for his awesome JSON parser and fifo_map\n");

    if (argc != 3) return help(argv);

    //info
    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile Time"] = std::string(__DATE__) + ' ' + __TIME__;
    debug_js["Info"]["Option"] = argv[1];
    debug_js["Info"]["File"] = argv[2];

    //get file_name
    std::string file_name;
    file_name = argv[2];
    for (int i = file_name.size(); i > 0; --i){
        if (file_name[i] == '.'){
            file_name = file_name.substr(0, i);
            break;
        }
    }
    debug.open(file_name + "_log.json");

    //rton2json
    if (strcmp(argv[1], "-rton2json") == 0){
        //read
        input.open(argv[2], std::ifstream::binary);
        //write
        output.open(file_name + ".json");
        output << std::setw(4) << init_json_decode();
        //close
        input.close();
        output.close();
    }
    //json2rton
    else if (strcmp(argv[1], "-json2rton") == 0){
        //read
        input.open(argv[2]);
        //write
        output.open(file_name + ".rton", std::ofstream::binary);
        rton_encode(); //write directly to file
        output.close();
    }
    //else
    else return help(argv);
    puts("Done!");
    //log at the end
    debug << std::setw(4) << debug_js;
    debug.close();
    return 0;
}
