# serialport_dll
С++ DLL code for integrate functional to communicate (read, write, service) with serial port

Checked for:
- Unity 2021.3.36f1 (06.05.2025)

Build instructions:
1. Create a DLL project in your development environment (Visual Studio, etc.)
2. Add the SerialPort.h, SerialPort.cpp, and SerialPort.def files
3. In the project settings, define the SERIALPORT_EXPORTS macro to export functions:
- In Project Properties: "Configuration Properties" → "C/C++" → "Preprocessor
- Add in field "Preprocessor Definitions" string SERIALPORT_EXPORTS;
- Click Apply
4. Build the project

Some usage tips:
1. Set the values ​​in the COMMTIMEOUTS structure correctly for your data transfer rates.

Using example (C++)
```
#include <iostream>
#include "SerialPort.h"

int main() {
    void* port = CreateSerialPort();
    
    if (OpenPort(port, "COM3", CBR_9600)) {
        std::cout << "Port opened successfully\n";
        
        const char* data = "Hello, COM port!";
        int bytesWritten = WriteData(port, data, strlen(data));
        std::cout << "Written " << bytesWritten << " bytes\n";
        
        char buffer[256];
        int bytesRead = ReadData(port, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            std::cout << "Read " << bytesRead << " bytes: " << std::string(buffer, bytesRead) << "\n";
        }
        
        ClosePort(port);
    } else {
        std::cerr << "Failed to open port\n";
    }
    
    DestroySerialPort(port);
    return 0;
}
```
Using example C#
```
using System;
using System.Runtime.InteropServices;

class SerialPortWrapper
{
    [DllImport("SerialPort.dll")]
    public static extern IntPtr CreateSerialPort();
    
    [DllImport("SerialPort.dll")]
    public static extern void DestroySerialPort(IntPtr port);
    
    [DllImport("SerialPort.dll")]
    public static extern int OpenPort(IntPtr port, string portName, int baudRate);
    
    [DllImport("SerialPort.dll")]
    public static extern void ClosePort(IntPtr port);
    
    [DllImport("SerialPort.dll")]
    public static extern int WriteData(IntPtr port, byte[] data, int length);
    
    [DllImport("SerialPort.dll")]
    public static extern int ReadData(IntPtr port, byte[] buffer, int bufferSize);
    
    [DllImport("SerialPort.dll")]
    public static extern int IsPortOpen(IntPtr port);
}

class Program
{
    static void Main()
    {
        IntPtr port = SerialPortWrapper.CreateSerialPort();
        
        if (SerialPortWrapper.OpenPort(port, "COM3", 9600) != 0)
        {
            Console.WriteLine("Port opened successfully");
            
            byte[] data = System.Text.Encoding.ASCII.GetBytes("Hello from C#");
            int bytesWritten = SerialPortWrapper.WriteData(port, data, data.Length);
            Console.WriteLine($"Written {bytesWritten} bytes");
            
            byte[] buffer = new byte[256];
            int bytesRead = SerialPortWrapper.ReadData(port, buffer, buffer.Length);
            if (bytesRead > 0)
            {
                string received = System.Text.Encoding.ASCII.GetString(buffer, 0, bytesRead);
                Console.WriteLine($"Read {bytesRead} bytes: {received}");
            }
            
            SerialPortWrapper.ClosePort(port);
        }
        else
        {
            Console.WriteLine("Failed to open port");
        }
        
        SerialPortWrapper.DestroySerialPort(port);
    }
}
```
