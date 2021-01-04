// #include "TokenType.h"
#include <iostream>
#include <string>
#include <sstream>
#include "lexer.h"
#include "Token.h"

namespace jxcode
{
    namespace lexer
    {

        using namespace std;

        LexerException::LexerException(const size_t& line, const size_t& pos_, const std::wstring& message)
            : line_(line), pos_(pos_), message_(message) { }

        wstring LexerException::what() const
        {
            wstringstream ss;
            ss << L"line: " << this->line_ << L", pos: " << this->pos_ << L"  ";
            ss << this->message_;
            return ss.str();
        }

        using std::wstring;
        using std::map;
        using std::vector;

        wchar_t scan_space = L' ';
        bool scan_is_parse_note = false;
        bool scan_is_parse_lf = true;

        wchar_t scan_string_bracket = L'"';
        wchar_t scan_string_escape_char = L'\\';

        std::wstring scan_single_note_opr = std::wstring(L"//");
        std::wstring scan_multiline_note_lbracket_opr = std::wstring(L"/*");
        std::wstring scan_multiline_note_rbracket_opr = std::wstring(L"*/");

        int cur_global_pos;
        int cur_line;
        int cur_position;

        //一次性数据，悬垂了也无所谓
        wstring* code_content;
        map<wstring, TokenType_t>* token_map;
        map<wstring, wstring>* esc_char_map;

        inline wchar_t GetChar(int pos = cur_global_pos) {
            if (code_content->length() <= pos) {
                return 0;
            }
            return (*code_content)[pos];
        }

        inline void Reset() {
            cur_line = -1;
            cur_position = -1;
            cur_global_pos = -1;
        }
        inline bool IsNumber(const wchar_t& c) {
            return c >= 48 && c <= 57;
        }

        inline bool IsString(const wchar_t& c) {
            return scan_string_bracket == c;
        }
        inline bool IsSymbol(const wchar_t& c) {
            if (c > 127) {
                return false;
            }
            return (c >= 33 && c <= 47) ||
                (c >= 58 && c <= 64) ||
                (c >= 91 && c <= 94) || (c == 96) || //95 = _
                (c >= 123 && c <= 126);
        }
        inline bool IsControlSymbol(const wchar_t& c) {
            return (c >= 0 && c <= 31) || c == 127;
        }
        inline bool IsWord(const wchar_t& c) {
            return !(IsSymbol(c) || IsControlSymbol(c) || IsNumber(c));
        }

        inline bool IsKeyOpr(const wstring& str, TokenType_t* out_type) {
            bool b = token_map->count(str) != 0;
            if (b) {
                *out_type = (*token_map)[str];
            }
            return b;
        }

        inline int GetCharType(const wchar_t& c) {
            if (c == scan_space)
                return 0;
            if (IsNumber(c))
                return 1;
            if (IsSymbol(c))
                return 2;
            if (IsControlSymbol(c))
                return 3;
            return 4; //Word
        }
        inline bool CharTypeEquals(int pos, int pos2) {
            wchar_t c1 = GetChar(pos);
            wchar_t c2 = GetChar(pos2);
            return GetCharType(c1) == GetCharType(c2);
        }

        inline wchar_t NextChar() {
            if ((int)code_content->length() <= cur_global_pos + 1) {
                cur_global_pos = (int)code_content->length();
                return 0;
            }
            cur_global_pos++;
            wchar_t c = (*code_content)[cur_global_pos];
            if (c == L'\n') {
                cur_position = -1;
                cur_line++;
            }
            else {
                cur_position++;
            }
            return c;
        }
        inline void Next(int length) {
            for (int i = 0; i < length; i++) NextChar();
        }
        inline void SkipSpace() {
            wchar_t c = GetChar();
            while (c == L' ' || c == L'\t' || c == L'\r') {
                NextChar();
                c = GetChar();
            }
        }
        inline wchar_t PeekChar(int offset = 1) {
            if ((int)code_content->length() <= cur_global_pos + offset) {
                return 0;
            }
            return (*code_content)[cur_global_pos + offset];
        }

        inline bool is_cur_singleline_note() {
            for (int i = 0; i < scan_single_note_opr.length(); i++) {
                if (scan_single_note_opr[i] != PeekChar(i)) {
                    return false;
                }
            }
            return true;
        }
        inline bool is_cur_multiline_note() {
            for (int i = 0; i < scan_multiline_note_lbracket_opr.length(); i++) {
                if (scan_multiline_note_lbracket_opr[i] != PeekChar(i)) {
                    return false;
                }
            }
            return true;
        }
        inline bool is_curc_space() {
            return GetChar() == L' ' || GetChar() == L'\r';
        }
        inline bool is_curc_lf() {
            return GetChar() == L'\n';
        }
        inline bool is_curc_symbol() {
            return IsSymbol(GetChar());
        }
        inline bool is_curc_null() {
            return GetChar() == 0;
        }
        wstring GetNumber() {

            int first = cur_global_pos;

            int length = 0;
            bool isDecimal = false;

            wchar_t c = GetChar();
            while (IsNumber(c)) {
                if (c == L'.') {
                    if (isDecimal) {
                        throw LexerException(cur_line + 1, cur_position + 1, L"Number format error");
                    }
                    continue;
                }
                c = NextChar();
                length++;
            }

            return code_content->substr(first, length);
        }
        wstring GetString() {
            NextChar();
            wchar_t c;
            int len = 0;
            while (true) {
                c = PeekChar(len);
                if (c == 0) {
                    throw LexerException(cur_line + 1, cur_position + 1, L"字符串没有结尾");
                }

                //是转移符则跳过下个字符
                if (c == scan_string_escape_char) {
                    len += 2;
                    continue;
                }
                //字符串结束符
                if (c == scan_string_bracket) {
                    break;
                }
                len++;
            }
            wstring str = (*code_content).substr(cur_global_pos, len);
            Next(len + 1);
            return str;
        }
        wstring GetSymbol(TokenType_t* type) {
            vector<wstring> matchList;
            int length = 0;
            //<字符, TokenType>
            for (auto it = token_map->begin(); it != token_map->end(); it++) {
                length = 0;
                const wstring& key = it->first;
                for (int i = 0; i < key.length(); i++) {
                    if (key[i] == PeekChar(i)) {
                        length++;
                    }
                }
                if (key.length() == length) {
                    matchList.push_back(key);
                }
            }

            wstring str;
            if (matchList.size() == 0) {
                *type = TokenType::Unknow;
                str = GetChar();
            }
            else {
                //匹配最长
                int maxIndex = 0;
                for (int i = 0; i < matchList.size(); i++) {
                    if (matchList[i].length() > matchList[maxIndex].length()) {
                        maxIndex = i;
                    }
                }
                str = matchList[maxIndex];
                *type = (*token_map)[str];
            }

            Next((int)str.length());
            return str;
        }
        wstring GetIdent() {
            int length = 0;
            int first = cur_global_pos;

            while (true) {
                NextChar();
                length++;
                if (is_curc_space() || is_curc_lf() || is_curc_symbol() || is_curc_null()) {
                    break;
                }
            }
            wstring str = code_content->substr(first, length);
            return str;
        }

        wstring GetNote() {
            int length = 0;
            int first = 0;
            if (is_cur_singleline_note()) {
                first = cur_global_pos + (int)scan_single_note_opr.length();
                Next((int)scan_single_note_opr.length() - 1);
                wchar_t c;
                while ((c = NextChar()) != L'\n') {
                    length++;
                }
            }
            else if (is_cur_multiline_note()) {
                first = cur_global_pos + (int)scan_multiline_note_lbracket_opr.length();
                Next((int)scan_multiline_note_lbracket_opr.length() - 1);
                wchar_t nChar;
                while (true) {
                    if ((nChar = NextChar()) != scan_multiline_note_rbracket_opr[0]) {
                        length++;
                        continue;
                    }
                    int count = 0;
                    for (int i = 0; i < scan_multiline_note_rbracket_opr.length(); i++) {
                        if (scan_multiline_note_rbracket_opr[i] == PeekChar(i)) {
                            count++;
                        }
                    }
                    //注释右括号匹配
                    if (count == scan_multiline_note_rbracket_opr.length()) {
                        length += count;
                        Next(count);
                        length -= (int)scan_multiline_note_rbracket_opr.length();
                        break;
                    }
                    else {
                        length++;
                        continue;
                    }
                }
            }
            return code_content->substr(first, length);
        }

        bool GetToken(Token* out_token) {

            SkipSpace();
            wchar_t c = GetChar();
            if (!c) return false;

            //从1行开始
            out_token->line = cur_line + 1;
            out_token->position = cur_position + 1;

            int startPosition = cur_global_pos;
            if (c == L'\n') {
                out_token->value = make_shared<wstring>(wstring(L"\n"));
                NextChar();
                out_token->token_type = TokenType::LF;
            }
            else if (is_cur_singleline_note() || is_cur_multiline_note()) {
                out_token->value = make_shared<wstring>(GetNote());
                out_token->token_type = TokenType::Note;
            }
            else if (IsNumber(c)) {
                out_token->value = make_shared<wstring>(GetNumber());
                out_token->token_type = TokenType::Number;
            }
            else if (scan_string_bracket == c) {
                out_token->value = make_shared<wstring>(GetString());
                out_token->token_type = TokenType::String;
            }
            else if (IsSymbol(c)) {
                TokenType_t type;
                out_token->value = make_shared<wstring>(GetSymbol(&type));
                out_token->token_type = (type);
            }
            else if (IsWord(c)) {
                wstring str = GetIdent();
                TokenType_t type;
                out_token->value = make_shared<wstring>(str);
                if (IsKeyOpr(str, &type)) {
                    out_token->token_type = type;
                }
                else {
                    out_token->token_type = TokenType::Ident;
                }
            }
            if (startPosition == cur_global_pos)
                return false;
            return true;
        }
        vector<Token> Scanner(
            wstring* code,
            map<wstring, TokenType_t>* tokenList,
            std::map<std::wstring, std::wstring>* escMap
        ) {
            Reset();
            code_content = code;
            token_map = tokenList;
            esc_char_map = escMap;
            vector<Token> tokens;
            Token token;
            cur_line = 0;

            NextChar();
            while (GetToken(&token)) {
                TokenType_t type = token.token_type;
                if (type == TokenType::Note && !scan_is_parse_note) {
                    continue;
                }
                else if (type == TokenType::LF && !scan_is_parse_lf) {
                    continue;
                }
                tokens.push_back(token);
            }
            return tokens;
        }


        map<wstring, TokenType_t> get_std_operator_map() {
            map<wstring, TokenType_t> mp;
            mp[L"=="] = TokenType::DoubleEqual;
            mp[L"="] = TokenType::Equal;
            mp[L"("] = TokenType::LBracket;
            mp[L")"] = TokenType::RBracket;
            mp[L"&&"] = TokenType::And;
            mp[L"||"] = TokenType::Or;
            mp[L"*"] = TokenType::Multiple;
            mp[L"-"] = TokenType::Minus;
            mp[L"+"] = TokenType::Plus;
            mp[L"/"] = TokenType::Division;
            mp[L"!="] = TokenType::ExclamatoryAndEqual;
            mp[L"::"] = TokenType::DoubleColon;
            mp[L":"] = TokenType::Colon;
            mp[L","] = TokenType::Comma;
            mp[L"."] = TokenType::Dot;
            return mp;
        }

        map<wstring, TokenType_t> get_nep_operator_map() {
            map<wstring, TokenType_t> mp;
            return mp;
        }
        map<wstring, wstring> get_std_esc_char_map() {
            map<wstring, wstring> mp;
            mp[L"r"] = wstring(L"\r");
            mp[L"n"] = wstring(L"\n");
            mp[L"t"] = wstring(L"\t");
            return mp;
        }

    }
}

