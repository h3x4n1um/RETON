#include "include/rton2json.hpp"

#include "include/error.hpp"
#include "include/rton-json.hpp"
#include "include/RTON_number.hpp"

template <class T>
T get_data(const std::vector <uint8_t> &byte_array, std::size_t &pos){
    std::size_t pre_pos = pos;
    pos += sizeof(T);
    std::vector<uint8_t> sub_byte_array(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));
    return *reinterpret_cast<T*>(sub_byte_array.data());
}

std::vector <uint8_t> get_uRTON_t(const std::vector<uint8_t> &byte_array, std::size_t &pos){
    std::size_t pre_pos = pos;
    for(;byte_array.at(pos) > 0x7f; ++pos);
    ++pos;
    return std::vector <uint8_t>(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));
}

json_fifo::json decode_RTON_chunk(const std::vector <uint8_t> &byte_array, std::size_t &pos, std::vector<std::string> &array_0x91, std::vector<std::string> &array_0x93, json_fifo::json &rton_info){
    uint8_t chunk_type = byte_array.at(pos);
    ++pos;
    rton_info["List of chunks"][to_hex_string(pos-1)] = to_hex_string(chunk_type);

    switch(chunk_type){
    //false
    case 0x0:{
        return false;
        break;
    }
    //true
    case 0x1:{
        return true;
        break;
    }
    //int8_t
    case 0x8:{
        return get_data<int8_t>(byte_array, pos);
        break;
    }
    //uint8_t
    case 0xa:{
        return get_data<uint8_t>(byte_array, pos);
        break;
    }
    //int16_t
    case 0x10:{
        return get_data<int16_t>(byte_array, pos);
        break;
    }
    //uint16_t
    case 0x12:{
        return get_data<uint16_t>(byte_array, pos);
        break;
    }
    //int32_t
    case 0x20:{
        return get_data<int32_t>(byte_array, pos);
        break;
    }
    //float32
    case 0x22:{
        float tmp = get_data<decltype(tmp)>(byte_array, pos);
        if (std::isinf(tmp)) return tmp > 0 ? "inf" : "-inf";
        if (std::isnan(tmp)) return "nan";
        return tmp;
        break;
    }
    //uint32_t
    case 0x26:{
        return get_data<uint32_t>(byte_array, pos);
        break;
    }
    //int64_t
    case 0x40:{
        return get_data<int64_t>(byte_array, pos);
        break;
    }
    //float64
    case 0x42:{
        double tmp = get_data<decltype(tmp)>(byte_array, pos);
        if (std::isinf(tmp)) return tmp > 0 ? "inf" : "-inf";
        if (std::isnan(tmp)) return "nan";
        return tmp;
        break;
    }
    //uint64_t
    case 0x46:{
        return get_data<uint64_t>(byte_array, pos);
        break;
    }
    //string
    case 0x81:{
        uint64_t buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

        std::size_t pre_pos = pos;
        pos += buffer;
        return std::string(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));;
        break;
    }
    //utf-8 string
    case 0x82:{
        uint64_t buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
        buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

        std::size_t pre_pos = pos;
        pos += buffer;
        return std::string(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));
        break;
    }
    //RTID
    case 0x83:{
        uint8_t subset = get_data<decltype(subset)>(byte_array, pos);

        switch (subset){
        case 0x2:{
            uint64_t buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
            buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

            std::size_t pre_pos = pos;
            pos += buffer;
            std::string s(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));

            uint64_t second_uid = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
            uint64_t first_uid = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
            uint32_t third_uid = get_data<decltype(third_uid)>(byte_array, pos);

            std::stringstream ss;
            ss << std::dec << first_uid << '.' << second_uid << '.' << std::hex << third_uid;

            return std::string("RTID(") + ss.str() + '@' + s + ')';
            break;
        }
        case 0x3:{
            uint64_t s1_buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
            s1_buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

            std::size_t pre_pos = pos;
            pos += s1_buffer;
            std::string s1(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));

            uint64_t s2_buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
            s2_buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

            pre_pos = pos;
            pos += s2_buffer;
            std::string s2(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));

            return std::string("RTID(") + s2 + '@' + s1 + ')';
            break;
        }
        default:{
            throw std::logic_error(chunk_error(pos, subset));
        }
        }
        break;
    }
    //null
    case 0x84:{
        return nullptr;
        break;
    }
    //sub-object
    case 0x85:{
        return decode_RTON(byte_array, pos, array_0x91, array_0x93, rton_info);
        break;
    }
    //array
    case 0x86:{
        uint8_t arr_begin = get_data<decltype(arr_begin)>(byte_array, pos);

        if (arr_begin == 0xfd){
            std::size_t arr_size = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

            json_fifo::json arr = json_fifo::json::array();
            for (std::size_t i = 0; i < arr_size; ++i) arr.push_back(decode_RTON_chunk(byte_array, pos, array_0x91, array_0x93, rton_info));

            uint8_t arr_end = get_data<decltype(arr_end)>(byte_array, pos);
            if (arr_end != 0xfe) throw std::logic_error(chunk_error(pos, arr_end));

            return arr;
        }
        else throw std::logic_error(chunk_error(pos, arr_begin));
        break;
    }
    //cached string
    case 0x90:{
        uint64_t buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

        std::size_t pre_pos = pos;
        pos += buffer;
        std::string temp(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));

        rton_info["0x91 array"][to_hex_string(uint64_t2uRTON_t(array_0x91.size()))] = temp;
        array_0x91.push_back(temp);

        return array_0x91.at(array_0x91.size() - 1);
        break;
    }
    //recall
    case 0x91:{
        return array_0x91.at(uRTON_t2uint64_t(get_uRTON_t(byte_array, pos)));
        break;
    }
    //cached utf-8 string
    case 0x92:{
        uint64_t buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
        buffer = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));

        std::size_t pre_pos = pos;
        pos += buffer;
        std::string temp(std::next(byte_array.begin(), pre_pos), std::next(byte_array.begin(), pos));

        rton_info["0x93 array"][to_hex_string(uint64_t2uRTON_t(array_0x93.size()))] = temp;
        array_0x93.push_back(temp);

        return array_0x93.at(array_0x93.size() - 1);
        break;
    }
    //recall
    case 0x93:{
        return array_0x93.at(uRTON_t2uint64_t(get_uRTON_t(byte_array, pos)));
        break;
    }
    //end of object
    case 0xFF:{
        return json_fifo::json();
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
        return 0;
        break;
    }

    //0.0???
    case 0x23:
    case 0x43:{
        return 0.0;
        break;
    }

    //unsigned RTON number
    case 0x24:
    case 0x28:
    case 0x44:
    case 0x48:{
        return uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
        break;
    }

    //RTON number
    case 0x25:
    case 0x29:
    case 0x45:
    case 0x49:{
        int64_t num = uRTON_t2uint64_t(get_uRTON_t(byte_array, pos));
        if (num % 2) num = -(num + 1);
        num /= 2;

        return num;
        break;
    }

    //else just exit error
    default:{
        throw std::logic_error(chunk_error(pos, chunk_type));
    }
    }
}

json_fifo::json decode_RTON(const std::vector <uint8_t> &byte_array, std::size_t &pos, std::vector<std::string> &array_0x91, std::vector<std::string> &array_0x93, json_fifo::json &rton_info){
    std::size_t pre_pos = pos;
    json_fifo::json res,
                    js_key = decode_RTON_chunk(byte_array, pos, array_0x91, array_0x93, rton_info);
    while(js_key.size()){
        if(!js_key.is_string()) throw std::logic_error("Error! Key at " + to_hex_string(pre_pos) + " is not a string!!!");

        res[js_key.get<std::string>()] = decode_RTON_chunk(byte_array, pos, array_0x91, array_0x93, rton_info);

        pre_pos = pos;
        js_key = decode_RTON_chunk(byte_array, pos, array_0x91, array_0x93, rton_info);
    }
    return res;
}

json_fifo::json rton2json(const std::vector <uint8_t> &byte_array, json_fifo::json &rton_info){
    std::size_t pos = 8;//skip RTON
    std::vector<std::string> array_0x91,
                             array_0x93;
    json_fifo::json res = decode_RTON(byte_array, pos, array_0x91, array_0x93, rton_info);

    if (pos == byte_array.size()) std::cout << "Missing \"DONE\" at EOF?" << std::endl;
    else{
        std::string footer(std::next(byte_array.begin(), pos), std::next(byte_array.begin(), pos+4));

        if (footer != "DONE") throw std::logic_error("Error! End of RTON reached, expected \"DONE\" but found \"" + footer + "\" at " + to_hex_string(pos));
    }

    return res;
}
