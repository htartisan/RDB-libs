# RDB-libs

These CPP modules provide vaious cross platform logic, to speed development time.

Modules include:
- Buffer classes = including: audio, video, ring buffer, ... 
- File I/O classes - including: media, text, binary, ... 
- A GStreamer wrapper class
- Keyboard input classses
- An Application Logging class
- Network I/O classes
- Application Plug-in (Shared Object/DLL) classes
- String Utility classes
- Thread Manager classes
- XML Data classes

This code is primarilly based on c++17, and has been compiled using: MSVC [2017], GCC [G++], Xcode, QCC [Q++].

It has been desingned to work on the following OSs:
- Windows [32/64]
- Linux
- MacOS
- QNX
- ... 

Code Dependencies:
- DR_wav
- GStreamer
- Poco
- RapidXML
- SpdLog
