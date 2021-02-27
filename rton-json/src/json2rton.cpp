#include "include/json2rton.hpp"

#include <sstream>
#include <regex>

#include "include/error.hpp"
#include "include/rton-json.hpp"
#include "include/RTON_number.hpp"

enum CHUNK_TYPE : uint8_t{
    FLOAT64         = 0x42,
    uRTON_t         = 0x48,
    RTON_t          = 0x49,
    null            = 0x84,
    RTID            = 0x83,
    OBJECT          = 0x85,
    ARRAY           = 0x86,
    ASCII           = 0x90,
    ASCII_ARRAY     = 0x91,
    UTF8            = 0x92,
    UTF8_ARRAY      = 0x93,
    ARRAY_BEGIN     = 0xfd,
    ARRAY_END       = 0xfe,
    OBJECT_END      = 0xff
};

template <class T>
std::vector <uint8_t> set_raw_data(const T &val){
    std::vector <uint8_t> res(sizeof(T));
    *(reinterpret_cast<T*>(res.data())) = val;
    return res;
}

//https://en.wikipedia.org/wiki/UTF-8#Examples
std::size_t get_utf8_size(const std::string &q){
    std::size_t utf8_size = 0;
    for (uint8_t i : q){
        if (i <= 0b01111111) ++utf8_size;
        else if (i >= 0b11000010 && i <= 0b11011111) ++utf8_size;
        else if (i >= 0b11100001 && i <= 0b11101111) ++utf8_size;
        else if (i >= 0b11110000 && i <= 0b11110111) ++utf8_size;
    }
    return utf8_size;
}

std::vector <uint8_t> encode_JSON_chunk(const json_fifo::json &js, std::unordered_map <std::string, uint64_t> &map_0x91, std::unordered_map <std::string, uint64_t> &map_0x93, std::size_t &pos, json_fifo::json &rton_info){
    std::vector <uint8_t> res;

    switch(js.type()){
    case json_fifo::json::value_t::null:{
        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(null);

        res.push_back(CHUNK_TYPE::null);
        break;
    }
    case json_fifo::json::value_t::boolean:{
        bool temp = js.get<decltype(temp)>();

        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(temp);

        res.push_back(temp);
        break;
    }
    case json_fifo::json::value_t::string:{
        std::string temp = js.get<decltype(temp)>();
        //rtid
        if (regex_match(temp, std::regex("RTID(.*@.*)"))){
            res.push_back(CHUNK_TYPE::RTID);

            //delete "RTID(" and ")"
            temp.erase(std::prev(temp.end(), 1));
            temp.erase(0, 5);

            std::string s1 = temp.substr(temp.find("@") + 1),
                        s2 = temp.substr(0, temp.find("@"));

            uint8_t subset = 0x83;
            if (regex_match(s2, std::regex("\\d+\\.\\d+\\.[0-9a-fA-F]+"))) subset = 0x2;

            rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::RTID*0x100 + subset);

            res.push_back(subset);

            std::vector<uint8_t> s1_utf8_size = uint64_t2uRTON_t(get_utf8_size(s1)),
                                 s1_size = uint64_t2uRTON_t(s1.size());

            res.insert(res.end(), s1_utf8_size.begin(), s1_utf8_size.end());
            res.insert(res.end(), s1_size.begin(), s1_size.end());
            res.insert(res.end(), s1.begin(), s1.end());

            if (subset == 0x2){
                std::stringstream raw_ss(s2), ss;
                std::string tmp_str;
                uint64_t uid1, uid2;
                uint32_t uid3;

                while(getline(raw_ss, tmp_str, '.')) ss << tmp_str << " ";
                ss >> std::dec >> uid1 >> uid2 >> std::hex >> uid3;

                std::vector <uint8_t> byte_array_uid1 = uint64_t2uRTON_t(uid1),
                                      byte_array_uid2 = uint64_t2uRTON_t(uid2),
                                      byte_array_uid3 = set_raw_data(uid3);

                res.insert(res.end(), byte_array_uid2.begin(), byte_array_uid2.end());
                res.insert(res.end(), byte_array_uid1.begin(), byte_array_uid1.end());
                res.insert(res.end(), byte_array_uid3.begin(), byte_array_uid3.end());
            }
            else if (subset == 0x3){
                std::vector<uint8_t> s2_utf8_size = uint64_t2uRTON_t(get_utf8_size(s2)),
                                     s2_size = uint64_t2uRTON_t(s2.size());

                res.insert(res.end(), s2_utf8_size.begin(), s2_utf8_size.end());
                res.insert(res.end(), s2_size.begin(), s2_size.end());
                res.insert(res.end(), s2.begin(), s2.end());
            }
        }
        //nan
        else if (temp == "nan"){
            rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::FLOAT64);

            double dnan = std::numeric_limits<decltype(dnan)>::signaling_NaN();

            std::vector <uint8_t> byte_array = set_raw_data(dnan);

            res.push_back(CHUNK_TYPE::FLOAT64);
            res.insert(res.end(), byte_array.begin(), byte_array.end());
        }
        //inf
        else if (temp == "inf" || temp == "-inf"){
            rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::FLOAT64);

            double dinf = std::numeric_limits<decltype(dinf)>::infinity();
            if (temp[0] == '-') dinf = -dinf;

            std::vector <uint8_t> byte_array = set_raw_data(dinf);

            res.push_back(CHUNK_TYPE::FLOAT64);
            res.insert(res.end(), byte_array.begin(), byte_array.end());
        }
        //utf-8 string
        if(get_utf8_size(temp) < temp.size()){
            if (map_0x93[temp] == 0){
                rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::UTF8);

                std::vector <uint8_t> utf8_size = uint64_t2uRTON_t(get_utf8_size(temp)),
                                      byte_size = uint64_t2uRTON_t(temp.size());

                res.push_back(CHUNK_TYPE::UTF8);
                res.insert(res.end(), utf8_size.begin(), utf8_size.end());
                res.insert(res.end(), byte_size.begin(), byte_size.end());
                res.insert(res.end(), temp.begin(), temp.end());

                rton_info["0x93 array"][to_hex_string(uint64_t2uRTON_t(map_0x93.size() - 1))] = temp;
                map_0x93[temp] = map_0x93.size();
            }
            else{
                rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::UTF8_ARRAY);

                std::vector <uint8_t> utf8_array_pos = uint64_t2uRTON_t(map_0x93[temp] - 1);

                res.push_back(CHUNK_TYPE::UTF8_ARRAY);
                res.insert(res.end(), utf8_array_pos.begin(), utf8_array_pos.end());
            }
        }
        //ascii string
        else{
            if (map_0x91[temp] == 0){
                rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::ASCII);

                std::vector <uint8_t> byte_size = uint64_t2uRTON_t(temp.size());

                res.push_back(CHUNK_TYPE::ASCII);
                res.insert(res.end(), byte_size.begin(), byte_size.end());
                res.insert(res.end(), temp.begin(), temp.end());

                rton_info["0x91 array"][to_hex_string(uint64_t2uRTON_t(map_0x91.size() - 1))] = temp;
                map_0x91[temp] = map_0x91.size();
            }
            else{
                rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::ASCII_ARRAY);

                std::vector <uint8_t> ascii_array_pos = uint64_t2uRTON_t(map_0x91[temp] - 1);

                res.push_back(CHUNK_TYPE::ASCII_ARRAY);
                res.insert(res.end(), ascii_array_pos.begin(), ascii_array_pos.end());
            }
        }
        break;
    }
    case json_fifo::json::value_t::number_integer:{
        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::RTON_t);

        int64_t temp = js.get<decltype(temp)>();
        std::vector <uint8_t> byte_array = uint64_t2uRTON_t(temp < 0 ? -2 * temp - 1 : 2 * temp);

        res.push_back(CHUNK_TYPE::RTON_t);
        res.insert(res.end(), byte_array.begin(), byte_array.end());
        break;
    }
    case json_fifo::json::value_t::number_unsigned:{
        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::uRTON_t);

        uint64_t temp = js.get<decltype(temp)>();
        std::vector <uint8_t> byte_array = uint64_t2uRTON_t(temp);

        res.push_back(CHUNK_TYPE::uRTON_t);
        res.insert(res.end(), byte_array.begin(), byte_array.end());
        break;
    }
    case json_fifo::json::value_t::number_float:{
        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::FLOAT64);

        double temp = js.get<decltype(temp)>();
        std::vector <uint8_t> byte_array = set_raw_data(temp);

        res.push_back(CHUNK_TYPE::FLOAT64);
        res.insert(res.end(), byte_array.begin(), byte_array.end());
        break;
    }
    case json_fifo::json::value_t::object:{
        std::size_t tmp_pos = pos;
        rton_info["List of chunks"][to_hex_string(tmp_pos)] = to_hex_string(CHUNK_TYPE::OBJECT);
        ++tmp_pos;

        std::vector <uint8_t> byte_array = encode_JSON(js, map_0x91, map_0x93, tmp_pos, rton_info);

        res.push_back(CHUNK_TYPE::OBJECT);
        res.insert(res.end(), byte_array.begin(), byte_array.end());
        break;
    }
    case json_fifo::json::value_t::array:{
        std::size_t tmp_pos = pos;

        rton_info["List of chunks"][to_hex_string(tmp_pos)] = to_hex_string(CHUNK_TYPE::ARRAY);
        ++tmp_pos;
        rton_info["List of chunks"][to_hex_string(tmp_pos)] = to_hex_string(CHUNK_TYPE::ARRAY_BEGIN);
        ++tmp_pos;

        std::vector <uint8_t> byte_array_size = uint64_t2uRTON_t(js.size());
        tmp_pos += byte_array_size.size();

        res.push_back(CHUNK_TYPE::ARRAY);
        res.push_back(CHUNK_TYPE::ARRAY_BEGIN);
        res.insert(res.end(), byte_array_size.begin(), byte_array_size.end());
        for (auto i : js){
            std::vector <uint8_t> byte_array = encode_JSON_chunk(i, map_0x91, map_0x93, tmp_pos, rton_info);
            res.insert(res.end(), byte_array.begin(), byte_array.end());
        }

        rton_info["List of chunks"][to_hex_string(tmp_pos)] = to_hex_string(CHUNK_TYPE::ARRAY_END);
        res.push_back(CHUNK_TYPE::ARRAY_END);
        break;
    }
    default:{
        throw std::logic_error(json_error);
    }
    }

    pos += res.size();

    return res;
}

std::vector <uint8_t> encode_JSON(const json_fifo::json &js, std::unordered_map <std::string, uint64_t> &map_0x91, std::unordered_map <std::string, uint64_t> &map_0x93, std::size_t &pos, json_fifo::json &rton_info){
    std::vector <uint8_t> res;
    try{
        for (std::pair <std::string, json_fifo::json> i : js.get<std::map <std::string, json_fifo::json>>()){
            std::vector <uint8_t> tmp = encode_JSON_chunk(i.first, map_0x91, map_0x93, pos, rton_info);
            res.insert(res.end(), tmp.begin(), tmp.end());

            tmp = encode_JSON_chunk(i.second, map_0x91, map_0x93, pos, rton_info);
            res.insert(res.end(), tmp.begin(), tmp.end());
        }
        rton_info["List of chunks"][to_hex_string(pos)] = to_hex_string(CHUNK_TYPE::OBJECT_END);
        ++pos;

        res.push_back(CHUNK_TYPE::OBJECT_END);
    }
    catch(json_fifo::json::exception &e){
        throw std::logic_error(json_error);
    }
    return res;
}

std::vector <uint8_t> json2rton(const json_fifo::json &js, json_fifo::json &rton_info){
    std::size_t pos = 8;
    std::unordered_map <std::string, uint64_t> map_0x91,
                                               map_0x93;
    std::vector <uint8_t> res = {'R', 'T', 'O', 'N',
                                0x1, 0x0, 0x0, 0x0,
                                'D', 'O', 'N', 'E'},
                          rton = encode_JSON(js, map_0x91, map_0x93, pos, rton_info);
    res.insert(std::next(res.begin(), 8), rton.begin(), rton.end());
    return res;
}
