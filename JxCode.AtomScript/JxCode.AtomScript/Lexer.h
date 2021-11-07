#include <vector>
#include <string>
#include <map>
#include "Token.h"
#include "wexceptionbase.h"

namespace jxcode::lexer
{
    class Lexer
    {
    public:
        wchar_t scan_space = L' ';
        bool is_parse_note = false;
        bool is_parse_lf = true;
        wchar_t string_bracket = L'"';
        wchar_t string_escape_char = L'\\';

        std::wstring single_note_opr = std::wstring(L"//");
        std::wstring multiline_note_lbracket_opr = std::wstring(L"/*");
        std::wstring multiline_note_rbracket_opr = std::wstring(L"*/");

        std::vector<std::shared_ptr<Token>> Scanner(
            std::wstring* code,
            std::map<std::wstring, TokenType>* tokenList,
            std::map<std::wstring, std::wstring>* escMap
        );
        
    private:
        int cur_global_pos;
        int cur_line;
        int cur_position;

        //一次性数据，悬垂了也无所谓
        std::wstring* code_content;
        std::map<std::wstring, TokenType>* token_map;
        std::map<std::wstring, std::wstring>* esc_char_map;

        wchar_t GetChar();
        wchar_t GetChar(int pos);

        bool IsOperatorInMap(const std::wstring& str, TokenType* out_type);
        bool CharTypeEquals(int pos, int pos2);
        wchar_t NextChar();
        void Next(int length);
        void SkipSpace();
        wchar_t PeekChar(int offset = 1);
        bool is_cur_singleline_note();
        bool is_cur_multiline_note();
        bool is_curc_space(){ return GetChar() == L' ' || GetChar() == L'\r'; }
        bool is_curc_lf() { return GetChar() == L'\n'; }
        bool is_curc_symbol();
        bool is_curc_null(){ return GetChar() == 0; }
        bool IsString(const wchar_t& c) { return string_bracket == c; }
        bool ShouldGetNumber();
        std::wstring GetNumber();
        std::wstring GetString();
        std::wstring GetSymbol(TokenType* type);
        std::wstring GetIdent();
        std::wstring GetNote();
        bool GetToken(Token* out_token);
        int GetCharType(const wchar_t& c);

        void Reset() {
            cur_line = -1;
            cur_position = -1;
            cur_global_pos = -1;
        }
    };

    class LexerException : public wexceptionbase
    {
    protected:
        size_t line_;
        size_t pos_;
    public:
        LexerException(const size_t& line, const size_t& pos_, const std::wstring& message);
    public:
        virtual std::wstring what() override;
    };

    std::map<std::wstring, TokenType> get_std_operator_map();
    std::map<std::wstring, std::wstring> get_std_esc_char_map();


}