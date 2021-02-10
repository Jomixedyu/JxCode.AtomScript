#include <vector>
#include <string>
#include <map>

#include <malloc.h>

#include "DLL.h"


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
};

map<int, InterpreterState*> g_inters;
static int g_index = 0;

static void SetErrorMessage(int id, const wchar_t* str);

static int kSuccess = 0;
static int kNullResult = 1;
static int kErrorMsg = 2;


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


static TokenGroup GetTokenGroup(const vector<Token>& tokens) {
    TokenGroup group;
    memset(&group, 0, sizeof(TokenGroup));
    for (group.size = 0; group.size < tokens.size(); group.size++)
    {
        TokenInfo info;
        info.line = tokens[group.size].line;
        info.position = tokens[group.size].position;
        info.value = const_cast<wchar_t*>(tokens[group.size].value->c_str());
        group.tokens[group.size] = info;
    }
    return group;
}
static VariableGroup GetVariableGroup(const vector<Variable>& vars)
{
    VariableGroup group;
    memset(&group, 0, sizeof(VariableGroup));
    for (group.size = 0; group.size < vars.size(); group.size++)
    {
        group.vars[group.size] = vars[group.size];
    }
    return group;
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

    intptr_t userid = user_type_id;
    TokenGroup _domain = GetTokenGroup(domain);
    TokenGroup _path = GetTokenGroup(path);
    
    VariableGroup _param = GetVariableGroup(params);

    return inter->_funcall(id, userid, _domain, _path, _param);
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
        [id](const intptr_t& user_type_id,
            const vector<Token>& domain,
            const vector<Token>& path,
            const vector<Variable>& params)->bool {
                return OnFuncall(id, user_type_id, domain, path, params);
        });
    g_inters[g_index] = state;
    *out_id = g_index;
    return kSuccess;
}

int CALLAPI Initialize(int id, LoadFileCallBack _loadfile_, FunctionCallBack _funcall_)
{
    auto inter = GetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    inter->_loadfile = _loadfile_;
    inter->_funcall = _funcall_;
    return kSuccess;
}

int CALLAPI SetCode(int id, wchar_t* str)
{
    
    return 0;
}

int CALLAPI ResetState(int id)
{
    auto state = GetState(id);
    if (state == nullptr) {
        return kNullResult;
    }
    state->interpreter->Reset();
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

int CALLAPI ExecuteCode(int id, const wchar_t* code)
{
    auto inter = CheckAndGetState(id);

    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->ExecuteCode(code);
    }
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI ExecuteFile(int id, const wchar_t* file)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->ExecuteProgram(file);
    }
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI SetString(int id, wchar_t* str, int* out_ptr)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        *out_ptr = inter->interpreter->NewStrPtr(str);
    }
    catch (atomscript::InterpreterException& e) {
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
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI DelVar(int id, const wchar_t* varname)
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
    catch (atomscript::InterpreterException& e) {
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
        strcpy(ser_buf, inter->serialize_data.c_str());
        inter->serialize_data.clear();
    }
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int CALLAPI DeserializeState(int id, char* deser_buf)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->Deserialize(deser_buf);
    }
    catch (atomscript::InterpreterException& e) {
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