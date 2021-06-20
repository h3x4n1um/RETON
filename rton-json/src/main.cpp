#include <filesystem>
#include <fstream>
#include <iostream>

#include "include/json2rton.hpp"
#include "include/rton-json.hpp"
#include "include/rton2json.hpp"

enum FILE_TYPE {
    JSON,
    RTON,
    UNKNOWN
};

int help(const std::vector <std::string> &arg){
    std::cout << "Usage:\t" << arg.front() << " [options]" << " <file or folder>" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "\t--help\t\tShow help" << std::endl
              << "\t--noui\t\tSkip UI" << std::endl
              << "\t--rton2json\tForce covert RTON to JSON" << std::endl
              << "\t--json2rton\tForce covert JSON to RTON" << std::endl
              << std::endl;
    return 1;
}

int process_file(const std::vector <std::string> &arg, const std::filesystem::path &file_name, FILE_TYPE file_type, reton::fifo_json &json_list, reton::fifo_json &rton_list){
    std::chrono::time_point<std::chrono::steady_clock> time_start = std::chrono::steady_clock::now();

    std::ifstream input;
    std::ofstream output, debug;

    std::cout << "Processing file " << file_name;

    //info
    reton::fifo_json debug_js;

    debug_js["Info"]["Log"]             = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"]      = arg.front();
    debug_js["Info"]["Version"]         = ver;
    debug_js["Info"]["Compile time"]    = std::string(__DATE__) + ' ' + __TIME__;
    debug_js["Info"]["Options"]         = arg;

    //detect json or rton or unknown
    if (file_type != UNKNOWN){
        debug_js["Info"]["Mode"] = "Manual";
    }
    else{
        debug_js["Info"]["Mode"] = "Auto";

        input.open(file_name, std::ios::binary);

        char header[5];
        input.read(header, 4);
        header[4] = '\0';

        if (strcmp(header, "RTON") == 0) file_type = RTON;
        else{
            try{
                input.seekg(0);
                reton::fifo_json js = reton::fifo_json::parse(input);
                file_type = JSON;
            }
            catch(reton::fifo_json::exception &e){
            }
        }

        input.close();
    }
    debug_js["Info"]["File"] = file_name.string();

    //init RTON Stats
    reton::fifo_json rton_info;

    rton_info["RTON version"]                       = 1;                //not sure if it ever higher than 1
    rton_info["List of chunks"]["Offset"]           = "Chunk type";
    rton_info["0x91 array"]["Unsigned RTON number"] = "String";
    rton_info["0x93 array"]["Unsigned RTON number"] = "UTF-8 string";

    try{
        switch(file_type){
        case JSON:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::cout << " - JSON Detected" << std::endl;
            json_list["JSON files"].push_back(file_name.string());

            input.open(file_name);
            reton::fifo_json json_file = reton::fifo_json::parse(input);
            input.close();

            std::filesystem::create_directory(file_name.parent_path() / "json2rton");
            output.open((file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton", std::ios::binary);
            std::vector <uint8_t> result = json2rton(json_file, rton_info);
            output.write(reinterpret_cast <const char*>(result.data()), result.size());
            output.close();

            //log at the end
            debug_js["RTON info"] = rton_info;
            debug << std::setw(4) << debug_js;
            break;
        }
        case RTON:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::cout << " - RTON Detected" << std::endl;
            rton_list["RTON files"].push_back(file_name.string());

            input.open(file_name, std::ios::binary);
            std::vector <uint8_t> raw_file;
            std::for_each(std::istreambuf_iterator<char>(input),
                          std::istreambuf_iterator<char>(),
                          [&raw_file](const char c){
                                raw_file.push_back(c);
                          });
            input.close();

            std::filesystem::create_directory(file_name.parent_path() / "rton2json");
            output.open((file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json");
            output << std::setw(4) << rton2json(raw_file, rton_info);
            output.close();

            //log at the end
            debug_js["RTON info"] = rton_info;
            debug << std::setw(4) << debug_js;
            break;
        }
        default:{
            std::cout << " - Unknown" << std::endl;
        }
        }
    }
    catch (std::exception &e){
        debug_js["RTON info"] = rton_info;
        debug << std::setw(4) << debug_js;

        std::cerr << e.what() << std::endl;

        //remove unfinished file
        std::filesystem::path out_file;
        if (file_type == RTON) out_file = (file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json";
        else if (file_type == JSON) out_file = (file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton";

        output.close();
        std::filesystem::remove(out_file);
    }

    std::chrono::time_point<std::chrono::steady_clock> time_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_process = time_end - time_start;

    std::cout << "Completed in " << time_process.count() << "s" << std::endl
              << std::endl;

    return 0;
}

int main(int argc, char *argv[]){
    // init arg
    std::vector <std::string> arg;
    for (int i = 0; i < argc; ++i) arg.push_back(argv[i]);

    // init arg default value
    bool noui = false;
    FILE_TYPE file_type = UNKNOWN;

    std::cout << std::endl
              << "rton-json made by H3x4n1um" << std::endl
              << std::endl
              << "Version: " << ver << " " << architecture << std::endl
              << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl
              << "Credits: nlohmann for his awesome JSON parser and fifo_map" << std::endl
              << std::endl;

    std::filesystem::path path;
    if (arg.size() == 1) {
        std::string temp;
        std::cout << "Enter file or folder path: ";
        getline(std::cin, temp);
        path = temp;
        std::cout << std::endl;
    }
    // arg parser, simple but it works :P
    else {
        if (arg.size() == 2 && arg.back() == "--help") return help(arg);    // special case for help

        for (uint64_t i = 1; i+1 < arg.size(); ++i){                        // skip arg.front() and arg.back() and get opt
            if (arg.at(i) == "--help") return help(arg);
            else if (arg.at(i) == "--noui") noui = true;
            else if (arg.at(i) == "--rton2json") file_type = RTON;
            else if (arg.at(i) == "--json2rton") file_type = JSON;
            // invalid arg :P
            else return help(arg);
        }

        path = arg.back();
    }

    if (!(std::filesystem::is_regular_file(path) || std::filesystem::is_directory(path))){
        std::cerr << "Error! Can't find file or folder " << path << "!!!" << std::endl;
        if (!noui) std::cin.get();
        else std::cout << std::endl;
        return 1;
    }

    reton::fifo_json rton_list, json_list;

    if (std::filesystem::is_regular_file(path)){
        process_file(arg, path, file_type, json_list, rton_list);
        path = path.parent_path();
    }
    else{
        for (std::filesystem::directory_entry file : std::filesystem::directory_iterator(path)){
            if (std::filesystem::is_regular_file(file)){
                process_file(arg, file.path(), file_type, json_list, rton_list);
            }
        }
    }

    rton_list["Total files"] = rton_list["RTON files"].size();
    json_list["Total files"] = json_list["JSON files"].size();

    std::ofstream result;
    //log processed json & rton
    result.open(path / "log" / "json2rton.json");
    result << std::setw(4) << json_list;
    result.close();
    result.open(path / "log" / "rton2json.json");
    result << std::setw(4) << rton_list;
    result.close();

    std::cout << "Finished" << std::endl;
    if (!noui) std::cin.get();
    else std::cout << std::endl;
    return 0;
}
