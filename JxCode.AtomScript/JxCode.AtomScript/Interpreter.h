#pragma once
#include <string>
#include <vector>
#include <map>
#include <cinttypes>
#include <memory>
#include <functional>

#include "Token.h"
#include "OpCommand.h"


namespace jxcode::atomscript
{
    using std::wstring;
    using std::map;
    using std::vector;
    using std::shared_ptr;
    using std::function;
    using lexer::Token;

    enum class VariableType : int
    {
        Null,
        Numeric,
        String,
        UserVarId
    };

    class Variable
    {
    public:
        VariableType type;
        float num;
        wstring str;
        int user_type_ptr;
    public:
        Variable();
        Variable(const float& num);
        Variable(const wstring& str);
        Variable(const int& user_type_ptr);
    public:
        void SetNumber(const float& num);
        void SetString(const wstring& str);
        void SetUserVarId(const int& user_type_ptr);
    public:
        wstring GetSerializeData();
        static Variable DeserializeData(const wstring& data);
    };

    class InterpreterException
    {
    public:
        static std::wstring ParserException;
        static std::wstring RuntimeException;
    protected:
        std::wstring message_;
        lexer::Token token_;
    public:

    public:
        InterpreterException(const lexer::Token& token, const std::wstring& message);
    public:
        virtual std::wstring what() const;
    };

    class Interpreter
    {
    public:
        using LoadFileCallBack = function<wstring(const wstring& path)>;
        //如果调用对象为静态对象（无法在变量表中找到）则user_type_ptr为0
        using FuncallCallBack = function<bool(
            const intptr_t& user_type_ptr, 
            const vector<Token>& domain,
            const vector<Token>& path,
            const vector<Token>& params)>;
        using ErrorInfoCallBack = function<void(const wstring& error_info)>;
    protected:
        LoadFileCallBack _loadfile_;
        FuncallCallBack _funcall_;
        ErrorInfoCallBack _errorcall_;

        vector<OpCommand> commands_;
        int32_t exec_ptr_;
        map<wstring, size_t> labels_;

        map<wstring, Variable*> variables_;
    public:
        int32_t line_num() const;
        size_t opcmd_count() const;
        map<wstring, Variable*>& variables();
    public:
        Interpreter(
            LoadFileCallBack _loadfile_,
            FuncallCallBack _funcall_,
            ErrorInfoCallBack _errorcall_);
    protected:
        void ResetCodeState();
        bool ExecuteLine(const OpCommand& cmd);
    public:
        bool IsExistLabel(const wstring& label);
        void SetVar(const wstring& name, const float& num);
        void SetVar(const wstring& name, const wstring& str);
        void SetVar(const wstring& name, const int& user_id);
        void SetVar(const wstring& name, const Variable& var);
        void DelVar(const wstring& name);
        Variable* GetVar(const wstring& name);
    public:
        Interpreter* ExecuteCode(const wstring& code);
        Interpreter* Next();
        
        void Reset();

        wstring Serialize();
        void Deserialize(const wstring& text);
    };
}


