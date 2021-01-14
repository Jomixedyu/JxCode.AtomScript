#ifndef JXCODE_LANG_atomscript_DLL_H
#define JXCODE_LANG_atomscript_DLL_H

#include <stdint.h>
#define CALLAPI __stdcall

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#pragma warning(disable:4996)
#endif // _WIN32

typedef struct
{
    wchar_t* value;
    int line;
    int position;
} TokenInfo;

typedef struct
{
    TokenInfo tokens[8];
    int size;
} TokenGroup;

typedef wchar_t* (*LoadFileCallBack)(int id, const wchar_t* path);
typedef int(*FunctionCallBack)(int id, intptr_t user_type_id, TokenGroup domain, TokenGroup path, TokenGroup params);
typedef void(*ErrorInfoCallBack)(int id, const wchar_t* path);

#ifdef __cplusplus
extern "C" {
#endif
    DLLEXPORT void CALLAPI GetErrorMessage(int id, wchar_t* out_str);
    DLLEXPORT int CALLAPI Initialize(LoadFileCallBack _loadfile_, FunctionCallBack _funcall_);
    DLLEXPORT int CALLAPI NewInterpreter(int* id);
    DLLEXPORT void CALLAPI Terminate(int id);
    DLLEXPORT int CALLAPI ResetState(int id);
    DLLEXPORT int CALLAPI ExecuteCode(int id, const wchar_t* code);
    DLLEXPORT int CALLAPI Next(int id);
    DLLEXPORT int CALLAPI SerializeState(int id, wchar_t* out_ser_str);
    DLLEXPORT int CALLAPI DeserializeState(int id, wchar_t* deser_str);
    DLLEXPORT int CALLAPI GetStateStatus(int id, int* exeptr, int* var_counts);
    DLLEXPORT int CALLAPI ReleaseSerializeStr(wchar_t* ptr);
    DLLEXPORT void CALLAPI GetLibVersion(wchar_t* out);
#ifdef __cplusplus
}
#endif

#endif // !JXCODE_LANG_atomscript_DLL_H
