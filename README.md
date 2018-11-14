# RETON: Reverse Engineering RTON (RE RTON -> RETON)
## Basic
* RTON file is a serialize type that very similar to JSON but it use byte code.

* RTON file begin with `52 54 4F 4E` (a.k.a RTON in ASCII) follow by 4-byte little-endian indicate version of RTON (usually `01 00 00 00`) and end with `44 4F 4E 45` (a.k.a DONE in ASCII).

## Substitute (`90` and `91`)
* The `90 xx` create a new string follow by `xx` bytes of data.

* By using `90` command the data will push in a stack then it can be recalled by `91 xx` to call the `xx`th in the stack (`xx` starting from 0). Let's call the stack is PREV.

* Example: the following dump contain 2 objects:
    ```
    52 54 4F 4E 01 00 00 00
        90 08 23 63 6F 6D 6D 65 6E 74 90 33 50 6C 61 6E 74 20 6C 65 76 65 6C 69 6E 67 20 64 61 74 61 21 20 20 42 65 77 61 72 65 20 79 65 20 61 6C 6C 20 77 68 6F 20 65 6E 74 65 72 20 68 65 72 65 21
        91 00 90 07 54 65 73 74 69 6E 67
    FF
    44 4F 4E 45
    ```
    * The 1st object it create a 8 bytes string `23 63 6F 6D 6D 65 6E 74` (`#comment` in ASCII) (1st in PREV stack), string inside it is 51 bytes long (`0x33`) `Plant leveling data!  Beware ye all who enter here!` (2nd in PREV stack).

    * The 2nd object use `91 00` which mean it recall the 1st string in PREV stack, data inside it is 7 bytes `Testing`.

    * JSON decode:
    ```JSON
    {
        "#comment": "Plant leveling data!  Beware ye all who enter here!",
        "#comment": "Testing"
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

## Number (`24` and `20`)
### `24`
* If the 1st-byte is 00-7F in hex (0-127 decimal) then it is just 1-byte and used as is. 

* If the 1st-byte has 7th bit (80-FF in hex, 128-255 decimal) the RTON parser reads the second byte and do different things.
    * If the second byte is 00 the first byte gets ANDed with 7F, which means the 7th bit gets removed.

    * If the second byte is 01 then the first byte is used as is (with 7th bit intact).

    * If the second byte is 02 then first byte gets ANDed with 7F and ORed with 0x100 (7th bit removed and 8th bit set).

    * If the second byte is 03 then first byte gets ORed with 100 (8th bit set).

    * This pattern repeats until 4th-byte. If 2nd-byte above `FF` apply this rule to generate the new 2nd-byte and 3rd-byte.

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 09 53 6F 6D 65 56 61 6C 75 65 24 3D
        91 00 24 FE 01
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "SomeValue": 61,
        "SomeValue": 254
    }
    ```

* Pseudo code:
    ```cpp
    //this implementation still has bug when convert 0xbe87f407 to 0xfd03be (correct would be 0x7e81df3b ~ 2 billions), kinda strange even I can't find why it would be 0x7e81df3b???
    int RTONnum2int(int q){
        if (log256(q) < 1 && q > 0x7f) return 0xffffffff; //return max when RTON number has 1 byte and > 0x7f
        int lastByte = q % 0x100;
        q /= 0x100;
        while(q > 0){
            int nearLastByte = q % 0x100;
            q /= 0x100;
            if (lastByte % 2 == 0) nearLastByte &= 0x7f;
            nearLastByte += 0x100 * floor((float) lastByte / 2);
            lastByte = nearLastByte;
        }
        return lastByte;
    }

    int int2RTONnum(int q){
        if (q <= 0x7f) return q;
        int firstByte = q % 0x100;
        q /= 0x100;
        int secondByte = q * 2;
        if (firstByte > 0x7f) ++secondByte;
        else firstByte += 0x80; //reverse & 0x7f
        int newSecondByte = int2RTONnum(secondByte);
        return firstByte * pow(0x100, ceil(log256(newSecondByte)) ? ceil(log256(newSecondByte)) : 1) + newSecondByte;
    }
    ```

### `20`
* This is much more simpler than the `24` just like normal 4-byte little-endian.

* Example:
    ```
    52 54 4F 4E 01 00 00 00
        90 09 53 6F 6D 65 56 61 6C 75 65 20 3D 91 00 24
    FF
    44 4F 4E 45
    ```
    * JSON decode:
    ```JSON
    {
        "SomeValue": 604016957
    }
    ```

## RTID (`83 03`)
* `83 03` begin the RTID (RTON ID???) of RTON (cross-reference???).

* It has 2 strings after the RTID: `RTID(2nd_string@1st_string)`.

* After `83 03` is 2 strings format: `xx yy [string]` where `xx` = `yy` = length of string.

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

## Array (`86 FD`, `FE`)
* Array begin with `86 FD xx` and end with `FE`, where `xx` is the number of elements.

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

## P/s:
If there is anything wrong please contact `Noob n useless guy#2215` on Discord