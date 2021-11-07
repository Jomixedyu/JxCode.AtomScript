#pragma once
#include <string>
#include <memory>
#include <map>
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
    inline std::map<TokenType, std::wstring> tokentype_mapping =
    {
        {TokenType::Unknow, L"Unknow"},
        {TokenType::LF, L"LF"},
        {TokenType::Note, L"Note"},
        {TokenType::String, L"String"},
        {TokenType::Number, L"Number"},
        {TokenType::Ident, L"Ident"},
        {TokenType::True, L"True"},
        {TokenType::False, L"False"},
        {TokenType::Define, L"Define"},
        {TokenType::Equal, L"Equal"},
        {TokenType::DoubleEqual, L"DoubleEqual"},
        {TokenType::Plus, L"Plus"},
        {TokenType::Minus, L"Minus"},
        {TokenType::Multiple, L"Multiple"},
        {TokenType::Division, L"Division"},
        {TokenType::ExclamatoryAndEqual, L"ExclamatoryAndEqual"},
        {TokenType::And, L"And"},
        {TokenType::Or, L"Or"},
        {TokenType::LBracket, L"LBracket"},
        {TokenType::RBracket, L"RBracket"},
        {TokenType::Colon, L"Colon"},
        {TokenType::DoubleColon, L"DoubleColon"},
        {TokenType::Comma, L"Comma"},
        {TokenType::Dot, L"Dot"},
        {TokenType::GreaterThan, L"GreaterThan"},
        {TokenType::DoubleGreaterThan, L"DoubleGreaterThan"},
        {TokenType::TripleGreaterThan, L"TripleGreaterThan"},
        {TokenType::GreaterThanEqual, L"GreaterThanEqual"},
        {TokenType::LessThan, L"LessThan"},
        {TokenType::DoubleLessThan, L"DoubleLessThan"},
        {TokenType::TripleLessThan, L"TripleLessThan"},
        {TokenType::LessThanEqual, L"LessThanEqual"},
        {TokenType::SingleArrow, L"SingleArrow"},
        {TokenType::DoubleArrow, L"DoubleArrow"},
        {TokenType::Tilde, L"Tilde"},
        {TokenType::Exclamatory, L"Exclamatory"},
        {TokenType::At, L"At"},
        {TokenType::Pound, L"Pound"},
        {TokenType::Doller, L"Doller"},
        {TokenType::Precent, L"Precent"},
        {TokenType::Question, L"Question"},
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