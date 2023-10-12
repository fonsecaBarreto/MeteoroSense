#pragma once
#include<string>

class BLE
{
  public:
  static void Init(const char* boardName);
  static void Update();


  static void SetConfigCallback(int(*jsCallback)(const std::string& json));

};


