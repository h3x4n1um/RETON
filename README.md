# RETON: Reverse Engineering RTON (RE RTON -> RETON)
## Basic
* RTON file is a serialize type that very similar to JSON but it use bytecode.

* RTON file begin with `52 54 4F 4E` (`RTON` in ASCII) follow by 4-byte little-endian indicate version of RTON (usually `01 00 00 00`) and end with `44 4F 4E 45` (`DONE` in ASCII).

## Boolean (`00` and `01`)
* In RTON, boolean value just a byte: `00` is false, `01` is true

## Number
### Normal number
#### `0a` (uint8_t)
* After `0a` is 1-byte unsigned int number

#### `0b` (0 in int8_t???)
* It's appear to be `0` value in RTON

#### `10` (int16_t)
* After `10` is 2-byte signed int number

#### `11` (0 in int16_t???)
* It's appear to be `0` value in RTON

#### `12` (uint16_t)
* After `12` is 2-byte unsigned int number

#### `20` (int32_t)
* After `20` is 4-byte signed int number

#### `21` (0 in int32_t???)
* It's appear to be `0` value in RTON

#### `22` (float32)
* After `22` is 4-byte float number

#### `23` (0 in float32???)
* It's appear to be `0.0` value in RTON

#### `26` (uint32_t)
* After `26` is 4-byte unsigned int number

#### `27` (0 in uint32_t???)
* It's appear to be `0` value in RTON

#### `41` (0 in float64???)
* It's appear to be `0.0` value in RTON

#### `42` (float64)
* After `42` is 8-byte float number

### RTON Number
#### Unsigned RTON Number (`24`, `28` and `44`)
* It read 1 byte at a time and keep reading til it found a byte that SMALLER or EQUAL `7f`

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

#### Signed RTON Number (`25` and `45`)
* Pseudocode:
    ```cpp
    signed_RTON_number = unsigned_RTON_number;
    if (signed_RTON_number % 2 == 1) signed_RTON_number = (signed_RTON_number + 1) / -2;
    else signed_RTON_number /= 2;
    ```


## String (`81` and `82`)
### `81`
* `81 xx [string]` create a `[string]` that has EXACTLY `xx` **unsigned RTON number** of bytes.

### `82`
* `82 [L1] [L2] [string]` where `[L1]` = `[L2]` = **unsigned RTON number** length of `[string]`.

## RTID (`83 03`)
* `83 03` begin the RTID (RTON ID???) of RTON (cross-reference???).

* It has 2 strings after the RTID: `RTID(2nd_string@1st_string)`.

* After `83 03` is 2 strings format: `[L1] [L2] [string]` where `[L1]` = `[L2]` = **unsigned RTON number** length of `[string]`.

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

## null (`84`)
* Use `null` as a value

* Example:
    ```
    52 54 4F 4E 01 00 00 00 
        90 0A 54 68 69 73 49 73 4E 75 6C 6C 84
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "ThisIsNull": null
    }
    ```

## Sub-object (`85`)
* Create a sub-object as value

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

## Array (`86 FD`, `FE`)
* Array begin with `86 FD xx` and end with `FE`, where `xx` is the number of elements in array.

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

## Substitute (`90` and `91`)
* `90 xx [string]` create a `[string]` that has EXACTLY `xx` **unsigned RTON number** of bytes.

* By using `90`, the string will push in a stack then it can be recalled by `91 xx` to call the `xx` **unsigned RTON number** element in the stack (`xx` starting from 0). Let's call the stack is PREV.

* Example: the following dump contain 2 objects:
    ```
    52 54 4F 4E 01 00 00 00
        90 08 23 63 6F 6D 6D 65 6E 74 90 33 50 6C 61 6E 74 20 6C 65 76 65 6C 69 6E 67 20 64 61 74 61 21 20 20 42 65 77 61 72 65 20 79 65 20 61 6C 6C 20 77 68 6F 20 65 6E 74 65 72 20 68 65 72 65 21
        90 07 54 65 73 74 69 6E 67 91 00
    FF
    44 4F 4E 45
    ```
    * The 1st object it create a 8 bytes string `23 63 6F 6D 6D 65 6E 74` (`#comment` in ASCII) (1st in PREV stack), string inside it is 51 bytes long (`0x33`) `Plant leveling data!  Beware ye all who enter here!` (2nd in PREV stack).

    * The 2nd object create 7 bytes string `54 65 73 74 69 6E 67` (`Testing` in ASCII) , value inside it is `91 00` which mean it recall the 1st string in PREV stack.

    * JSON decode:
    ```JSON
    {
        "#comment": "Plant leveling data!  Beware ye all who enter here!",
        "Testing": "#comment"
    }
    ```

## End of Object (`FF`)
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

## P/s:
If there is anything wrong please contact `Noob n useless guy#2215` on Discord