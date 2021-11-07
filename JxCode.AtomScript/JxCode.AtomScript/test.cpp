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
    ss << ifs.rdbuf();
    ifs.close();
    return ss.str();
}

struct InterpreterState;
extern map<int, InterpreterState*> g_inters;

int main() {

    using namespace std;
    using namespace jxcode;
    using namespace jxcode::lexer;
    using namespace jxcode::atomscript;

    //wstring dir(L"C:\\Users\\Jayshonyves\\Desktop\\");
    //wstring path(L"C:\\Users\\Jayshonyves\\Desktop\\atom.txt");

    //int id = 0;
    //NewInterpreter(&id);
    //Initialize(
    //    id,
    //    [](int id, const wchar_t* path)->wchar_t* {
    //        wstring dir(L"C:\\Users\\Jayshonyves\\Desktop\\" + wstring(path) + L".txt");
    //        wstring* str = new wstring(readAllText(dir));
    //        wcout << dir << endl;
    //        return const_cast<wchar_t*>(str->c_str());
    //    },
    //    [](int id, int user_type_id, TokenGroup domain, TokenGroup path, VariableGroup params)->int {
    //        wcout << user_type_id << endl;
    //        return false;
    //    },
    //    [](int id, const wchar_t* name)->void {

    //    });

    //ExecuteProgram(id, L"atom");
    //Next(id);

    try
    {
        std::wstring code = readAllText(L"F:\\JxCode.AtomScript\\JxCode.AtomScript\\JxCode.AtomScript\\OpCommand.h");

        Lexer lex;
        lex.is_parse_lf = false;

        auto list = lex.Scanner(&code, &lexer::get_std_operator_map(), &lexer::get_std_esc_char_map());
        for (auto& item : list)
        {
            std::wcout << item->to_string() << std::endl;
        }
    }
    catch (wexceptionbase& e)
    {
        std::wcout << e.what() << std::endl;
    }

}
