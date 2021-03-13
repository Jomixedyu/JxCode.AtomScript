#include "Token.h"
#include <sstream>

namespace jxcode::lexer 
{
    namespace TokenType
    {
        TokenType_t Unknow = L"Unknow";
        TokenType_t LF = L"LF";
        TokenType_t Note = L"Note";
        TokenType_t String = L"String";
        TokenType_t Number = L"Number";
        TokenType_t Ident = L"Ident";
        TokenType_t True = L"True";
        TokenType_t False = L"False";
        TokenType_t Define = L"Define";
        TokenType_t Equal = L"Equal";
        TokenType_t DoubleEqual = L"DoubleEqual";
        TokenType_t Plus = L"Plus";
        TokenType_t Minus = L"Minus";
        TokenType_t Multiple = L"Multiple";
        TokenType_t Division = L"Division";
        TokenType_t ExclamatoryAndEqual = L"ExclamatoryAndEqual";
        TokenType_t And = L"And";
        TokenType_t Or = L"Or";
        TokenType_t LBracket = L"LBracket";
        TokenType_t RBracket = L"RBracket";
        TokenType_t Colon = L"Colon";
        TokenType_t DoubleColon = L"DoubleColon";
        TokenType_t Comma = L"Comma";
        TokenType_t Dot = L"Dot";
        TokenType_t GreaterThan = L"GreaterThan";
        TokenType_t LessThan = L"LessThan";
        TokenType_t SingleArrow = L"SingleArrow";
        TokenType_t DoubleArrow = L"DoubleArrow";
        TokenType_t Tilde = L"Tilde";
        TokenType_t Exclamatory = L"Exclamatory";
        TokenType_t At = L"At";
        TokenType_t Pound = L"Pound";
        TokenType_t Doller = L"Doller";
        TokenType_t Precent = L"Precent";
        TokenType_t Question = L"Question";
    }               
    
    std::wstring Token::to_string() const {
        std::wstringstream ss;
        ss << L"TokenType: ";
        ss << this->token_type;
        ss << L", Line: ";
        ss << this->line;
        ss << L", Position: ";
        ss << this->position;
        ss << L", Value: " << *this->value;
        return ss.str();
    }
}