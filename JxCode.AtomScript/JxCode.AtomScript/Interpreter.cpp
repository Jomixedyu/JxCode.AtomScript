#include <stdexcept>
#include "Interpreter.h"
#include "Lexer.h"
#include <regex>
#include <codecvt>
#include <sstream>
#pragma warning(disable:4996)

namespace jxcode::atomscript
{
    using namespace std;
    using namespace lexer;

#pragma region InterpreterException
    InterpreterException::InterpreterException(const lexer::Token& token, const std::wstring& message)
        : token_(token), message_(message) {}

    std::wstring InterpreterException::what() const
    {
        return this->token_.to_string();
    }
#pragma endregion


#pragma region Interpreter

    int32_t Interpreter::line_num() const
    {
        return this->exec_ptr_;
    }
    size_t Interpreter::opcmd_count() const
    {
        return this->commands_.size();
    }
    const wstring& Interpreter::program_name() const
    {
        return this->program_name_;
    }
    const map<wstring, Variable>& Interpreter::variables()
    {
        return this->variables_;
    }
    const map<int32_t, wstring>& Interpreter::strpool()
    {
        return this->strpool_;
    }
    Interpreter::Interpreter(LoadFileCallBack _loadfile_, FuncallCallBack _funcall_)
        : ptr_alloc_index_(0), exec_ptr_(-1), _loadfile_(_loadfile_), _funcall_(_funcall_)
    {
    }

    bool Interpreter::IsExistLabel(const wstring& label)
    {
        return this->labels_.find(label) != this->labels_.end();
    }

    void Interpreter::SetVar(const wstring& name, const float& num)
    {
        Variable var = this->GetVar(name);
        if (var.type == VARIABLETYPE_UNDEFINED) {
            SetVariableNumber(&var, num);
            this->variables_[name] = var;
        }
        else {
            SetVariableNumber(&this->variables_[name], num);
        }
    }

    void Interpreter::SetVar(const wstring& name, const wstring& str)
    {
        Variable var = this->GetVar(name);
        if (var.type == VARIABLETYPE_UNDEFINED) {
            this->variables_[name] = var;
        }
        int id = this->NewStrPtr(str);
        SetVariableStrPtr(&this->variables_[name], id);
    }

    void Interpreter::SetVar(const wstring& name, const int& user_id)
    {
        Variable var = this->GetVar(name);
        if (var.type == VARIABLETYPE_UNDEFINED) {
            this->variables_[name] = var;
        }
        SetVariableUserPtr(&this->variables_[name], user_id);
    }

    void Interpreter::SetVar(const wstring& name, const Variable& _var)
    {
        if (_var.type == VARIABLETYPE_UNDEFINED) {
            return;
        }
        this->variables_[name] = _var;
    }

    void Interpreter::DelVar(const wstring& name)
    {
        auto it = this->variables_.find(name);
        if (it != this->variables_.end()) {
            this->variables_.erase(it);
        }
    }

    Variable Interpreter::GetVar(const wstring& name)
    {
        auto it = this->variables_.find(name);
        if (it == this->variables_.end()) {
            Variable var;
            SetVariableUndefined(&var);
            return var;
        }
        return it->second;
    }

    int Interpreter::GetStrPtr(const wstring& str)
    {
        int id = 0;
        for (auto& item : this->strpool_) {
            if (item.second == str) {
                id = item.first;
            }
        }
        return id;
    }

    int Interpreter::NewStrPtr(const wstring& str)
    {
        int _id = this->GetStrPtr(str);
        if (_id != 0) {
            return _id;
        }

        ++this->ptr_alloc_index_;

        this->strpool_[this->ptr_alloc_index_] = str;
        return this->ptr_alloc_index_;
    }

    wstring* Interpreter::GetString(const int& strptr)
    {
        return &this->strpool_[strptr];
    }
    void Interpreter::GCollect()
    {
        //如果变量中没有持有字符串池里的引用时销毁字符串
        vector<int> delay_remove_list;
        for (auto& item : this->strpool_) {
            bool has_var = false;
            for (auto& var_item : this->variables_) {
                const Variable& var = var_item.second;
                if (var.type == VARIABLETYPE_STRPTR && var.ptr == item.first) {
                    has_var = true;
                    break;
                }
            }
            if (!has_var) {
                delay_remove_list.push_back(item.first);
            }
        }
        for (auto& item : delay_remove_list) {
            this->strpool_.erase(item);
        }
    }

    inline static bool IsLiteralToken(const Token& token) {
        return token.token_type == TokenType::Number || token.token_type == TokenType::String;
    }
    inline static bool IsLiteralOrVarStrToken(Interpreter* inter, const Token& token) {
        if (token.token_type == TokenType::String) {
            return true;
        }
        if (token.token_type == TokenType::Ident) {
            auto var = inter->GetVar(*token.value);
            if (var.type == VARIABLETYPE_STRPTR) {
                return true;
            }
        }
        return false;
    }
    inline static void CheckValidLength(const OpCommand& cmd, int length) {
        auto size = cmd.targets.size();
        if (size != length) {
            throw InterpreterException(cmd.op_token, L"arguments error");
        }
    }
    inline static void CheckValidMinLength(const OpCommand& cmd, int min_length) {
        auto size = cmd.targets.size();
        if (size < min_length) {
            throw InterpreterException(cmd.op_token, L"arguments error");
        }
    }
    inline static void CheckValidIdent(const Token& token) {
        if (token.token_type != TokenType::Ident) {
            throw InterpreterException(token, L"argument not is ident");
        }
    }
    inline static void CheckValidTokenType(const Token& token, const TokenType_t& type) {
        if (token.token_type != type) {
            throw InterpreterException(token, L"token type error");
        }
    }
    inline static void CheckValidStrVarOrStrLiteral(Interpreter* inter, const Token& token) {
        if (!IsLiteralOrVarStrToken(inter, token)) {
            throw InterpreterException(token, L"type error");
        }
    }
    inline static void CheckValidVariable(Interpreter* inter, const Token& token) {
        auto var = inter->GetVar(*token.value);
        if (var.type == VARIABLETYPE_UNDEFINED) {
            throw InterpreterException(token, L"variable undefined");
        }
    }
    inline static void CheckValidVariableOrLiteral(Interpreter* inter, const Token& token) {
        //不是字面值 并且 变量不存在
        if (!IsLiteralToken(token)
            && inter->GetVar(*token.value).type == VARIABLETYPE_UNDEFINED)
        {
            throw InterpreterException(token, L"variable undefined");
        }
    }
    inline static void CheckValidVariableType(const Token& token, const Variable& var, int type) {
        if (var.type != type) {
            throw InterpreterException(token, L"variable type error");
        }
    }


    void Interpreter::ResetState()
    {
        //清除 命令集，命令集执行指针，标签集
        decltype(this->commands_)().swap(this->commands_);
        this->exec_ptr_ = -1;
        decltype(this->labels_)().swap(this->labels_);
        this->program_name_.clear();
    }

    bool Interpreter::ExecuteLine(const OpCommand& cmd)
    {
        if (cmd.code == OpCode::Unknow) {
            throw InterpreterException(cmd.op_token, L"unknow opcode");
        }
        if (cmd.code == OpCode::Label) {
            //ignore;
        }
        else if (cmd.code == OpCode::Goto) {
            wstring* label;
            if (cmd.targets.size() == 2 && *cmd.targets[0].value == L"var") {
                //goto var a
                CheckValidIdent(cmd.targets[0]);
                CheckValidStrVarOrStrLiteral(this, cmd.targets[1]);

                Variable var = this->GetVar(*cmd.targets[1].value);
                label = this->GetString(var.ptr);
            }
            else if (cmd.targets.size() == 1) {
                //goto label
                CheckValidStrVarOrStrLiteral(this, cmd.targets[0]);
                label = cmd.targets[0].value.get();
            }
            else {
                throw InterpreterException(cmd.op_token, L"goto语句错误");
            }
            //Check
            if (this->labels_.find(*label) == this->labels_.end()) {
                throw InterpreterException(cmd.op_token, L"Label不存在");
            }
            //jump
            size_t pos = this->labels_[*label];
            this->exec_ptr_ = pos;
        }
        else if (cmd.code == OpCode::Set) {
            //暂时不用表达式，只使用一个值 暂时就三个
            CheckValidLength(cmd, 3);
            CheckValidIdent(cmd.targets[0]);
            CheckValidTokenType(cmd.targets[1], TokenType::Equal);

            wstring& varname = *cmd.targets[0].value;

            if (cmd.targets[2].token_type == TokenType::Number) {
                this->SetVar(varname, std::stof(*cmd.targets[2].value));
            }
            else if (cmd.targets[2].token_type == TokenType::String)
            {
                this->SetVar(varname, *cmd.targets[2].value);
            }
            else if (cmd.targets[2].token_type == TokenType::Ident) {
                Variable v = this->GetVar(*cmd.targets[2].value);
                if (v.type == VARIABLETYPE_UNDEFINED) {
                    throw InterpreterException(cmd.targets[2], L"variable not found");
                }
                this->SetVar(varname, v);
            }
        }
        else if (cmd.code == OpCode::Del) {
            CheckValidLength(cmd, 1);
            CheckValidIdent(cmd.targets[0]);
            this->DelVar(*cmd.targets[0].value);
        }
        else if (cmd.code == OpCode::JumpFile) {
            wstring* pfilestr;
            CheckValidLength(cmd, 1);
            Token token = cmd.targets[0];
            CheckValidStrVarOrStrLiteral(this, token);

            if (token.token_type == TokenType::Ident) {
                auto var = this->GetVar(*token.value);
                pfilestr = this->GetString(var.ptr);
            }
            else {
                pfilestr = token.value.get();
            }
            this->ExecuteProgram(*pfilestr);
        }
        else if (cmd.code == OpCode::If) {
            //暂时不做表达式，目前只可用==号判断

        }
        else if (cmd.code == OpCode::ClearSubVar) {
            //清理子变量
            //子变量规则，Obj__subvar
            CheckValidLength(cmd, 1);
            CheckValidIdent(cmd.targets[0]);

            auto target = cmd.targets[0].value;

            auto it = this->variables_.begin();
            while (it != this->variables_.end()) {
                wstring name = it->first;
                if (name.length() > target->length() + 2 && name.substr(0, target->length() + 2) == *target + L"__") {
                    this->variables_.erase(it++);
                }
                else {
                    it++;
                }
            }
        }
        else if (cmd.code == OpCode::Call) {
            //
            Variable var = this->GetVar(*cmd.targets[0].value);

            vector<Token> domain;
            vector<Token> path;
            vector<Variable> params;

            int var_userptr = 0;

            int32_t index = 0;

            //instance
            if (var.type != VARIABLETYPE_UNDEFINED) {
                CheckValidVariableType(cmd.targets[0], var, VARIABLETYPE_USERPTR);
                var_userptr = var.ptr;
                index = 1;
            }

            bool is_symbol = false;
            bool is_last_domain = false;
            bool is_last_path = false;

            if (var.type != VARIABLETYPE_UNDEFINED) {
                is_symbol = true; //看下一个符号是什么
                //is_last_path = true; //对象直接获取子对象
            }
            else {
                is_last_domain = true; //静态从顶级域开始查找
            }

            for (; index < cmd.targets.size(); index++) {
                const Token& token = cmd.targets[index];

                if (is_symbol) {
                    if (token.token_type == TokenType::DoubleColon) {
                        //域运算符
                        is_last_domain = true;
                    }
                    else if (token.token_type == TokenType::Dot) {
                        //子对象运算符
                        is_last_path = true;
                    }
                    else if (token.token_type == TokenType::Colon) {
                        //函数运算符，退出
                        index++;
                        break;
                    }
                    else {
                        throw InterpreterException(token, L"parser error");
                    }
                    is_symbol = false;
                }
                else {
                    if (is_last_domain) {
                        domain.push_back(cmd.targets[index]);
                        is_last_domain = false;
                    }
                    if (is_last_path) {
                        path.push_back(cmd.targets[index]);
                        is_last_path = false;
                    }

                    is_symbol = true;
                }
            }

            for (; index < cmd.targets.size(); index++) {
                const Token& token = cmd.targets[index];

                //先直接省略逗号
                if (token.token_type == TokenType::Comma) {
                    continue;
                }

                Variable temp_var;

                CheckValidVariableOrLiteral(this, token);
                if (IsLiteralToken(token)) {
                    if (token.token_type == TokenType::Number) {
                        SetVariableNumber(&temp_var, stof(*token.value));
                    }
                    else if (token.token_type == TokenType::String) {
                        auto strptr = this->NewStrPtr(*token.value);
                        SetVariableStrPtr(&temp_var, strptr);
                    }
                }
                else {
                    temp_var = this->GetVar(*token.value);
                }

                params.push_back(temp_var);

            }

            return this->_funcall_(var_userptr, domain, path, params);
        }

        return true;
    }

    static map<wstring, TokenType_t> get_atom_operator_map() {
        map<wstring, TokenType_t> mp;
        mp[L"=="] = TokenType::DoubleEqual;
        mp[L"="] = TokenType::Equal;
        mp[L"("] = TokenType::LBracket;
        mp[L")"] = TokenType::RBracket;
        mp[L"&&"] = TokenType::And;
        mp[L"||"] = TokenType::Or;
        mp[L"*"] = TokenType::Multiple;
        mp[L"-"] = TokenType::Minus;
        mp[L"+"] = TokenType::Plus;
        mp[L"/"] = TokenType::Division;
        mp[L"!="] = TokenType::ExclamatoryAndEqual;
        mp[L"::"] = TokenType::DoubleColon;
        mp[L":"] = TokenType::Colon;
        mp[L","] = TokenType::Comma;
        mp[L"."] = TokenType::Dot;
        mp[L">"] = TokenType::GreaterThan;
        mp[L"<"] = TokenType::LessThan;

        mp[L"~"] = TokenType::Tilde;
        mp[L"!"] = TokenType::Exclamatory;
        mp[L"@"] = TokenType::At;
        mp[L"#"] = TokenType::Pound;
        mp[L"$"] = TokenType::Doller;
        mp[L"%"] = TokenType::Precent;

        mp[L"->"] = TokenType::SingleArrow;
        mp[L"=>"] = TokenType::DoubleArrow;

        return mp;
    }

    Interpreter* Interpreter::ExecuteProgram(const wstring& program_name_)
    {
        this->ResetState();

        this->program_name_ = program_name_;
        wstring code = this->_loadfile_(program_name_);

        auto tokens = lexer::Scanner(&const_cast<wstring&>(code),
            &get_atom_operator_map(),
            &lexer::get_std_esc_char_map());

        this->commands_ = ParseOpList(tokens);

        //获取所有标签
        for (size_t i = 0; i < this->commands_.size(); i++) {
            const OpCommand& item = this->commands_[i];
            const wstring& token_value = *item.targets[0].value;
            if (item.code == OpCode::Label) {
                this->labels_[token_value] = i;
            }
        }
        return this;
    }

    bool Interpreter::Next()
    {
        //check
        if (this->exec_ptr_ + 1 >= (int32_t)this->opcmd_count()) {
            this->ResetState();
            return false;
        }

        bool is_next = false;
        //execute
        ++this->exec_ptr_;

        while (is_next = this->ExecuteLine(this->commands_[this->exec_ptr_])) {
            ++this->exec_ptr_;
            //每隔128行执行一次GC
            if (this->exec_ptr_ % 128 == 0) {
                this->GCollect();
            }
            //check range
            if (this->exec_ptr_ >= (int32_t)this->opcmd_count()) {
                this->ResetState();
                return false;
            }
            else {
                //监控
                int line = this->commands_[this->exec_ptr_].op_token.line;
            }
        }

        return true;
    }

    void Interpreter::ResetMemory()
    {
        this->ptr_alloc_index_ = 0;
        decltype(this->variables_)().swap(this->variables_);
        decltype(this->strpool_)().swap(this->strpool_);
    }

    inline static void StreamWriteInt32(ostream* stream, int32_t i)
    {
        stream->write((char*)&i, sizeof(int32_t));
    }
    inline static int32_t StreamReadInt32(istream* stream)
    {
        char bytes[sizeof(int32_t)];
        stream->read(bytes, sizeof(int32_t));
        int32_t i = *(int32_t*)&bytes;
        return i;
    }
    inline static void StreamWriteString(ostream* stream, const string& str)
    {
        int32_t size = str.size();
        StreamWriteInt32(stream, size + 1);
        stream->write(str.c_str(), size);
        stream->write("\0", 1);
    }
    inline static string StreamReadString(istream* stream)
    {
        int32_t size = StreamReadInt32(stream);
        char* buf = new char[size];
        stream->read(buf, size);
        string str(buf);
        delete[] buf;
        return str;
    }
    inline static void StreamWriteVariable(ostream* stream, Variable& var)
    {
        char var_ser[sizeof(Variable)];
        SerializeVariable(&var, var_ser);
        stream->write(var_ser, sizeof(Variable));
    }
    inline static Variable StreamReadVariable(istream* stream)
    {
        char var_ser[sizeof(Variable)];
        stream->read(var_ser, sizeof(Variable));
        Variable var = DeserializeVariable(var_ser);
        return var;
    }

    string Interpreter::Serialize()
    {
        this->GCollect();

        std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
        stringstream ss;

        //state
        
        StreamWriteString(&ss, c.to_bytes(this->program_name_));
        StreamWriteInt32(&ss, this->exec_ptr_);
        StreamWriteInt32(&ss, this->ptr_alloc_index_);

        //variables
        StreamWriteInt32(&ss, (int32_t)this->variables_.size());
        for (auto& item : this->variables_) {

            string name = c.to_bytes(item.first);
            StreamWriteString(&ss, name);
            StreamWriteVariable(&ss, item.second);
        }
        //strpool
        StreamWriteInt32(&ss, (int32_t)this->strpool_.size());
        for (auto& item : this->strpool_) {

            StreamWriteInt32(&ss, item.first);

            string encode_str = c.to_bytes(item.second);
            StreamWriteString(&ss, encode_str);
        }

        return ss.str();
    }


    void Interpreter::Deserialize(const string& data)
    {
        this->ResetMemory();

        stringstream ss(data);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> c;

        wstring program_name = c.from_bytes(StreamReadString(&ss));
        this->ExecuteProgram(program_name);
        this->exec_ptr_ = StreamReadInt32(&ss);
        this->ptr_alloc_index_ = StreamReadInt32(&ss);

        //variables
        int32_t _length = StreamReadInt32(&ss);

        for (int32_t i = 0; i < _length; i++)
        {
            wstring name = c.from_bytes(StreamReadString(&ss));
            Variable var = StreamReadVariable(&ss);
            this->SetVar(name, var);
        }

        //strpool
        
        int32_t strpool_len = StreamReadInt32(&ss);
        for (int32_t i = 0; i < strpool_len; i++)
        {
            int32_t str_ptr = StreamReadInt32(&ss);
            wstring str = c.from_bytes(StreamReadString(&ss));
            this->strpool_[str_ptr] = str;
        }
    }

#pragma endregion


}

