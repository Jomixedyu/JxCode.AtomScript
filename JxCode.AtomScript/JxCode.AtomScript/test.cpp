#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "Interpreter.h"
#include "DLL.h"

using namespace std;

static std::wstring readAllText(const std::wstring& path) {
    std::wifstream ifs;
    std::wstringstream ss;
    ifs.open(path);
    if (!ifs.is_open()) {
        throw std::invalid_argument("Unable to open file");
    }
    ss << ifs.rdbuf() << std::endl;
    ifs.close();
    return ss.str();
}

int main() {

    using namespace std;
    using namespace jxcode;
    using namespace jxcode::lexer;
    using namespace jxcode::atomscript;

    wstring dir(L"C:\\Users\\Jayshonyves\\Desktop\\");
    wstring path(L"C:\\Users\\Jayshonyves\\Desktop\\AtomScript.txt");

    int id = 0;
    Initialize(
        [](const wchar_t* path)->wchar_t* {
            wstring dir(L"C:\\Users\\Jayshonyves\\Desktop\\" + wstring(path));
            wstring* str = new wstring(readAllText(dir));
            wcout << dir << endl;
            return const_cast<wchar_t*>(str->c_str());
        },
        [](intptr_t user_type_id, TokenGroup domain, TokenGroup path, TokenGroup params)->int {
            wcout << user_type_id << endl;
            return false;
        },
            nullptr, &id
            );
    ExecuteCode(id, L"jumpfile \"AtomScript.txt\"");
    Next(id);

}