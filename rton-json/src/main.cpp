#include <filesystem>
#include <fstream>
#include <iostream>

#include "include/rton-json.hpp"

#include "include/json2rton.hpp"
#include "include/rton2json.hpp"

int help(const char *argv[]){
    std::cout << "Usage:\t" << argv[0] << " [options]" << " <file or folder>" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "\t--help\t\tShow help" << std::endl
              << "\t--rton2json\tForce covert RTON to JSON" << std::endl
              << "\t--json2rton\tForce covert JSON to RTON" << std::endl;
    std::cin.get();
    return 1;
}

int process_file(const std::filesystem::path &file_name, const int &argc, const char *argv[], json_fifo::json &json_list, json_fifo::json &rton_list){
    std::ifstream input;
    std::ofstream output, debug;

    std::cout << "Processing file " << file_name;

    //info
    json_fifo::json debug_js;

    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile time"] = std::string(__DATE__) + ' ' + __TIME__;

    //detect json or rton or unknown
    enum FILE_TYPE{
        JSON,
        RTON,
        UNKNOWN
    } in_file = UNKNOWN;

    if (argc == 3){
        debug_js["Info"]["Mode"] = "Manual";
        if (strcmp(argv[1], "--rton2json") == 0) in_file = RTON;
        else if (strcmp(argv[1], "--json2rton") == 0) in_file = JSON;
        debug_js["Info"]["Option"] = argv[1];
    }
    else{
        debug_js["Info"]["Mode"] = "Auto";

        input.open(file_name, std::ios_base::binary);

        char header[5];
        input.read(header, 4);
        header[4] = 0;

        if (strcmp(header, "RTON") == 0) in_file = RTON;
        else{
            try{
                input.seekg(0);
                json_fifo::json js = json_fifo::json::parse(input);
                in_file = JSON;
            }
            catch(json_fifo::json::exception &e){
            }
        }

        input.close();
    }
    debug_js["Info"]["File"] = file_name.string();

    //init RTON Stats
    json_fifo::json rton_info;

    rton_info["RTON version"] = 1; //not sure if it ever higher than 1
    rton_info["List of chunks"]["Offset"] = "Chunk type";
    rton_info["0x91 array"]["Unsigned RTON number"] = "String";
    rton_info["0x93 array"]["Unsigned RTON number"] = "UTF-8 string";

    try{
        switch(in_file){
        case JSON:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::cout << " - JSON Detected" << std::endl;
            json_list["JSON files"].push_back(file_name.string());

            input.open(file_name);
            json_fifo::json json_file = json_fifo::json::parse(input);
            input.close();

            std::filesystem::create_directory(file_name.parent_path() / "json2rton");
            output.open((file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton", std::ios_base::binary);
            std::vector <uint8_t> result = json2rton(json_file, rton_info);
            output.write(reinterpret_cast <const char*>(result.data()), result.size());
            output.close();

            //log at the end
            debug_js["RTON info"] = rton_info;
            debug << std::setw(4) << debug_js;

            std::cout << "Done" << std::endl
                      << std::endl;

            break;
        }
        case RTON:{
            std::filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            std::cout << " - RTON Detected" << std::endl;
            rton_list["RTON files"].push_back(file_name.string());

            input.open(file_name, std::ios_base::binary);
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

            std::cout << "Done" << std::endl
                      << std::endl;

            break;
        }
        default:{
            std::cout << " - Unknown" << std::endl
                      << "Skipped" << std::endl
                      << std::endl;
        }
        }
    }
    catch (const std::exception &e){
        debug_js["RTON info"] = rton_info;
        debug << std::setw(4) << debug_js;

        std::cout << e.what() << std::endl
                  << std::endl;

        //remove unfinished file
        std::filesystem::path out_file;
        if (in_file == RTON) out_file = (file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json";
        else out_file = (file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton";

        output.close();
        std::filesystem::remove(out_file);
    }
    return 0;
}

int main(const int argc, const char *argv[]){
    std::cout << std::endl
              << "rton-json made by H3x4n1um" << std::endl
              << std::endl
              << "Version: " << ver << " - " << architecture << " executable" << std::endl
              << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl
              << "Credits: nlohmann for his awesome JSON parser and fifo_map" << std::endl
              << std::endl;

    std::filesystem::path path;
    switch (argc){
    case 1:{
        std::string temp;
        std::cout << "Enter file or folder path: ";
        getline(std::cin, temp);
        path = temp;
        std::cout << std::endl;
        break;
    }
    case 2:{
        if (strcmp(argv[1], "--help") == 0) return help(argv);
        path = argv[1];
        break;
    }
    case 3:{
        path = argv[2];
        if (!(strcmp(argv[1], "--rton2json") == 0 || strcmp(argv[1], "--json2rton") == 0)) return help(argv);
        break;
    }
    default:{
        return help(argv);
    }
    }

    if (!(std::filesystem::is_regular_file(path) || std::filesystem::is_directory(path))){
        std::cerr << "Error! Can't find file or folder " << path << "!!!" << std::endl;
        std::cin.get();
        return 1;
    }

    json_fifo::json rton_list, json_list;

    if (std::filesystem::is_regular_file(path)){
        process_file(path, argc, argv, json_list, rton_list);
        path = path.parent_path();
    }
    else{
        for (std::filesystem::directory_entry file : std::filesystem::directory_iterator(path)){
            if (std::filesystem::is_regular_file(file)){
                process_file(file.path(), argc, argv, json_list, rton_list);
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
    std::cin.get();
    return 0;
}
