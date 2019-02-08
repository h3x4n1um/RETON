#include <chrono>
#include <conio.h>
#include <ctime>
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

const std::string ver = "1.1.1";

json read_block_RTON();
json json_decode();
std::string rton_encode();

std::ifstream input;
std::ofstream output, debug;
std::string file_path;
json debug_js;

std::string to_hex_string(uint64_t q){
    if (q == 0) return "0x0";
    std::string s;
    std::stringstream ss;
    ss << std::hex << std::showbase << q;
    ss >> s;
    return s;
}

int help(const char* argv[]){
    std::cout << "Usage: " << argv[0] << " [option] [file_path]\n"
    "\n[option]:\n"
    "\t-rton2json\tcovert RTON to JSON\n"
    "\t-json2rton\tcovert JSON to RTON (to be implemented)\n";
    getch();
    return 1;
}

int not_RTON(){
    std::cout << "ERROR! THIS FILE IS NOT RTON FORMAT!!!\n";
    return 1;
}

int main(const int argc, const char* argv[]){
    std::cout << "\nrton-json made by H3x4n1um version " + ver << std::endl;
    std::cout << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
    puts("Credits: nlohmann for his awesome JSON parser and fifo_map\n");

    if (argc != 3) return help(argv);

    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile Time"] = std::string(__DATE__) + ' ' + __TIME__;
    debug_js["Info"]["Option"] = argv[1];
    debug_js["Info"]["File"] = argv[2];

    //rton2json
    if (strcmp(argv[1], "-rton2json") == 0){

        //get file_path
        file_path = argv[2];
        input.open(file_path, std::ifstream::binary);
        for (int i = file_path.size(); i > 0; --i){
            if (file_path[i] == '.'){
                file_path = file_path.substr(0, i);
                break;
            }
        }

        //header
        char header[5];
        input.read(header, 4);
        header[4] = 0;
        if (strcmp(header, "RTON") != 0) return not_RTON();
        uint32_t RTON_ver;
        input.read(reinterpret_cast <char*> (&RTON_ver), sizeof RTON_ver);

        debug_js["RTON Stats"]["RTON Version"] = RTON_ver;
        debug_js["RTON Stats"]["List of Bytecodes"].push_back("Offset: Bytecode");
        debug_js["RTON Stats"]["0x91 Stack"].push_back("Unsigned RTON Number: String");

        //Write
        output.open(file_path + ".json");
        debug.open(file_path + "_log.json");
        output << std::setw(4) << json_decode();
        debug << std::setw(4) << debug_js;

        puts("Done!");

        //Close
        input.close();
        debug.close();
        output.close();
    }
    //json2rton
    else if (strcmp(argv[1], "-json2rton") == 0){
        puts("To be implemented");
        return 1;
    }
    //else
    else return help(argv);
    return 0;
}
