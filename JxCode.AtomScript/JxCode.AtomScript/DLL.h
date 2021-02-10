#ifndef JXCODE_LANG_atomscript_DLL_H
#define JXCODE_LANG_atomscript_DLL_H

#include <stdint.h>
#include "Interpreter.h"

#define CALLAPI __stdcall

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#pragma warning(disable:4996)
#endif // _WIN32

typedef struct
{
    const wchar_t* value;
    int line;
    int position;
} TokenInfo;

typedef struct
{
    TokenInfo tokens[8];
    int size;
} TokenGroup;

typedef struct
{
    jxcode::atomscript::Variable vars[8];
    int size;
} VariableGroup;

typedef wchar_t* (*LoadFileCallBack)(int id, const wchar_t* path);
typedef int(*FunctionCallBack)(int id, int user_ptr, TokenGroup domain, TokenGroup path, VariableGroup params);

#ifdef __cplusplus
extern "C" {
#endif
    DLLEXPORT void CALLAPI GetErrorMessage(int id, wchar_t* out_str);
    DLLEXPORT int CALLAPI NewInterpreter(int* id);
    DLLEXPORT int CALLAPI Initialize(int id, LoadFileCallBack _loadfile_, FunctionCallBack _funcall_);
    DLLEXPORT int CALLAPI SetCode(int id, wchar_t* str);
    DLLEXPORT void CALLAPI Terminate(int id);
    DLLEXPORT int CALLAPI ResetState(int id);
    DLLEXPORT int CALLAPI ExecuteCode(int id, const wchar_t* code);
    DLLEXPORT int CALLAPI ExecuteFile(int id, const wchar_t* file);
    DLLEXPORT int CALLAPI Next(int id);

    DLLEXPORT int CALLAPI GetProgramName(int id, wchar_t* out_name);

    DLLEXPORT int CALLAPI GetVariable(int id, wchar_t* varname, jxcode::atomscript::Variable* out_var);
    DLLEXPORT int CALLAPI SetVariable(int id, wchar_t* varname, jxcode::atomscript::Variable var);
    DLLEXPORT int CALLAPI SetStringVariable(int id, wchar_t* varname, wchar_t* str);
    DLLEXPORT int CALLAPI SetString(int id, wchar_t* str, int* out_ptr);
    DLLEXPORT int CALLAPI GetString(int id, int str_ptr, wchar_t* out_str);

    DLLEXPORT int CALLAPI DelVar(int id, const wchar_t* varname);

    DLLEXPORT int CALLAPI SerializeState(int id, int* out_length);
    DLLEXPORT int CALLAPI TakeSerializationData(int id, char* ser_buf);

    DLLEXPORT int CALLAPI DeserializeState(int id, char* deser_buf);

    DLLEXPORT void CALLAPI GetLibVersion(wchar_t* out);
#ifdef __cplusplus
}
#endif

#endif // !JXCODE_LANG_atomscript_DLL_H
