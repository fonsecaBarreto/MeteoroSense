#pragma once
#include <string>


//#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
//#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
//#endif

class BluetoothConnection
{
public:
    static bool Init();
    static bool Shutdown();
    static void Handle_BT_Input(const std::string& message);
    static void ReadBluetooth();
    static void SetCallback(void* func);

};

