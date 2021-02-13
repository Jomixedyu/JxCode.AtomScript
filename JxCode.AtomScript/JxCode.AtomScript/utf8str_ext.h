#pragma once
#include <string>

namespace jxcode_strext 
{
    int at(const std::string& str, char* out_char);
    
    void foreach(const std::string& str, void(*f)(char* c, int length));
    int length(const std::string& str);
    int indexof(const std::string& str);
}