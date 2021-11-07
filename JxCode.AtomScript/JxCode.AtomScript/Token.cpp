#include "Token.h"
#include <sstream>

namespace jxcode::lexer 
{

    std::wstring Token::to_string() const {
        std::wstringstream ss;
        ss << L"token: ";
        ss << tokentype_mapping[this->token_type] << L"(type:" << (int)this->token_type << L")";
        ss << L", line: ";
        ss << this->line;
        ss << L", pos: ";
        ss << this->position;
        ss << L", Value: " << *this->value;
        return ss.str();
    }

    TokenException::TokenException(const std::shared_ptr<Token>& token, const std::wstring& message)
        : token_(token), wexceptionbase(message)
    {
    }

    std::wstring TokenException::what()
    {
        return this->get_name() + this->token_->to_string() + this->message_;
    }

}