# RTON Format
## Basic
* RTON file is a serialize type that very similar to JSON but it use byte code.
* RTON file begin with `52 54 4F 4E` (a.k.a RTON in ASCII) follow by 4-byte little-endian indicate version of RTON (usually `01 00 00 00`) and end with `44 4F 4E 45` (a.k.a DONE in ASCII).

## Substitute (`90` and `91`)
* The `90 xx` create a new string follow by `xx` bytes of data.
* By using `90` command the data will push in a stack then it can be recalled by `91 xx` to call the `xx`th in the stack (`xx` starting from 0). Let's call the stack is PREV.
* Example: the following dump contain 2 blocks:
    ```
    52 54 4F 4E 01 00 00 00
    90 08 23 63 6F 6D 6D 65 6E 74 90 33 50 6C 61 6E 74 20 6C 65 76 65 6C 69 6E 67 20 64 61 74 61 21 20 20 42 65 77 61 72 65 20 79 65 20 61 6C 6C 20 77 68 6F 20 65 6E 74 65 72 20 68 65 72 65 21
    91 00 90 07 54 65 73 74 69 6E 67
    FF
    44 4F 4E 45
    ```
    * The first block it create a 8 bytes `23 63 6F 6D 6D 65 6E 74` (`#comment` in ASCII) (1st in PREV stack), data inside it is 51 bytes long (`0x33`) `Plant leveling data!  Beware ye all who enter here!` (2nd in PREV stack).
    * The second block use `91 00` which mean it recall the 1st data in PREV stack, data inside it is 7 bytes `Testing`.
    * JSON decode:
    ```JSON
    {
        "#comment": "Plant leveling data!  Beware ye all who enter here!",
        "#comment": "Testing"
    }
    ```

## Number (`24` and `20`)

## String (`83 03`)

## Array (`86 FD`, `FE`)

## Sub-object (`85`)

## End of Object (`FF`)

## null (`84`)
