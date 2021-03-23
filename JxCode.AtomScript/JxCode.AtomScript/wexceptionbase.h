#pragma once
#include <string>

class wexceptionbase
{
protected:
    std::wstring message_;
public:
    wexceptionbase(const std::wstring& message);
public:
    virtual std::wstring what() = 0;
};

