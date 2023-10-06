#include "bt-integration.h"

#include "BluetoothSerial.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

BluetoothSerial SerialBT;
bool isBluetoothOn = 0;
typedef int (*CALLBACK)(const std::vector<std::string>& document);
//const char *configFileName = "/config.txt";
CALLBACK functionPtr{nullptr};
bool isParagraph = 0;
bool BluetoothConnection::Init()
{
  if(isBluetoothOn)return 0;
  SerialBT.begin("ESP32test");
  isBluetoothOn = true;
  return 1;
}

bool BluetoothConnection::Shutdown()
{
  if(!isBluetoothOn)return 0;
  SerialBT.end();
  isBluetoothOn=false;
  return 1;
}

const char* tokens[10] =  {"STATION_UID", "STATION_NAME", "WIFI_SSID", "WIFI_PASSWORD", "MQTT_SERVER", "MQTT_USERNAME", "MQTT_PASSWORD", "MQTT_TOPIC", "MQTT_PORT", "INTERVAL"} ;
int l = 10;

void split_in_args(std::vector<std::string>& args, std::string command) {
    int len = command.length();
    bool double_quoted = false, single_quoted = false, backtick_quoted = false;
    int arg_start = 0;
    int t = 0;
    for (int i = 0; i < len; i++) {
        char current_char = command[i];
        
       /* if (current_char == '\"') {
            double_quoted = !double_quoted;
        }
        else if (current_char == '\'') {
            single_quoted = !single_quoted;
        }*/
        if (current_char == '`') {
            backtick_quoted = !backtick_quoted;
            if (!backtick_quoted)t = 1;
        }

        if (!double_quoted && !single_quoted && !backtick_quoted && current_char == ' ') {
            // Found a space outside of quotes and backticks, it's an argument separator
            args.push_back(command.substr(arg_start+t, i - arg_start-t-t));
            t = 0;
            arg_start = i + 1;
        }
    }

    // Handle the last argument
    args.push_back(command.substr(arg_start+t,command.length()- arg_start-t-t));
}

void BluetoothConnection::Handle_BT_Input(const std::string& message) 
{  
    std::cout<<"message length: "<<message.length() <<" "<<message<<"\n";
  
    std::vector<std::string> parts;
 
    split_in_args(parts,message);
    if (parts.empty()) {
        std::cout << "No input provided" << std::endl;
        return;
    }

    for (const std::string& s : parts) {
      std::cout <<s.length()<<": "<< s << "\n";
    }
        // Check if there are any parts in the input
    if(functionPtr)
    functionPtr(parts);
    

}
//void createFile(fs::FS &fs, const char * path, const char * message);

void BluetoothConnection::ReadBluetooth()
{

  static std::string msbf = "";
    if (Serial.available()) {
      
      char c = Serial.read();
      if (c == '\n') // Assuming '\n' is the message terminator
      { 
        const char* myString = msbf.c_str();
        for (int i = 0; msbf[i] != '\0'; i++)
        SerialBT.write(msbf[i]);
        SerialBT.write('\n');
    
        msbf = ""; // Clear the buffer for the next message
      } 
      else 
      {
        msbf += c;
      }
      delay(1);
    }


  char c = ';';
  static std::string messageBuffer = "";
  if (SerialBT.available()) 
  {
    c = SerialBT.read();
    //std::cout<<"Im here: "<<(int)c<<"\n";
    messageBuffer += c;
    delay(1);
  }
  if(c == ';' && messageBuffer.length()>0)
  {
    if (messageBuffer.length() >= 2) messageBuffer = messageBuffer.substr(0, messageBuffer.length() - 2);
    Handle_BT_Input(messageBuffer);
    messageBuffer = "";
  }

}

void BluetoothConnection::SetCallback(void* func)
{
  functionPtr = (CALLBACK)func;
}




