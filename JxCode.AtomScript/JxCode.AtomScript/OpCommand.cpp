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
        OpCode_t Call = L"Call";
        OpCode_t Jump = L"Jump";
        OpCode_t Label = L"Label";
        OpCode_t Goto = L"Goto";
        OpCode_t If = L"If";
        OpCode_t Set = L"Set";
        OpCode_t Del = L"Del";
        OpCode_t ClearSubVar = L"Clear";
        OpCode_t JumpFile = L"JumpFile";
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
                else if (*token.value == L"goto") {
                    cmd.code = OpCode::Goto;
                }
                else if (*token.value == L"if") {
                    cmd.code = OpCode::If;
                }
                else if (*token.value == L"set") {
                    cmd.code = OpCode::Set;
                }
                else if (*token.value == L"clear") {
                    cmd.code = OpCode::ClearSubVar;
                }
                else if (*token.value == L"del") {
                    cmd.code = OpCode::Del;
                }
                else if (*token.value == L"jumpfile") {
                    cmd.code = OpCode::JumpFile;
                }
                else if (token.token_type == TokenType::DoubleColon) {
                    cmd.code = OpCode::Label;
                }
                else if(*token.value == L"call") {
                    cmd.code = OpCode::Call;
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