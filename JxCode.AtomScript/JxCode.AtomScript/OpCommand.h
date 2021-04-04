#pragma once
#include <vector>
#include <string>
#include "Token.h"

namespace jxcode::atomscript
{
    enum class OpCode
    {
        Unknow,
        Call,
        Label,
        Goto,
        If,
        Set,
        Del,
        ClearSub,
        ToProg,
    };
    struct OpCommand 
    {
        OpCode code;
        std::shared_ptr<lexer::Token> op_token;
        std::vector<std::shared_ptr<lexer::Token>> targets;

        OpCommand();
        OpCommand(
            const OpCode& code,
            const std::shared_ptr<lexer::Token>& optoken, 
            const std::vector<std::shared_ptr<lexer::Token>>& targets);

        std::wstring to_string() const;
    };

    class CommandParserException : public lexer::TokenException
    {
    protected:
        std::wstring get_name() override;
    public:
        CommandParserException(const std::shared_ptr<lexer::Token>& token, const std::wstring& message);
    };

    std::shared_ptr<std::vector<OpCommand>> ParseOpList(
        std::wstring* program_name, 
        std::vector<std::shared_ptr<lexer::Token>>* _tokens);

}