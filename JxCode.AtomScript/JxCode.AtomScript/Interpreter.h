#pragma once
#include <string>
#include <vector>
#include <map>
#include <cinttypes>
#include <memory>
#include <functional>
#include <stack>
#include "Token.h"
#include "OpCommand.h"
#include "Variable.h"

namespace jxcode::atomscript
{
    using std::string;
    using std::wstring;
    using std::map;
    using std::vector;
    using std::shared_ptr;
    using std::function;
    using lexer::Token;
    using lexer::TokenException;

    class InterpreterException : public TokenException
    {
    protected:
        virtual std::wstring get_name() override;
    public:
        InterpreterException(const std::shared_ptr<Token>& token, const std::wstring& message);
    public:
        virtual std::wstring what() override;
    };

    class Interpreter
    {
    public:
        using LoadFileCallBack = function<wstring(const wstring& program_name_)>;
        //如果调用对象为静态对象（无法在变量表中找到）则user_type_ptr为0
        using FuncallCallBack = function<bool(
            const int32_t& user_ptr, 
            const vector<Token>& domain,
            const vector<Token>& path,
            const vector<Variable>& params)>;
        using EndCallBack = function<void(const wstring& program_name)>;
    protected:
        LoadFileCallBack _loadfile_;
        FuncallCallBack _funcall_;
        EndCallBack _end_;

        bool OnFunCall(const int32_t& user_ptr,
            const vector<Token>& domain,
            const vector<Token>& path,
            const vector<Variable>& params);

        wstring program_name_; //ser
        bool is_end_; // 当前脚本运行是否结束

        map<wstring, shared_ptr<vector<OpCommand>>> commands_cache_; // TODO 功能搁置
        shared_ptr<vector<OpCommand>> commands_;

        int32_t exec_ptr_; //ser
        map<wstring, size_t> labels_;

        map<wstring, Variable> variables_; //ser
        map<int32_t, wstring> strpool_; //ser
        int32_t ptr_alloc_index_; //ser
    public:
        int32_t line_num() const;
        size_t opcmd_count() const;
        const wstring& program_name() const;
        const map<wstring, Variable>& variables();
        const map<int32_t, wstring>& strpool();
    public:
        Interpreter(
            LoadFileCallBack _loadfile_,
            FuncallCallBack _funcall_,
            EndCallBack _end_);
    protected:
        bool ExecuteLine(const OpCommand& cmd);
        Variable GenTempVar(const std::shared_ptr<Token>& token);
        Variable GenTempVar(const float& num);
        Variable GenTempVar(const wstring& str);
    public:
        bool IsExistLabel(const wstring& label);
        void SetVar(const wstring& name, const float& num);
        void SetVar(const wstring& name, const wstring& str);
        void SetVar(const wstring& name, const int& user_id);
        void SetVar(const wstring& name, const Variable& var);
        void DelVar(const wstring& name);
        Variable GetVar(const wstring& name);
    public:
        int GetStrPtr(const wstring& str);
        int NewStrPtr(const wstring& str);
        wstring* GetString(const int& strptr);
        void GCollect();
        void SetReturnVariable(const Variable& var);
    public:
        Interpreter* ExecuteProgram(const wstring& program_name);
        //返回是否运行结束
        bool Next();

        void ResetState();
        void ResetMemory();

        string Serialize();
        void Deserialize(const string& data);
    };    
    
    class math_lib {
    public:
        static float add(float x, float y);
        static float sub(float x, float y);
        static float mul(float x, float y);
        static float div(float x, float y);
        static float pow(float x, float y);
        static float sqrt(float x);
        static void Invoke(Interpreter* inter, const wstring& name, std::stack<Variable>* params);
    };
    class strlib_lib {
    public:
        static wstring cat(const wstring& str1, const wstring& str2);
        static int cmp(const wstring& str1, const wstring& str2);
        static void Invoke(Interpreter* inter, const wstring& name, std::stack<Variable>* params);
    };

}


