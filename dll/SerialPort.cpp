#include "SerialPort.h"
#include <stdexcept>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <algorithm>

#pragma comment(lib, "setupapi.lib")

SerialPort::SerialPort() : m_hComPort(INVALID_HANDLE_VALUE), m_isOpen(false) {}

SerialPort::~SerialPort() {
    Close();
}

bool SerialPort::Open(const std::string& portName, DWORD baudRate, BYTE dataBits, BYTE stopBits, BYTE parity) {
    if (m_isOpen) {
        Close();
    }

    std::string fullPortName = "\\\\.\\" + portName; // To support COM ports with numbers > 9

    m_hComPort = CreateFileA(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_hComPort == INVALID_HANDLE_VALUE) {
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(m_hComPort, &dcbSerialParams)) {
        CloseHandle(m_hComPort);
        m_hComPort = INVALID_HANDLE_VALUE;
        return false;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = dataBits;
    dcbSerialParams.StopBits = stopBits;
    dcbSerialParams.Parity = parity;

    if (!SetCommState(m_hComPort, &dcbSerialParams)) {
        CloseHandle(m_hComPort);
        m_hComPort = INVALID_HANDLE_VALUE;
        return false;
    }

    // Setting timeouts - https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 1;           // Max. byte delay (ms)
    timeouts.ReadTotalTimeoutMultiplier = 0;    // Don't add delay to each byte
    timeouts.ReadTotalTimeoutConstant = 15;     // Total timeout for read operation (ms)
    timeouts.WriteTotalTimeoutMultiplier = 0;   // For recording (not critical)
    timeouts.WriteTotalTimeoutConstant = 50;    // Write timeout (ms)
    
    if (!SetCommTimeouts(m_hComPort, &timeouts)) {
        CloseHandle(m_hComPort);
        CloseHandle(m_hComPort);
        m_hComPort = INVALID_HANDLE_VALUE;
        return false;
    }

    m_isOpen = true;
    return true;
}

void SerialPort::Close() {
    if (m_isOpen) {
        CloseHandle(m_hComPort);
        m_hComPort = INVALID_HANDLE_VALUE;
        m_isOpen = false;
    }
}

bool SerialPort::IsOpen() const {
    return m_isOpen;
}

DWORD SerialPort::Write(const char* data, DWORD length) {
    if (!m_isOpen) {
        return 0;
    }

    DWORD bytesWritten;
    if (!WriteFile(m_hComPort, data, length, &bytesWritten, NULL)) {
        return 0;
    }

    return bytesWritten;
}

DWORD SerialPort::Read(char* buffer, DWORD bufferSize) {
    if (!m_isOpen) {
        return 0;
    }

    DWORD bytesRead;
    if (!ReadFile(m_hComPort, buffer, bufferSize, &bytesRead, NULL)) {
        return 0;
    }

    return bytesRead;
}

void SerialPort::Flush() {
    if (m_isOpen) {
        PurgeComm(m_hComPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

std::vector<std::string> SerialPort::GetAvailablePorts() {
    std::vector<std::string> ports;

    // Checking standard COM ports (1-256)
    for (int i = 1; i <= 256; ++i) {
        std::string portName = "COM" + std::to_string(i);
        HANDLE hPort = CreateFileA(
            ("\\\\.\\" + portName).c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hPort != INVALID_HANDLE_VALUE) {
            ports.push_back(portName);
            CloseHandle(hPort);
        }
    }

    /* Additional search via SetupAPI for a more accurate list
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    if (hDevInfo != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {
            DWORD dataType;
            char buffer[256];
            DWORD bufferSize = sizeof(buffer);

            if (SetupDiGetDeviceRegistryPropertyA(
                hDevInfo,
                &devInfoData,
                SPDRP_FRIENDLYNAME,
                &dataType,
                (BYTE*)buffer,
                bufferSize,
                &bufferSize)) {

                std::string friendlyName(buffer);
                size_t comPos = friendlyName.find("(COM");
                if (comPos != std::string::npos) {
                    size_t start = comPos + 1;
                    size_t end = friendlyName.find(")", start);
                    std::string portName = friendlyName.substr(start, end - start);
                    if (std::find(ports.begin(), ports.end(), portName) == ports.end()) {
                        ports.push_back(portName);
                    }
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }*/

    // Sort ports by number
    std::sort(ports.begin(), ports.end(), [](const std::string& a, const std::string& b) {
        int numA = atoi(a.substr(3).c_str());
        int numB = atoi(b.substr(3).c_str());
        return numA < numB;
        });

    return ports;
}

// Implementing C-style function exports
SERIALPORT_API void* CreateSerialPort() {
    return new SerialPort();
}

SERIALPORT_API void DestroySerialPort(void* port) {
    delete static_cast<SerialPort*>(port);
}

SERIALPORT_API int OpenPort(void* port, const char* portName, int baudRate) {
    SerialPort* sp = static_cast<SerialPort*>(port);
    return sp->Open(portName, baudRate) ? 1 : 0;
}

SERIALPORT_API void ClosePort(void* port) {
    SerialPort* sp = static_cast<SerialPort*>(port);
    sp->Close();
}

SERIALPORT_API int WriteData(void* port, const char* data, int length) {
    SerialPort* sp = static_cast<SerialPort*>(port);
    return static_cast<int>(sp->Write(data, static_cast<DWORD>(length)));
}

SERIALPORT_API int ReadData(void* port, char* buffer, int bufferSize) {
    SerialPort* sp = static_cast<SerialPort*>(port);
    return static_cast<int>(sp->Read(buffer, static_cast<DWORD>(bufferSize)));
}

SERIALPORT_API int IsPortOpen(void* port) {
    SerialPort* sp = static_cast<SerialPort*>(port);
    return sp->IsOpen() ? 1 : 0;
}

SERIALPORT_API int GetPortsCount() {
    auto ports = SerialPort::GetAvailablePorts();
    return static_cast<int>(ports.size());
}

SERIALPORT_API bool GetPortName(int index, char* buffer, int bufferSize) {
    auto ports = SerialPort::GetAvailablePorts();
    if (index < 0 || index >= static_cast<int>(ports.size()) || bufferSize <= 0) {
        return false;
    }

    strncpy_s(buffer, bufferSize, ports[index].c_str(), _TRUNCATE);
    return true;
}