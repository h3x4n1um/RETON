#include "include/rton-json.hpp"

#include "include/error.hpp"
#include "include/rton2json.hpp"
#include "include/RTON_number.hpp"

vector <string> stack_0x91;
vector <string> stack_0x93;

template <class T>
T read(){
    T res;
    input.read(reinterpret_cast <char*> (&res), sizeof res);
    return res;
}

vector <uint8_t> read_uRTON_t(){
    vector <uint8_t> RTON_t;
    uint8_t sub_num;
    do{
        sub_num = read<uint8_t>();
        RTON_t.push_back(sub_num);
    }while(sub_num > 0x7f);
    return RTON_t;
}

json read_RTON_block(){
    json res;

    uint8_t bytecode = read<uint8_t>();
    debug_js["RTON stats"]["List of bytecodes"][to_hex_string((uint64_t) input.tellg() - 1)] = to_hex_string(bytecode);

    switch (bytecode){
    //false
    case 0x0:{
        res.push_back(false);
        break;
    }
    //true
    case 0x1:{
        res.push_back(true);
        break;
    }
    //int8_t
    case 0x8:{
        res.push_back(read<int8_t>());
        break;
    }
    //uint8_t
    case 0xa:{
        res.push_back(read<uint8_t>());
        break;
    }
    //int16_t
    case 0x10:{
        res.push_back(read<int16_t>());
        break;
    }
    //uint16_t
    case 0x12:{
        res.push_back(read<uint16_t>());
        break;
    }
    //int32_t
    case 0x20:{
        res.push_back(read<int32_t>());
        break;
    }
    //float32
    case 0x22:{
        float tmp = read<float>();
        if (isinf(tmp)) res.push_back(tmp > 0 ? "inf" : "-inf");
        else
            if (isnan(tmp)) res.push_back("nan");
            else res.push_back(tmp);
        break;
    }
    //uint32_t
    case 0x26:{
        res.push_back(read<uint32_t>());
        break;
    }
    //int64_t
    case 0x40:{
        res.push_back(read<int64_t>());
        break;
    }
    //float64
    case 0x42:{
        double tmp = read<double>();
        if (isinf(tmp)) res.push_back(tmp > 0 ? "inf" : "-inf");
        else
            if (isnan(tmp)) res.push_back("nan");
            else res.push_back(tmp);
        break;
    }
    //uint64_t
    case 0x46:{
        res.push_back(read<uint64_t>());
        break;
    }
    //string
    case 0x81:{
        uint64_t buffer = uRTON_t2uint64_t(read_uRTON_t());

        char temp[buffer + 1];
        input.read(temp, buffer);
        temp[buffer] = 0;

        res.push_back(string(temp));
        break;
    }
    //utf-8 string
    case 0x82:{
        uint64_t buffer = uRTON_t2uint64_t(read_uRTON_t());
        buffer = uRTON_t2uint64_t(read_uRTON_t());

        char s[buffer + 1];
        input.read(s, buffer);
        s[buffer] = 0;

        res.push_back(string(s));
        break;
    }
    //RTID
    case 0x83:{
        uint8_t subset = read<uint8_t>();

        switch (subset){
        case 0x2:{
            uint64_t buffer = uRTON_t2uint64_t(read_uRTON_t());
            buffer = uRTON_t2uint64_t(read_uRTON_t());

            char s[buffer + 1];
            input.read(s, buffer);
            s[buffer] = 0;

            uint64_t second_uid = uRTON_t2uint64_t(read_uRTON_t());
            uint64_t first_uid = uRTON_t2uint64_t(read_uRTON_t());
            uint32_t third_uid = read<uint32_t>();

            stringstream ss;
            ss << dec << first_uid << '.' << second_uid << '.' << hex << third_uid;

            res.push_back(string("RTID(") + ss.str() + '@' + s + ')');
            break;
        }
        case 0x3:{
            uint64_t s1_buffer = uRTON_t2uint64_t(read_uRTON_t());
            s1_buffer = uRTON_t2uint64_t(read_uRTON_t());

            char s1[s1_buffer + 1];
            input.read(s1, s1_buffer);
            s1[s1_buffer] = 0;

            uint64_t s2_buffer = uRTON_t2uint64_t(read_uRTON_t());
            s2_buffer = uRTON_t2uint64_t(read_uRTON_t());

            char s2[s2_buffer + 1];
            input.read(s2, s2_buffer);
            s2[s2_buffer] = 0;

            res.push_back(string("RTID(") + s2 + '@' + s1 + ')');
            break;
        }
        default:{
            throw bytecode_error(subset);
        }
        }
        break;
    }
    //null
    case 0x84:{
        res.push_back(nullptr);
        break;
    }
    //sub-object
    case 0x85:{
        res.push_back(read_RTON());
        break;
    }
    //array
    case 0x86:{
        uint8_t arr_begin = read<uint8_t>();

        if (arr_begin == 0xfd){
            size_t arr_size = uRTON_t2uint64_t(read_uRTON_t());

            json arr = json::array();
            for (size_t i = 0; i < arr_size; ++i) arr.push_back(read_RTON_block()[0]);

            res.push_back(arr);

            uint8_t arr_end = read<uint8_t>();
            if (arr_end != 0xfe) throw bytecode_error(arr_end);
        }
        else throw bytecode_error(arr_begin);
        break;
    }
    //cached string
    case 0x90:{
        uint64_t buffer = uRTON_t2uint64_t(read_uRTON_t());

        char temp[buffer + 1];
        input.read(temp, buffer);
        temp[buffer] = 0;

        debug_js["RTON stats"]["0x91 stack"][to_hex_string(uint64_t2uRTON_t(stack_0x91.size()))] = string(temp);
        stack_0x91.push_back(temp);

        res.push_back(stack_0x91[stack_0x91.size() - 1]);
        break;
    }
    //recall
    case 0x91:{
        try{
            res.push_back(stack_0x91.at(uRTON_t2uint64_t(read_uRTON_t())));
        }
        catch(const out_of_range &oor){
            throw out_of_range_error(bytecode);
        }
        break;
    }
    //cached utf-8 string
    case 0x92:{
        uint64_t buffer = uRTON_t2uint64_t(read_uRTON_t());
        buffer = uRTON_t2uint64_t(read_uRTON_t());

        char temp[buffer + 1];
        input.read(temp, buffer);
        temp[buffer] = 0;

        debug_js["RTON stats"]["0x93 stack"][to_hex_string(uint64_t2uRTON_t(stack_0x93.size()))] = string(temp);
        stack_0x93.push_back(temp);

        res.push_back(stack_0x93[stack_0x93.size() - 1]);
        break;
    }
    //recall
    case 0x93:{
        try{
            res.push_back(stack_0x93.at(uRTON_t2uint64_t(read_uRTON_t())));
        }
        catch(const out_of_range &oor){
            throw out_of_range_error(bytecode);
        }
        break;
    }
    //end of object
    case 0xFF:{
        break;
    }

    //0???
    case 0x9:
    case 0xb:
    case 0x11:
    case 0x13:
    case 0x21:
    case 0x27:
    case 0x41:
    case 0x47:{
        res.push_back(0);
        break;
    }

    //0.0???
    case 0x23:
    case 0x43:{
        res.push_back(0.0);
        break;
    }

    //unsigned RTON number
    case 0x24:
    case 0x28:
    case 0x44:
    case 0x48:{
        res.push_back(uRTON_t2uint64_t(read_uRTON_t()));
        break;
    }

    //RTON number
    case 0x25:
    case 0x29:
    case 0x45:
    case 0x49:{
        int64_t num = uRTON_t2uint64_t(read_uRTON_t());
        if (num % 2) num = -(num + 1);
        num /= 2;

        res.push_back(num);
        break;
    }

    //else just exit error
    default:{
        throw bytecode_error(bytecode);
    }
    }
    return res;
}

json read_RTON(){
    json res, js_key = read_RTON_block();
    while(js_key.size() > 0){
        //prevent push entire array lol
        if (!js_key[0].is_string()) throw key_error();

        res[js_key[0].get<string>()] = read_RTON_block()[0];

        js_key = read_RTON_block();
    }
    return res;
}

json json_decode(){
    stack_0x91.clear();
    stack_0x93.clear();

    input.seekg((uint64_t) input.tellg() + 4); //skip RTON
    uint32_t RTON_ver = read<uint32_t>();
    debug_js["RTON stats"]["RTON version"] = RTON_ver;

    json js = read_RTON();

    if (input.peek() == EOF) clog << "Missing \"DONE\" at EOF?" << endl;
    else{
        char footer[5];
        input.read(footer, 4);
        footer[4] = 0;

        if (strcmp(footer, "DONE") != 0){
            input.seekg((uint8_t) input.tellg() - 3);
            throw eof_error(footer);
        }
    }

    return js;
}
