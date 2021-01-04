#include "OpCommand.h"
#include <vector>
#include <stdexcept>
#include <sstream>

namespace jxcode::atomscript
{
    using namespace std;
    using namespace lexer;

    namespace OpCode {
        OpCode_t Unknow = L"Unknow";
        OpCode_t Call = L"call";
        OpCode_t Label = L"label";
        OpCode_t Goto = L"goto";
        OpCode_t If = L"if";
        OpCode_t Set = L"set";
        OpCode_t Del = L"del";
        OpCode_t ClearSubVar = L"clear";
        OpCode_t JumpFile = L"jumpfile";
    }
    OpCommand::OpCommand() : code(OpCode::Unknow), op_token(Token()), targets(vector<Token>()) {

    }
    OpCommand::OpCommand(const OpCode_t& code, const lexer::Token& optoken, const vector<Token>& targets)
        : code(code), op_token(optoken), targets(targets)
    {

    }
    vector<OpCommand> ParseOpList(const vector<Token>& tokens) {
        using namespace jxcode::lexer;

        vector<OpCommand> list;
        bool is_op = true;

        OpCommand cmd = OpCommand();
        for (size_t i = 0; i < tokens.size(); i++) {
            const Token& token = tokens[i];

            if (is_op) {
                if (token.token_type == TokenType::LF) {
                    continue;
                }
                else if (*token.value == OpCode::Call || token.token_type == TokenType::At) {
                    // @
                    cmd.code = OpCode::Call;
                }
                else if (*token.value == OpCode::Goto || token.token_type == TokenType::SingleArrow) {
                    // ->
                    cmd.code = OpCode::Goto;
                }
                else if (*token.value == OpCode::If || token.token_type == TokenType::Question) {
                    // ?
                    cmd.code = OpCode::If;
                }
                else if (*token.value == OpCode::Set || token.token_type == TokenType::Doller) {
                    cmd.code = OpCode::Set;
                    // $
                }
                else if (*token.value == OpCode::ClearSubVar || token.token_type == TokenType::Tilde) {
                    // ~
                    cmd.code = OpCode::ClearSubVar;
                }
                else if (*token.value == OpCode::Del || token.token_type == TokenType::Division) {
                    // -
                    cmd.code = OpCode::Del;
                }
                else if (*token.value == OpCode::JumpFile || token.token_type == TokenType::DoubleArrow) {
                    // =>
                    cmd.code = OpCode::JumpFile;
                }
                else if (*token.value == OpCode::Label || token.token_type == TokenType::DoubleColon) {
                    // ::
                    cmd.code = OpCode::Label;
                }

                cmd.op_token = token;
                is_op = false;
            }
            else {
                if (token.token_type == TokenType::LF) {
                    list.push_back(cmd);
                    cmd = OpCommand();
                    is_op = true;
                }
                else {
                    cmd.targets.push_back(token);
                    //最后一个如果走到这里说明最后没有回车，直接加到列表里
                    if (i == tokens.size() - 1) {
                        list.push_back(cmd);
                    }
                }
            }

        }
        return list;
    }
    std::wstring OpCommand::to_string() const
    {
        wstringstream ss;
        ss.width(8);
        ss << code;
        for (const auto& item : targets) {
            ss.width(18);
            ss << item.value;
        }
        return ss.str();
    }
}