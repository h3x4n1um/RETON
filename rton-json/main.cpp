#include <iostream>
#include <iomanip>
#include <fstream>

#include "json.hpp"
#include "fifo_map.hpp"

using namespace std;
/// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
/// https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

json read_block_RTON();
json json_decode();
string rton_encode();

ifstream input;
ofstream output, debug;
string file_path;

int help(const char * argv[]){
    cout << "Usage: " << argv[0] << " [option] [file_path]\n"
    "\n[option]:\n"
    "\t-rton2json\tcovert RTON to JSON\n"
    "\t-json2rton\tcovert JSON to RTON (to be implemented)\n";
    cin.get();
    return 1;
}

int not_RTON(){
    cout << "ERROR! THIS FILE IS NOT RTON FORMAT!!!\n";
    return 1;
}

int main(const int argc, const char * argv[]){
    puts("\nrton-json made by H3x4n1um version 1.1.1");
    puts("Credits: nlohmann for his awesome JSON parser and fifo_map\n");

    if (argc != 3) return help(argv);
    ///rton2json
    if (strcmp(argv[1], "-rton2json") == 0){
        ///get file_path
        file_path = argv[2];
        input.open(file_path, ifstream::binary);
        for (int i = file_path.size(); i > 0; --i){
            if (file_path[i] == '.'){
                file_path = file_path.substr(0, i);
                break;
            }
        }

        ///header
        char header[5];
        input.read(header, 4);
        header[4] = 0;
        if (strcmp(header, "RTON") != 0) return not_RTON();
        uint32_t RTON_ver;
        input.read(reinterpret_cast <char*> (&RTON_ver), sizeof RTON_ver);
        cout << header << " version: " << RTON_ver << endl;

        ///Write
        debug.open(file_path + ".log");
        output.open(file_path + ".json");
        output << setw(4) << json_decode();

        puts("\nDone!");

        ///Close
        input.close();
        debug.close();
        output.close();
    }
    ///json2rton
    else if (strcmp(argv[1], "-json2rton") == 0){
        puts("To be implemented");
        return 1;
    }
    ///else
    else return help(argv);
    return 0;
}
