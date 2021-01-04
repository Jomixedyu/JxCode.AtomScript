#include <vector>
#include <string>
#include <map>
#include "Token.h"

namespace jxcode
{
    namespace lexer
    {
        class LexerException
        {
        protected:
            std::wstring message_;
            size_t line_;
            size_t pos_;
        public:

        public:
            LexerException(const size_t& line, const size_t& pos_, const std::wstring& message);
        public:
            virtual std::wstring what() const;
        };


        extern wchar_t scan_space;
        extern bool scan_is_parse_note;
        extern bool scan_is_parse_lf;

        extern wchar_t scan_string_bracket;
        extern wchar_t scan_string_escape_char;

        extern std::wstring scan_single_note_opr;
        extern std::wstring scan_multiline_note_lbracket_opr;
        extern std::wstring scan_multiline_note_rbracket_opr;

        std::vector<Token> Scanner(
            std::wstring* code,
            std::map<std::wstring, TokenType_t>* tokenList,
            std::map<std::wstring, std::wstring>* escMap
        );

        std::map<std::wstring, TokenType_t> get_std_operator_map();
        std::map<std::wstring, std::wstring> get_std_esc_char_map();
    }
}