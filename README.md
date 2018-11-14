# RTON Format
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
* Mark end of an object

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

### `20`
* This is much more simpler than the `24` just like normal 4-byte little-endian

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

## String (`83 03`)

## Array (`86 FD`, `FE`)

## Sub-object (`85`)
* Create a sub-object as value

* Example:
    ```
    52 54 4F 4E 01 00 00 00 
    90 07 54 65 73 74 69 6E 67 85
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
