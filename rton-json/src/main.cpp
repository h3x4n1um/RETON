#include <filesystem>
#include <iostream>

#include "include/rton-json.hpp"

#include "include/json2rton.hpp"
#include "include/rton2json.hpp"

ifstream input;
ofstream output, debug;
json debug_js, rton_list, json_list;

int help(const char *argv[]){
    cerr << "Usage:" << endl
         << '\t' << argv[0] << " <file-or-folder-path>" << endl
         << '\t' << argv[0] << " [options]" << " <file-or-folder-path>" << endl
         << endl
         << "Options:" << endl
         << "\t--help\t\tShow help (the thing you're looking at)" << endl
         << "\t--rton2json\tForce covert RTON to JSON" << endl
         << "\t--json2rton\tForce covert JSON to RTON" << endl;
    cin.get();
    return 1;
}

int process_file(filesystem::path file_name, const int argc, const char *argv[]){
    debug_js.clear();
    clog << "Processing file " << file_name;

    //info
    debug_js["Info"]["Log"] = "This log file created by rton-json made by H3x4n1um";
    debug_js["Info"]["Executable"] = argv[0];
    debug_js["Info"]["Version"] = ver;
    debug_js["Info"]["Compile time"] = string(__DATE__) + ' ' + __TIME__;

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

        input.open(file_name, ifstream::binary);

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
            filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            clog << " - JSON Detected" << endl;
            json_list["JSON files"].push_back(file_name.string());

            input.open(file_name);
            filesystem::create_directory(file_name.parent_path() / "json2rton");
            output.open((file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton", ofstream::binary);
            rton_encode(); //write directly to file

            input.close();
            output.close();

            //log at the end
            debug << setw(4) << debug_js;
            debug.close();
            clog << "Done" << endl
                 << endl;

            break;
        }
        case 1:{
            filesystem::create_directory(file_name.parent_path() / "log");
            debug.open((file_name.parent_path() / "log" / file_name.filename()).string() + "_log.json");

            clog << " - RTON Detected" << endl;
            rton_list["RTON files"].push_back(file_name.string());

            input.open(file_name, ifstream::binary);
            filesystem::create_directory(file_name.parent_path() / "rton2json");
            output.open((file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json");
            output << setw(4) << json_decode();

            input.close();
            output.close();

            //log at the end
            debug << setw(4) << debug_js;
            debug.close();
            clog << "Done" << endl
                 << endl;

            break;
        }
        default:{
            clog << " - Unknown" << endl
                 << "Skipped" << endl
                 << endl;
        }
        }
    }
    catch (int e){
        //reset fstream state
        input.close();
        output.close();
        debug.close();

        //remove unfinished file
        filesystem::path out_file;
        if (file_type) out_file = (file_name.parent_path() / "rton2json" / file_name.stem()).string() + ".json";
        else out_file = (file_name.parent_path() / "json2rton" / file_name.stem()).string() + ".rton";
        filesystem::remove(out_file);

        clog << "Error code: " << e << endl
             << endl;
        return e;
    }
    return 0;
}

int main(const int argc, const char *argv[]){
    clog << endl
         << "rton-json made by H3x4n1um" << endl
         << "Version: " << ver << endl
         << architecture << " executable" << endl
         << "Compiled on " << __DATE__ << " at " << __TIME__ << endl
         << "Credits: nlohmann for his awesome JSON parser and fifo_map" << endl
         << endl;

    filesystem::path path;
    switch (argc){
    case 1:{
        string temp;
        clog << "Enter file or folder path: ";
        getline(cin, temp);
        path = temp;
        clog << endl;
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

    if (!filesystem::is_regular_file(path) && !filesystem::is_directory(path)){
        cerr << "Error! Can't find file or folder " << path << "!!!" << endl;
        cin.get();
        return 1;
    }

    if (filesystem::is_regular_file(path)){
        process_file(path, argc, argv);
        path = path.parent_path();
    }
    else{
        for (auto &file : filesystem::directory_iterator(path)){
            if (filesystem::is_regular_file(file)){
                process_file(file.path(), argc, argv);
            }
        }
    }

    rton_list["Total files"] = rton_list["RTON files"].size();
    json_list["Total files"] = json_list["JSON files"].size();

    //log processed json & rton
    debug.open(path / "json2rton" / "json2rton.json");
    debug << setw(4) << json_list;
    debug.close();
    debug.open(path / "rton2json" / "rton2json.json");
    debug << setw(4) << rton_list;
    debug.close();

    clog << "Finished" << endl;
    cin.get();
    return 0;
}
