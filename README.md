# serialport_dll
С++ DLL code for integrate functional to communicate (read, write, service) with serial port

Checked for:
- Unity (06.05.2025)

Build instructions:
- Create a DLL project in your development environment (Visual Studio, etc.)
- Add the SerialPort.h, SerialPort.cpp, and SerialPort.def files
- In the project settings, define the SERIALPORT_EXPORTS macro to export functions:
- -In Project Properties: "Configuration Properties" → "C/C++" → "Preprocessor
- -Add in field "Preprocessor Definitions" string SERIALPORT_EXPORTS;
- -Click Apply
- Build the project
