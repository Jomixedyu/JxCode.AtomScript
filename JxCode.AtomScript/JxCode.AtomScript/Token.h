#pragma once
#include <string>
#include <memory>

namespace jxcode::lexer
{
    using TokenType_t = const wchar_t*;
    namespace TokenType
    {
        extern TokenType_t Unknow;
        extern TokenType_t LF;
        extern TokenType_t Note;
        extern TokenType_t String;
        extern TokenType_t Number;
        extern TokenType_t Ident;
        extern TokenType_t True;
        extern TokenType_t False;
        extern TokenType_t Define;
        extern TokenType_t Equal;
        extern TokenType_t DoubleEqual;
        extern TokenType_t Plus;
        extern TokenType_t Minus;
        extern TokenType_t Multiple;
        extern TokenType_t Division;
        extern TokenType_t ExclamatoryAndEqual;
        extern TokenType_t And;
        extern TokenType_t Or;
        extern TokenType_t LBracket;
        extern TokenType_t RBracket;
        extern TokenType_t Colon;
        extern TokenType_t DoubleColon;
        extern TokenType_t Comma;
        extern TokenType_t Dot;
        extern TokenType_t GreaterThan;
        extern TokenType_t LessThan;
        extern TokenType_t SingleArrow;
        extern TokenType_t DoubleArrow;
        extern TokenType_t Tilde;
        extern TokenType_t Exclamatory;
        extern TokenType_t At;
        extern TokenType_t Pound;
        extern TokenType_t Doller;
        extern TokenType_t Precent;
        extern TokenType_t Question;
    }

    struct Token
    {
        TokenType_t token_type;
        std::shared_ptr<std::wstring> value;
        size_t line;
        size_t position;
    public:
        std::wstring to_string() const;
    };
}