#ifndef JXCODE_LANG_atomscript_DLL_H
#define JXCODE_LANG_atomscript_DLL_H

#pragma warning(disable:4996)

#include <stdint.h>
#define CALLAPI __stdcall

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

typedef wchar_t* (*LoadFileCallBack)(const wchar_t* path);
typedef int(*FunctionCallBack)(intptr_t user_type_id, TokenGroup domain, TokenGroup path, TokenGroup params);
typedef void(*ErrorInfoCallBack)(const wchar_t* path);

#ifdef __cplusplus
extern "C" {
#endif
    void CALLAPI GetErrorMessage(int id, wchar_t* out_str);
    int CALLAPI Initialize(LoadFileCallBack _loadfile_, FunctionCallBack _funcall_, ErrorInfoCallBack _errorcb_, int* out_id);
    void CALLAPI Terminate(int id);
    int CALLAPI ExecuteCode(int id, const wchar_t* code);
    int CALLAPI Next(int id);
    int CALLAPI SerializeState(int id, wchar_t* out_ser_str);
    int CALLAPI DeserializeState(int id, wchar_t* deser_str);
    void CALLAPI GetLibVersion(wchar_t* out);
#ifdef __cplusplus
}
#endif

#endif // !JXCODE_LANG_atomscript_DLL_H
