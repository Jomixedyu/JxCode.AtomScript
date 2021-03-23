#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include <malloc.h>

#include "DLL.h"
#include "Interpreter.h"

#ifdef _WIN32 //DLL_MAIN
#include <Windows.h>
#endif

using namespace std;
using namespace jxcode;
using namespace jxcode::atomscript;

struct InterpreterState
{
    atomscript::Interpreter* interpreter;
    wchar_t last_error[1024];
    string serialize_data;

    LoadFileCallBack _loadfile;
    FunctionCallBack _funcall;
    ProgramEndingCallBack _end_;
};

map<int, InterpreterState*> g_inters;
static int g_index = 0;

static void SetErrorMessage(int id, const wchar_t* str);

inline static int kSuccess = 0;
inline static int kNullResult = 1;
inline static int kErrorMsg = 2;


inline static InterpreterState* GetState(int id)
{
    if (g_inters.find(id) == g_inters.end()) {
        return nullptr;
    }
    return g_inters[id];
}
inline static void DelState(int id)
{
    if (GetState(id) != nullptr) {
        g_inters.erase(id);
    }
}
inline static InterpreterState* CheckAndGetState(int id) {
    auto state = GetState(id);
    if (state == nullptr) {
        wchar_t str[] = L"not found interpreter instance";
        SetErrorMessage(id, str);
    }
    return state;
}

inline static Variable CheckAndGetVariable(int id, InterpreterState* inter, const wchar_t* name)
{
    auto var = inter->interpreter->GetVar(name);
    if (var.type == VARIABLETYPE_UNDEFINED) {
        wchar_t str[] = L"not found variable";
        SetErrorMessage(id, str);
    }
    return var;
}


inline static void SetTokenGroup(const vector<Token>& tokens, TokenGroup* group)
{
    for (int i = 0; i < tokens.size(); i++)
    {
        const Token& token_item = tokens[i];
        TokenInfo& item = group->tokens[i];
        
        item.line = token_item.line;
        item.position = token_item.position;
        item.value = token_item.value.get()->c_str();
    }
    group->size = tokens.size();
}
inline static void SetVariableGroup(const vector<Variable>& vars, VariableGroup* group)
{
    for (int i = 0; i < vars.size(); i++)
    {
        group->vars[i] = vars[i];
    }

    group->size = vars.size();
}

static wstring OnLoadFile(int id, const wstring& path)
{
    auto inter = GetState(id);
    return inter->_loadfile(id, path.c_str());
}

static bool OnFuncall(int id,
    const intptr_t& user_type_id,
    const vector<Token>& domain,
    const vector<Token>& path,
    const vector<Variable>& params)
{
    auto inter = GetState(id);

    int userid = user_type_id;

    TokenGroup _domain;
    TokenInfo _domain_token_info[8];
    _domain.tokens = _domain_token_info;
    SetTokenGroup(domain, &_domain);

    TokenGroup _path;
    TokenInfo _path_token_info[8];
    _path.tokens = _path_token_info;
    SetTokenGroup(path, &_path);
    
    VariableGroup _var;
    Variable _var_infos[16];
    _var.vars = _var_infos;
    SetVariableGroup(params, &_var);

    return inter->_funcall(id, userid, _domain, _path, _var);
}

static void OnEnd(int id, const wstring& name)
{
    auto inter = GetState(id);
    inter->_end_(id, name.c_str());
}

void CALLAPI GetErrorMessage(int id, wchar_t* out_str)
{
    auto inter = GetState(id);
    wcscpy(out_str, inter->last_error);
}
static void SetErrorMessage(int id, const wchar_t* str)
{
    auto inter = GetState(id);
    memcpy(inter->last_error, str, 1024);
    inter->last_error[1023] = L'\0';
}

int CALLAPI NewInterpreter(int* out_id)
{
    auto id = ++g_index;

    InterpreterState* state = new InterpreterState();
    memset(state, 0, sizeof(state));

    //closure
    state->interpreter = new atomscript::Interpreter(
        [id](const wstring& path)->wstring {
            return OnLoadFile(id, path);
        },
        [id](const int& user_type_id,
            const vector<Token>& domain,
            const vector<Token>& path,
            const vector<Variable>& params)->bool {
                return OnFuncall(id, user_type_id, domain, path, params);
        },
        [id](const wstring& program_name) {
            OnEnd(id, program_name);
        });
    g_inters[g_index] = state;
    *out_id = g_index;
    return kSuccess;
}

int CALLAPI Initialize(int id, LoadFileCallBack _loadfile_, FunctionCallBack _funcall_, ProgramEndingCallBack _end_)
{
    auto inter = GetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }

    inter->_loadfile = _loadfile_;
    inter->_funcall = _funcall_;
    inter->_end_ = _end_;

    return kSuccess;
}

int CALLAPI ResetState(int id)
{
    auto state = GetState(id);
    if (state == nullptr) {
        return kNullResult;
    }
    state->interpreter->ResetState();
    return kSuccess;
}

int CALLAPI ResetMemory(int id)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    inter->interpreter->ResetMemory();
    return kSuccess;
}

void CALLAPI Terminate(int id)
{
    auto state = GetState(id);
    if (state == nullptr) {
        return;
    }
    delete state->interpreter;
    delete state;
    DelState(id);
}

int CALLAPI ExecuteProgram(int id, const wchar_t* file)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->ExecuteProgram(file);
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI Next(int id)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->Next();
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    catch (const exception& e) {

        SetErrorMessage(id, L"error");
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI GetProgramName(int id, wchar_t* out_name)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        auto name = inter->interpreter->program_name();
        wcscpy(out_name, name.c_str());
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI GetVariable(int id, wchar_t* varname, Variable* out_var)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        auto var = inter->interpreter->GetVar(varname);
        *out_var = var;
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI SetVariable(int id, wchar_t* varname, Variable var)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->SetVar(varname, var);
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI SetStringVariable(int id, wchar_t* varname, wchar_t* str)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->SetVar(varname, wstring(str));
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI NewString(int id, wchar_t* str, int* out_ptr)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        *out_ptr = inter->interpreter->NewStrPtr(str);
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI GetString(int id, int str_ptr, wchar_t* out_str)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        wstring* str = inter->interpreter->GetString(str_ptr);
        wcscpy(out_str, str->c_str());
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI GetStringLength(int id, int str_ptr, int* out_length)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        *out_length = inter->interpreter->GetString(str_ptr)->size() + 1;
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI DelVariable(int id, const wchar_t* varname)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    inter->interpreter->DelVar(varname);
    return kSuccess;
}

int CALLAPI SerializeState(int id, int* out_length)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->serialize_data = inter->interpreter->Serialize();
        *out_length = inter->serialize_data.size();
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());

        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI TakeSerializationData(int id, char* ser_buf)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        if (inter->serialize_data.empty()) {
            SetErrorMessage(id, L"serialize data is empty");
            return kErrorMsg;
        }
        memcpy(ser_buf, inter->serialize_data.c_str(), inter->serialize_data.size());
        inter->serialize_data.clear();
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI DeserializeState(int id, char* deser_buf, int buf_size)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->Deserialize(string(deser_buf, buf_size));
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI StatisticVariable(int id, int* number, int* strptr, int* userptr, int* strpool_count)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        auto vars = inter->interpreter->variables();

        int _number = 0;
        int _strptr = 0;
        int _userptr = 0;

        for (auto& item : vars) {
            if (item.second.type == VARIABLETYPE_NUMBER) {
                ++_number;
            }
            else if (item.second.type == VARIABLETYPE_STRPTR) {
                ++_strptr;
            }
            else if (item.second.type == VARIABLETYPE_USERPTR) {
                ++_userptr;
            }
        }

        *number = _number;
        *strptr= _strptr;
        *userptr = _userptr;

        *strpool_count = inter->interpreter->strpool().size();
    }
    catch (wexceptionbase& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}



void CALLAPI GetLibVersion(wchar_t* out)
{
    wcscpy(out, L"JxCode.Lang.AtomScript 1.2");
}

#ifdef _WIN32

BOOL APIENTRY DllMain(
    HANDLE hModule,             // DLL模块的句柄
    DWORD ul_reason_for_call,   // 调用本函数的原因
    LPVOID lpReserved           // 保留
) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: //进程正在加载本DLL
            break;
        case DLL_THREAD_ATTACH://一个线程被创建
            break;
        case DLL_THREAD_DETACH://一个线程正常退出
            break;
        case DLL_PROCESS_DETACH://进程正在卸载本DLL
            break;
    }
    return TRUE;            //返回TRUE,表示成功执行本函数
}

#endif