# RETON: Reverse Engineering RTON (RE RTON -> RETON)

## Basic
* RTON file is a serialize type that very similar to JSON but it use bytecode.

* RTON file begin with `52 54 4F 4E` (`RTON` in ASCII) follow by 4-byte little-endian indicate version of RTON (usually `01 00 00 00`) and end with `44 4F 4E 45` (`DONE` in ASCII).

## RTON Cheatsheet
Bytecode | Type | Note
--- | --- | ---
`0x0` | false |
`0x1` | true |
`0x8` | int8_t | int 8 bit
`0x9` | 0 | 0 int int8_t?
`0xa` | uint8_t | unsigned int 8 bit
`0xb` | 0 | 0 in uint8_t?
`0x10` | int16_t | int 16 bit
`0x11` | 0 | 0 in int16_t?
`0x12` | uint16_t | unsigned int 16 bit
`0x13` | 0 | 0 in uint16_t?
`0x20` | int32_t | int 32 bit
`0x21` | 0 | 0 in int32_t?
`0x22` | float | [Single-precision floating-point](https://en.wikipedia.org/wiki/Single-precision_floating-point_format)
`0x23` | 0.0 | 0 in float?
`0x24` | [uRTON_t](#unsigned-rton-number) | unsigned RTON number
`0x25` | [RTON_t](#rton-number) | RTON number
`0x26` | uint32_t | unsigned int 32 bit
`0x27` | 0 | 0 in uint32_t?
`0x28` | [uRTON_t](#unsigned-rton-number) | unsigned RTON number
`0x29`? | [RTON_t](#rton-number) | RTON number
`0x41` | 0.0 | 0 in double
`0x42` | double | [Double-precision floating-point](https://en.wikipedia.org/wiki/Double-precision_floating-point_format)
`0x44` | [uRTON_t](#unsigned-rton-number) | unsigned RTON number
`0x45` | [RTON_t](#rton-number) | RTON number
`0x81` | [String](#string) |
`0x82` | [Utf-8 string](#utf-8-string) |
`0x83` | [RTID](#rtid) | RTON ID
`0x85` | [Object](#object) |
`0x86` | [Array](#array) |
`0x90` | [Cached string](#cached-string) |
`0x91` | [Cached string recall](#cached-string) |
`0x92` | [Cached utf-8 string](#cached-utf-8-string) |
`0x93` | [Cached utf-8 string recall](#cached-utf-8-string) |
`0xfd` | [Begin of array](#array) | 
`0xfe` | [End of array](#array) |
`0xff` | [End of object](#end-of-object) |

## Unsigned RTON Number
### `0x24`, `0x28` and `0x44`
* It read 1 byte at a time and keep reading til it found a byte that SMALLER or EQUAL `0x7f`

* After that it do something like this pseudocode:
    ```cpp
    uint64_t unsigned_RTON_num2int(vector <uint8_t> q){
        if (q.size() == 1){
            if (q[0] > 0x7f) return UINT_MAX; //return max when RTON number has 1 byte and > 0x7f
            else return q[0];
        }
        uint64_t near_last_byte, last_byte = q[q.size() - 1];
        q.pop_back();
        while(q.size() > 0){
            near_last_byte = q[q.size() - 1];
            if (last_byte % 2 == 0) near_last_byte &= 0x7f;
            near_last_byte += last_byte / 2 * 0x100;
            last_byte = near_last_byte;
            q.pop_back();
        }
        return last_byte;
    }
    ```

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 05 56 61 6C 75 65 24 3D
        90 09 53 6F 6D 65 56 61 6C 75 65 24 FE 01
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "Value": 61,
        "SomeValue": 254
    }
    ```

## RTON Number
### `0x25`, `0x29` and `0x45`
* Pseudocode:
    ```cpp
    RTON_number = unsigned_RTON_number;
    if (RTON_number % 2) RTON_number = -(RTON_number + 1);
    RTON_number /= 2;
    ```

## String
### `0x81`
* `81 xx [string]` create a `[string]` that has EXACTLY `xx` **unsigned RTON number** of bytes.

## UTF-8 String
### `0x82`
* `82 [L1] [L2] [string]` where `[L1]` is **unsigned RTON number** characters in utf-8 and `[L2]` is **unsigned RTON number** bytes of `[string]`.

## RTID
### `0x83`
* `0x83` begin the RTID (RTON ID???) of RTON (cross-reference???).

* It has 2 subsets (`0x2` and `0x3`)

#### `0x3` Subset
```
83 03 [L1] [L2] [string] [L3] [L4] [string 2]
```
* Format: **RTID(`[string 2]`@`[string]`)**

* After `0x8303` is 2 strings format: `[L1] [L2] [string]` and `[L3] [L4] [string 2]` same as `0x82`

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 0C 52 54 49 44 20 45 78 61 6D 70 6C 65
        83 03
            09 09 31 73 74 53 74 72 69 6E 67
            09 09 32 6E 64 53 74 72 69 6E 67 
    FF 
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "RTID Example": "RTID(2ndString@1stString)"
    }
    ```

#### `0x2` Subset (may not be correct)
```
83 02 [L1] [L2] [string] [U2] [U1] [4 bytes ID]
```
* Format: **RTID(`[U1]`.`[U2]`.`[4 bytes ID]`@`[string]`)** (this is my assumption, it may not correct)

* `[L1] [L2] [string]` is same as `0x82`

* `[U2]` is the second number in uid

* `[U1]` is the first number in uid

* `[ID]` the third (hex) number in uid

* Example:
    ```
    52 54 4F 4E 01 00 00 00 
        90 09 6D 5F 74 68 69 73 50 74 72 83 02 0C 0C 51 75 65 73 74 73 41 63 74 69 76 65 00 01 7D A7 7B 6D
    FF
    44 4F 4E 45
    ```

    * JSON decode:
    ```json
    {
        "m_thisPtr": "RTID(1.0.6d7ba77d@QuestsActive)"
    }
    ```

## Object
### `0x85`
* Create an object as value

* Example:
    ```
    52 54 4F 4E 01 00 00 00 
        90 07 54 65 73 74 69 6E 67
        85
            90 05 48 65 6C 6C 6F 90 02 48 69
        FF
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "Testing": {
            "Hello": "Hi"
        }
    }
    ```

## Array
### `0x86`, `0xfd` and `0xfe`
* `0x86` is declare an array

* Array begin with `0xfd xx` and end with `0xfe`, where `xx` is **unsigned RTON number** of elements in array.

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 0E 41 6E 45 78 61 6D 70 6C 65 41 72 72 61 79
        86 FD 03
            90 0A 31 73 74 45 6C 65 6D 65 6E 74
            90 0A 32 6E 64 45 6C 65 6D 65 6E 74
            90 0A 33 72 64 45 6C 65 6D 65 6E 74
        FE
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "AnExampleArray": [
            "1stElement",
            "2ndElement",
            "3rdElement"
        ]
    }
    ```

## Cached String
### `0x90` and `0x91`
* `90 xx [string]`, the `xx [string]` is just like `0x81`

* By using `0x90`, the string will push in a stack then it can be recall by `91 xx`, `xx` is **unsigned RTON number**-th element in the stack (starting from 0). Let's call the stack is ASCII_CACHE.

* Example: the following dump contain 2 objects:
    ```
    52 54 4F 4E 01 00 00 00
        90 08 23 63 6F 6D 6D 65 6E 74 90 33 50 6C 61 6E 74 20 6C 65 76 65 6C 69 6E 67 20 64 61 74 61 21 20 20 42 65 77 61 72 65 20 79 65 20 61 6C 6C 20 77 68 6F 20 65 6E 74 65 72 20 68 65 72 65 21
        90 07 54 65 73 74 69 6E 67 91 00
    FF
    44 4F 4E 45
    ```
    * The 1st object it create a 8 bytes string key `23 63 6F 6D 6D 65 6E 74` (`#comment` in ASCII) (1st in ASCII_CACHE stack), value inside it is 51 bytes long string (`0x33`) `Plant leveling data!  Beware ye all who enter here!` (2nd in ASCII_CACHE stack).

    * The 2nd object create 7 bytes string key `54 65 73 74 69 6E 67` (`Testing` in ASCII) , value inside it is `91 00` which mean recall the 1st string in ASCII_CACHE stack.

    * JSON decode:
    ```JSON
    {
        "#comment": "Plant leveling data!  Beware ye all who enter here!",
        "Testing": "#comment"
    }
    ```

## Cached UTF-8 String
### `0x92` and `0x93`
* Very much like the **Cached String**, `0x92` and `0x93` different is use the utf-8 encode

* `92 [L1] [L2] [string]`, the `[L1] [L2] [string]` same as `0x82`.

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 05 48 65 6C 6C 6F 92 0B 0E C4 90 C3 A2 79 20 6C C3 A0 20 75 74 66 38
        90 04 54 65 73 74 92 0A 0E 54 68 E1 BB AD 20 6E 67 68 69 E1 BB 87 6D
        93 01 93 00
    FF 44 4F 4E 45
    ```
    * The 1st object it create a 5 bytes string key `48 65 6C 6C 6F` (`Hello` in ASCII) (1st in ASCII_CACHE stack), value inside it is 11 characters (`0x0B`), 14 bytes long utf-8 string (`0x0E`) `Đây là utf8` (`This is utf8` in Vietnamese) (1st in UTF8_CACHE stack).

    * The 2nd object create 4 bytes string key `54 65 73 74` (`Test` in ASCII) (2nd in ASCII_CACHE stack), value inside it is 10 characters (`0x0A`), 14 bytes long utf-8 string (`0x0E`) `Thử nghiệm` (`Test` in Vietnamese) (2nd in UTF8_CACHE stack).

    * The 3rd object with key using `93 01` recall the 2nd string in UTF8_CACHE `Thử nghiệm` and the value `93 00` recall the 1st string `Đây là utf8`.

    * JSON decode:
    ```JSON
    {
        "Hello": "Đây là utf8",
        "Test": "Thử nghiệm",
        "Thử nghiệm": "Đây là utf8"
    }
    ```

## End of Object
### `0xff`
* Mark end of an object.

* Example:
    ```
    52 54 4F 4E 01 00 00 00
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {}
    ```

## TODO:
* **Find the correct format of 0x8302**

* Support regex input e.g: `rton-json *.rton`

* Check for endianness

* Write a header file act like extension to json.hpp, [something like `json::from_cbor()` and `json::to_cbor()`](https://github.com/nlohmann/json#binary-formats-bson-cbor-messagepack-and-ubjson)

## Credit
* [Niels Lohmann](http://nlohmann.me/) for his awesome [json parser](https://github.com/nlohmann/json) and [fifo_map](https://github.com/nlohmann/fifo_map)

## P/s:
* If there is anything wrong feel free to open an issue on github
