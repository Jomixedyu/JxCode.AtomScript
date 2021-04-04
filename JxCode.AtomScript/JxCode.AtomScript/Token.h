#pragma once
#include <string>
#include <memory>
#include "wexceptionbase.h"

namespace jxcode::lexer
{
    enum class TokenType
    {
        Unknow,
        LF,
        Note,
        String,
        Number,
        Ident,
        True,
        False,
        Define,
        Equal,
        DoubleEqual,
        Plus,
        Minus,
        Multiple,
        Division,
        ExclamatoryAndEqual,
        And,
        Or,
        LBracket,
        RBracket,
        Colon,
        DoubleColon,
        Comma,
        Dot,
        GreaterThan,
        DoubleGreaterThan,
        TripleGreaterThan,
        GreaterThanEqual,
        LessThan,
        DoubleLessThan,
        TripleLessThan,
        LessThanEqual,
        SingleArrow,
        DoubleArrow,
        Tilde,
        Exclamatory,
        At,
        Pound,
        Doller,
        Precent,
        Question
    };

    class Token
    {
    public:
        TokenType token_type;
        std::shared_ptr<std::wstring> program_name;
        std::shared_ptr<std::wstring> value;
        size_t line;
        size_t position;
    public:
        std::wstring to_string() const;
    };

    class TokenException : public wexceptionbase
    {
    protected:
        std::shared_ptr<Token> token_;
    protected:
        virtual std::wstring get_name() = 0;
    public:
        TokenException(const std::shared_ptr<Token>& token, const std::wstring& message);
    public:
        virtual std::wstring what() override;
    };
}