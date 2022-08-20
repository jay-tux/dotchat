# Dotchat protocol
*Reference for version 0.1*

## Table of Contents
- [Table of Contents](#table-of-contents)
- [Message Structure](#message-structure)
  - [Data Types](#data-types)
    - [Primitives](#primitives)
    - [Strings](#strings)
    - [Lists](#lists)
    - [Objects](#objects)

## Message Structure
Each message is expected to start with the magic string (two bytes) `.C` (or, in hexadecimal `0x2E 0x43`). After this, 
two bytes indicate the protocol version: major and minor version. For version 0.1 (the current version), this means 
`0x00 0x01`.

After the introductory bytes, the actual message can start. A message consists of two parts:
 1. The command. This is a string of arbitrary length (at most 255 characters). This value is sent in two parts:
    1. A single byte indicating the length of the command. 
    2. A sequence of bytes containing the actual message (as signed ASCII characters).
 2. The arguments. This is an [object](#objects) in the form of key-value pairs.

### Data Types
Dotchat supports many data types to be sent. For almost every value, the value itself is pre-pended with a byte
indicating the type of the value, see below. The only exceptions to this rule are:
 - The global argument. This is always an object.
 - The values in lists. There the value type is sent when "declaring" the list.

#### Primitives
Each primitive type has its own identifying byte and size:

 Type                    | C++ type   | Identifying byte | Size    | Notes (if any)      
-------------------------|------------|------------------|---------|---------------------
 8-bit signed integer    | `int8_t`   | `0x01`           | 1 byte  |
 16-bit signed integer   | `int16_t`  | `0x02`           | 2 bytes | Network-order (MSB) 
 32-bit signed integer   | `int32_t`  | `0x03`           | 4 bytes | Network-order (MSB) 
 8-bit unsigned integer  | `uint8_t`  | `0x11`           | 1 byte  |
 16-bit unsigned integer | `uint16_t` | `0x12`           | 2 bytes | Network-order (MSB) 
 32-bit unsigned integer | `uint32_t` | `0x13`           | 4 bytes | Network-order (MSB) 
 signed character        | `char`     | `0x21`           | 1 byte  |

Sending each type thus involves sending a single byte identifying the type, then sending 1, 2 or 4 bytes (depending on 
the type) which hold the actual value.

For example, sending the number 22022 (as a 16-bit unsigned integer), would result in:
```
0x12 0x56 0x06
---- ---------
 A       B
 
 A: Identifying byte
 B: Value, MSB
```

#### Strings
As mentioned earlier, strings are sent as character arrays. Each character array is pre-pended with the number of 
characters; this means strings can include null-characters (`0x00` or `\0`). This, however, means that the length of a
string is limited to 255 characters. For longer strings, you can use a list of characters (see below).

The identifying byte for a string is `0x22`.

For example, sending the string `Hello` would result in:
```
0x22 0x05 0x48 0x65 0x6C 0x6C 0x6F
---- ---- ------------------------
 A    B              C
 
 A: Identifying byte
 B: String length
 C: Character array
```

#### Lists
Lists are quite similar to strings in representation; except they also carry the data type of the values included. All 
values in the list must share the same data type.

The scheme for a list is:
 1. Identifying byte for a list `0x41` (1B).
 2. Identifying byte for the type of the values in the list (1B).
 3. The size of the list (the amount of elements) as a 32-bit unsigned integer (4B, in MSB).

A few examples:  
 - `[1, 2, 4, 8]` as a sequence of 8-bit unsigned integers:
   ```
   0x41 0x11 0x00 0x00 0x00 0x04 0x01 0x02 0x04 0x08
   ---- ---- ------------------- -------------------
    A    B            C                   D
   
   A: Identifying byte for lists
   B: Identifying byte for 8-bit unsigned integers
   C: The length of the list (4)
   D: The values, one by one
   ```
 - `[1024, 2, 512, 4, 128, 8, 64]` as a sequence of 16-bit signed integers:
   ```
   0x41 0x02 0x00 0x00 0x00 0x07 0x04 0x00 0x00 0x02 0x02 0x00 0x00 0x04 0x00 0x80 0x00 0x08 0x00 0x40
   ---- ---- ------------------- =========-=========-=========-=========-=========-=========-=========
    A    B            C                                               D
   
   A: Identifying byte for lists
   B: Identifying byte for 16-bit signed integers
   C: The length of the list (7)
   D: The values, one by one. Each value takes 2 bytes (as a 16-bit signed integer)
   ```
 - The string `.chat` as a list: `['.', 'c', 'h', 'a', 't']`:
   ```
   0x41 0x21 0x00 0x00 0x00 0x05 0x2E 0x63 0x68 0x61 0x74
   ---- ---- ------------------- ------------------------
    A    B            C                     D
   
   A: Identifying byte for lists
   B: Identifying byte for characters
   C: The length of the list (5)
   D: The values, one by one.
   ```
 - A 3-by-3 matrix: `[[113, 14, 85], [125, 69, 125], [255, 67, 64]]` (each as an 8-bit unsigned integer):
   ```
   0x41 0x41 0x00 0x00 0x00 0x03 0x11 0x00 0x00 0x00 0x03 0x81 0x0E 0x55 0x11 0x00 0x00 0x00 0x03 0x8D 0x45 0x8D 0x11 0x00 0x00 0x00 0x03 0xFF 0x43 0x40 
   ---- ---- ------------------- ---- ------------------- -------------- ---- ------------------- -------------- ---- ------------------- --------------
    A    B            C           D            E                F         D            E                F         D            E                 F
   
   A: Identifying byte for lists
   B: Identifying byte for sub-lists (same again)
   C: Length of the big list (3)
   D: Identifying byte for sub-list content (unsigned 8-bit integer)
   E: Length of the sub-list (3)
   F: Sub-list contents
   ```
 - The types and lengths between sublists don't have to match: `[[1], ['a', 'b', 'c']]`:
   ```
   0x41 0x41 0x00 0x00 0x00 0x02 0x01 0x00 0x00 0x00 0x01 0x01 0x21 0x00 0x00 0x00 0x03 0x61 0x62 0x63
   ---- ---- ------------------- ---- ------------------- ---- ---- ------------------- --------------
    A    B            C           D            E           F    G            H                I
   
   A: Identifying byte for lists
   B: Identifying byte for sub-lists
   C: Length of the big list (2)
   D: Identifying byte for the first sub-list's content (signed 8-bit integer)
   E: Length of the first sub-list (1)
   F: Content of the first sub-list
   G: Identifying byte for the second sub-list's content (character)
   H: Length of the second sub-list (3)
   I: Contents of the second sub-list
   ```

#### Objects
Objects are sent as sets of key-value pairs. Each key is a string, and each value can be of any type you'd like - be it 
primitives, strings, lists or other objects.

The scheme for an object is:
 1. The identifying byte `0x31`,
 2. The amount of keys in the object (1B, so max. 255),
 3. The key-value pairs:
    1. The length of the key (1B, so max. 255),
    2. The characters of the key
    3. The value; this is like any other value (identifying byte and content).

Some examples (the examples are written in JSON because that's a human-readable format):
 - `{ "Name": "John", "Age": 26 }` (where the `26` is an unsigned 8-bit integer):
   ```
   0x31 0x02 0x04 0x4E 0x61 0x6D 0x65 0x22 0x04 0x4A 0x6F 0x68 0x6E 0x03 0x41 0x67 0x65 0x11 0x1A
   ---- ---- ---- ------------------- ---- ---- ------------------- ---- -------------- ---- ----
    A    B    C            D           E    F            G           H          I        J    K
   
   A: Identifying byte for objects
   B: Amount of keys (2)
   C: Length of key #1 (4)
   D: Key #1
   E: Type of value #1 (string)
   F: Length of value #1 (4)
   G: Value #1 contents
   H: Length of key #2 (3)
   I: Key #2
   J: Type of value #2 (unsigned 8-bit integer)
   K: Value #2
   ```
 - An example with sub-objects (all numbers are 8-bit signed integers): 
   ```json
   { 
     "Location": { 
       "Long": 120, 
       "Lat": 16 
     }, 
     "Name": "Place #1", 
     "Details": {} 
   }
   ```
   ```
          0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
        +---------------------------------------------------------------------------------+
   0x00 | 0x31 0x03 0x08 0x4C 0x6F 0x63 0x61 0x74 0x69 0x6F 0x6E 0x31 0x02 0x04 0x4C 0x6F |
   0x10 | 0x6E 0x67 0x01 0x78 0x03 0x4C 0x61 0x74 0x01 0x10 0x04 0x4E 0x61 0x6D 0x65 0x22 | 
   0x20 | 0x08 0x50 0x6C 0x61 0x63 0x65 0x20 0x23 0x31 0x07 0x44 0x65 0x74 0x61 0x69 0x6C |
   0x30 | 0x73 0x31 0x00                                                                  |
        +---------------------------------------------------------------------------------+
   
   Byte  0x00: Identifying byte for objects
   Byte  0x01: Amount of keys in root object (3)
   Bytes 0x02-0x0A: Key #1
     Byte  0x02: Length of key #1 (8)
     Bytes 0x03-0x0A: Key #1 content
   Bytes 0x0B-0x19: Value #1
     Byte  0x0B: Identifying byte for objects (sub-object)
     Byte  0x0C: Amount of keys in sub-object (2)
     Bytes 0x0D-0x11: Key #1 for sub-object
       Byte  0x0D: Length of key #1 (sub-object, 4)
       Bytes 0x0E-0x11: Key #1 content (sub-object)
     Bytes 0x12-0x13: Value #1 (sub-object)
       Byte  0x12: Identifying byte for 8-bit signed integers
       Byte  0x13: Actual value
     Bytes 0x14-0x17: Key #2 for sub-object
       Byte  0x14: Length of key #2 (sub-object, 3)
       Bytes 0x15-0x17: Key #2 content (sub-object)
     Bytes 0x18-0x19: Value #2 (sub-object)
       Byte  0x18: Identifying byte for 8-bit signed integers
       Byte  0x19: Actual value
   Bytes 0x1A-0x1E: Key #2
     Byte  0x1A: Length of key #2 (4)
     Bytes 0x1B-0x1E: Key #2 content
   Bytes 0x1F-0x28: Value #2
     Byte  0x1F: Identifying byte for strings
     Byte  0x20: String length (8)
     Bytes 0x21-0x28: String content
   Bytes 0x29-0x30: Key #3
     Byte  0x29: Length of key #3 (7)
     Bytes 0x2A-0x30: Key #3 content
   Bytes 0x31-0x32: Value #3
     Byte  0x31: Identifying byte for objects (sub-object)
     Byte  0x32: Amount of keys in sub-object (0)
   ```