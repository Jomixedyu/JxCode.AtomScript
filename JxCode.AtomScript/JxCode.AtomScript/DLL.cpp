#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include <malloc.h>

#include "DLL.h"
#include "Interpreter.h"


using namespace std;
using namespace jxcode;
using namespace jxcode::atomscript;

struct InterpreterState
{
    atomscript::Interpreter* interpreter;
    LoadFileCallBack loadfile;
    FunctionCallBack funcall;
    ErrorInfoCallBack errorcb;
    wchar_t last_error[1024];
};

map<int, InterpreterState*> g_inters;
static int g_index = 0;

void SetErrorMessage(int id, const wchar_t* str);

int kSuccess = 0;
int kNullResult = 1;
int kErrorMsg = 2;

InterpreterState* GetState(int id)
{
    if (g_inters.find(id) == g_inters.end()) {
        return nullptr;
    }
    return g_inters[id];
}
void DelState(int id)
{
    if (GetState(id) != nullptr) {
        g_inters.erase(id);
    }
}
InterpreterState* CheckAndGetState(int id) {
    auto state = GetState(id);
    if (state == nullptr) {
        wchar_t str[] = L"not found interpreter instance";
        SetErrorMessage(id, str);
    }
    return state;
}

wstring OnLoadFile(int id, const wstring& path)
{
    auto inter = GetState(id);
    return inter->loadfile(path.c_str());
}

TokenGroup GetTokenGroup(const vector<Token>& tokens) {
    TokenGroup group;
    memset(&group, 0, sizeof(TokenGroup));
    for (group.size = 0; group.size <= tokens.size() - 1; group.size++)
    {
        TokenInfo info;
        info.line = tokens[group.size].line;
        info.position = tokens[group.size].position;
        info.value = const_cast<wchar_t*>(tokens[group.size].value->c_str());
        group.tokens[group.size] = info;
    }
    return group;
}

bool OnFuncall(int id,
    const intptr_t& user_type_id,
    const vector<Token>& domain,
    const vector<Token>& path,
    const vector<Token>& params)
{
    auto inter = GetState(id);

    intptr_t userid = user_type_id;

    TokenGroup _domain = GetTokenGroup(domain);

    TokenGroup _path = GetTokenGroup(path);

    TokenGroup _param = GetTokenGroup(params);

    return inter->funcall(userid, _domain, _path, _param);
}
void OnError(int id, const wstring& str)
{
    auto inter = GetState(id);
    if (inter->errorcb == nullptr) {
        return;
    }
    inter->errorcb(str.c_str());
}


void GetErrorMessage(int id, wchar_t* out_str)
{
    auto inter = GetState(id);
    wcscpy(out_str, inter->last_error);
}
void SetErrorMessage(int id, const wchar_t* str)
{
    auto inter = GetState(id);
    memcpy(inter->last_error, str, 1024);
    inter->last_error[1023] = L'\0';
}

int Initialize(LoadFileCallBack _loadfile_, FunctionCallBack _funcall_, ErrorInfoCallBack _errorcb_, int* out_id)
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
            const vector<Token>& params)->bool {
                return OnFuncall(id, user_type_id, domain, path, params);
        },
        [id](const wstring& str) {
            OnError(id, str);
        });

    state->loadfile = _loadfile_;
    state->funcall = _funcall_;
    state->errorcb = _errorcb_;

    g_inters[g_index] = state;
    *out_id = g_index;

    return kSuccess;
}

int ResetState(int id)
{
    auto state = GetState(id);
    if (state == nullptr) {
        return kNullResult;
    }
    state->interpreter->Reset();
    return kSuccess;
}

void Terminate(int id)
{
    auto state = GetState(id);
    if (state == nullptr) {
        return;
    }
    delete state->interpreter;
    delete state;
    DelState(id);
}

int ExecuteCode(int id, const wchar_t* code)
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

int Next(int id)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->Next();
    }
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, const_cast<wchar_t*>(e.what().c_str()));
        return kErrorMsg;
    }
    return kSuccess;
}

int SerializeState(int id, wchar_t* out_ser_str)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        wstring ser_str = inter->interpreter->Serialize();
        wcscpy(out_ser_str, const_cast<wchar_t*>(ser_str.c_str()));
    }
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());

        return kErrorMsg;
    }
    return kSuccess;
}

int DeserializeState(int id, wchar_t* deser_str)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    try {
        inter->interpreter->Deserialize(deser_str);
    }
    catch (atomscript::InterpreterException& e) {
        SetErrorMessage(id, e.what().c_str());
        return kErrorMsg;
    }
    return kSuccess;
}

int GetStateStatus(int id, int* exeptr, int* var_counts)
{
    auto inter = CheckAndGetState(id);
    if (inter == nullptr) {
        return kNullResult;
    }
    *exeptr = inter->interpreter->line_num();
    *var_counts = (int)inter->interpreter->variables().size();
    return kSuccess;
}

int ReleaseSerializeStr(wchar_t* ptr)
{
    delete[] ptr;
    return kSuccess;
}

void GetLibVersion(wchar_t* out)
{
    wcscpy(out, L"JxCode.Lang.AtomScript 1.0");
}

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

