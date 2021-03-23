#pragma once
#include <vector>
#include <string>
#include "Token.h"

namespace jxcode::atomscript
{
    using OpCode_t = const wchar_t*;
    namespace OpCode {
        extern OpCode_t Unknow;
        extern OpCode_t Call;
        extern OpCode_t Jump;
        extern OpCode_t Label;
        extern OpCode_t Goto;
        extern OpCode_t If;
        extern OpCode_t Set;
        extern OpCode_t Del;
        extern OpCode_t ClearSubVar;
        extern OpCode_t JumpFile;
    }

    struct OpCommand 
    {
        OpCode_t code;
        std::shared_ptr<lexer::Token> op_token;
        std::vector<std::shared_ptr<lexer::Token>> targets;

        OpCommand();
        OpCommand(
            const OpCode_t& code,
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