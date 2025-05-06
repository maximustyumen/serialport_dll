#pragma once

#ifdef SERIALPORT_EXPORTS
#define SERIALPORT_API __declspec(dllexport)
#else
#define SERIALPORT_API __declspec(dllimport)
#endif

#include <windows.h>
#include <string>
#include <vector>

class SERIALPORT_API SerialPort {
public:
    SerialPort();
    ~SerialPort();

    bool Open(const std::string& portName, DWORD baudRate = CBR_9600, BYTE dataBits = 8, BYTE stopBits = ONESTOPBIT, BYTE parity = NOPARITY);
    void Close();
    bool IsOpen() const;

    DWORD Write(const char* data, DWORD length);
    DWORD Read(char* buffer, DWORD bufferSize);
    void Flush();

    static std::vector<std::string> GetAvailablePorts();

private:
    HANDLE m_hComPort;
    bool m_isOpen;
};

// C-style function export for compatibility
extern "C" {
    SERIALPORT_API void* CreateSerialPort();
    SERIALPORT_API void DestroySerialPort(void* port);
    SERIALPORT_API int OpenPort(void* port, const char* portName, int baudRate);
    SERIALPORT_API void ClosePort(void* port);
    SERIALPORT_API int WriteData(void* port, const char* data, int length);
    SERIALPORT_API int ReadData(void* port, char* buffer, int bufferSize);
    SERIALPORT_API int IsPortOpen(void* port);

    // Functions for working with a list of ports
    SERIALPORT_API int GetPortsCount();
    SERIALPORT_API bool GetPortName(int index, char* buffer, int bufferSize);
}