// #include "TokenType.h"
#include <iostream>
#include <string>
#include <sstream>
#include "lexer.h"
#include "Token.h"

namespace jxcode::lexer
{
    using namespace std;

    LexerException::LexerException(const size_t& line, const size_t& pos_, const std::wstring& message)
        : line_(line), pos_(pos_), wexceptionbase(message) { }

    std::wstring LexerException::what()
    {
        wstringstream ss;
        ss << L"line: " << this->line_ << L", pos: " << this->pos_ << L"  ";
        ss << this->message_;
        return ss.str();
    }

    wchar_t Lexer::GetChar()
    {
        return GetChar(cur_global_pos);
    }

    wchar_t Lexer::GetChar(int pos)
    {
        if (code_content->length() <= pos) {
            return 0;
        }
        return (*code_content)[pos];
    }

    inline bool IsNumber(const wchar_t& c) {
        return c >= L'0' && c <= L'9';
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

    bool Lexer::IsOperatorInMap(const std::wstring& str, TokenType* out_type) {
        bool b = token_map->count(str) != 0;
        if (b) {
            *out_type = (*token_map)[str];
        }
        return b;
    }

    int Lexer::GetCharType(const wchar_t& c)
    {
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
    bool Lexer::CharTypeEquals(int pos, int pos2)
    {
        wchar_t c1 = GetChar(pos);
        wchar_t c2 = GetChar(pos2);
        return GetCharType(c1) == GetCharType(c2);
    }

    wchar_t Lexer::NextChar()
    {
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
    void Lexer::Next(int length)
    {
        for (int i = 0; i < length; i++) NextChar();
    }

    void Lexer::SkipSpace()
    {
        wchar_t c = GetChar();
        while (c == L' ' || c == L'\t' || c == L'\r') {
            NextChar();
            c = GetChar();
        }
    }
    wchar_t Lexer::PeekChar(int offset)
    {
        if ((int)code_content->length() <= cur_global_pos + offset) {
            return 0;
        }
        return (*code_content)[(size_t)cur_global_pos + (size_t)offset];
    }

    bool Lexer::is_cur_singleline_note()
    {
        for (int i = 0; i < single_note_opr.length(); i++) {
            if (single_note_opr[i] != PeekChar(i)) {
                return false;
            }
        }
        return true;
    }
    bool Lexer::is_cur_multiline_note()
    {
        for (int i = 0; i < multiline_note_lbracket_opr.length(); i++) {
            if (multiline_note_lbracket_opr[i] != PeekChar(i)) {
                return false;
            }
        }
        return true;
    }

    bool Lexer::is_curc_symbol()
    {
        return IsSymbol(GetChar());
    }

    bool Lexer::ShouldGetNumber()
    {
        wchar_t c = PeekChar(0);
        return IsNumber(c) || (c == L'-' && IsNumber(PeekChar(1)));
    }

    wstring Lexer::GetNumber()
    {
        int length = 0;
        bool isDecimal = false;

        wchar_t c = GetChar();
        if (c == L'-') {
            if (!IsNumber(PeekChar(1))) {
                throw LexerException((size_t)cur_line + 1, (size_t)cur_position + 1, L"Number format error");
            }
            length++;
            c = PeekChar(length);
        }

        while (IsNumber(c) || c == L'.') {
            if (c == L'.') {
                if (isDecimal) {
                    throw LexerException((size_t)cur_line + 1, (size_t)cur_position + 1, L"Number format error");
                }
                isDecimal = true;
            }
            length++;
            c = PeekChar(length);
        }
        int head = cur_global_pos;
        Next(length);
        return code_content->substr(head, length);
    }
    wstring Lexer::GetString()
    {
        wchar_t c;
        int len = 0;
        wstring appstr;
        while (true) {
            c = PeekChar();

            if (c == 0) {
                throw LexerException((size_t)cur_line + 1, (size_t)cur_position + 1, L"×Ö·û´®Ã»ÓÐ½áÎ²");
            }

            //ÊÇ×ªÒÆ·ûÔòÌø¹ýÏÂ¸ö×Ö·û
            if (c == string_escape_char) {
                const wstring* ref1 = nullptr;
                const wstring* ref2 = nullptr;
                for (auto& item : *esc_char_map) {
                    int count = 0;
                    for (int i = 0; i < item.first.length(); i++)
                    {
                        if (PeekChar(i + 2) == item.first[i]) {
                            count++;
                        }
                    }
                    if (item.first.length() == count) {
                        ref1 = &item.first;
                        ref2 = &item.second;
                        break;
                    }
                }
                if (ref2 == nullptr) {
                    throw LexerException((size_t)cur_line + 1, (size_t)cur_position + 1, L"×Ö·û´®×ªÒå´íÎó");
                }
                appstr += *ref2;
                Next(1 + ref1->length()); // \n
                continue;
            }
            //×Ö·û´®½áÊø·û
            if (c == string_bracket) {
                NextChar();
                break;
            }

            appstr += c;
            NextChar();
        }

        NextChar(); // "
        return appstr;
    }
    wstring Lexer::GetSymbol(TokenType* type) {
        vector<wstring> matchList;
        int length = 0;
        //<×Ö·û, TokenType>
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
            //Æ¥Åä×î³¤
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
    wstring Lexer::GetIdent()
    {
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

    wstring Lexer::GetNote()
    {
        int length = 0;
        int first = 0;
        if (is_cur_singleline_note()) {
            first = cur_global_pos + (int)single_note_opr.length();
            Next((int)single_note_opr.length() - 1);
            wchar_t c;
            while ((c = NextChar()) != L'\n') {
                length++;
            }
        }
        else if (is_cur_multiline_note()) {
            first = cur_global_pos + (int)multiline_note_lbracket_opr.length();
            Next((int)multiline_note_lbracket_opr.length() - 1);
            wchar_t nChar;
            while (true) {
                if ((nChar = NextChar()) != multiline_note_rbracket_opr[0]) {
                    length++;
                    continue;
                }
                int count = 0;
                for (int i = 0; i < multiline_note_rbracket_opr.length(); i++) {
                    if (multiline_note_rbracket_opr[i] == PeekChar(i)) {
                        count++;
                    }
                }
                //×¢ÊÍÓÒÀ¨ºÅÆ¥Åä
                if (count == multiline_note_rbracket_opr.length()) {
                    length += count;
                    Next(count);
                    length -= (int)multiline_note_rbracket_opr.length();
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

    bool Lexer::GetToken(Token* out_token)
    {

        SkipSpace();
        wchar_t c = GetChar();
        if (!c) return false;

        //´Ó1ÐÐ¿ªÊ¼
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
        else if (ShouldGetNumber()) {
            out_token->value = make_shared<wstring>(GetNumber());
            out_token->token_type = TokenType::Number;
        }
        else if (string_bracket == c) {
            out_token->value = make_shared<wstring>(GetString());
            out_token->token_type = TokenType::String;
        }
        else if (IsSymbol(c)) {
            TokenType type;
            out_token->value = make_shared<wstring>(GetSymbol(&type));
            out_token->token_type = (type);
        }
        else if (IsWord(c)) {
            wstring str = GetIdent();
            TokenType type;
            out_token->value = make_shared<wstring>(str);
            if (IsOperatorInMap(str, &type)) {
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
    std::vector<shared_ptr<Token>> Lexer::Scanner(
        wstring* code,
        map<wstring, TokenType>* tokenList,
        std::map<std::wstring, std::wstring>* escMap
    ) {
        Reset();
        code_content = code;
        token_map = tokenList;
        esc_char_map = escMap;

        vector<shared_ptr<Token>> tokens;
        Token token;
        cur_line = 0;

        NextChar();
        while (GetToken(&token)) {
            TokenType type = token.token_type;
            if (type == TokenType::Note && !is_parse_note) {
                continue;
            }
            else if (type == TokenType::LF && !is_parse_lf) {
                continue;
            }
            tokens.push_back(make_shared<Token>(token));
        }
        return tokens;
    }


    map<wstring, TokenType> get_std_operator_map() {
        map<wstring, TokenType> mp;
        mp[L"=="] = TokenType::DoubleEqual;
        mp[L"!="] = TokenType::ExclamatoryAndEqual;
        mp[L"="] = TokenType::Equal;
        mp[L"("] = TokenType::LBracket;
        mp[L")"] = TokenType::RBracket;
        mp[L"&&"] = TokenType::And;
        mp[L"||"] = TokenType::Or;
        mp[L"*"] = TokenType::Multiple;
        mp[L"-"] = TokenType::Minus;
        mp[L"+"] = TokenType::Plus;
        mp[L"/"] = TokenType::Division;
        mp[L","] = TokenType::Comma;
        mp[L"."] = TokenType::Dot;

        mp[L">"] = TokenType::GreaterThan;
        mp[L">="] = TokenType::GreaterThanEqual;

        mp[L"<"] = TokenType::LessThan;
        mp[L"<="] = TokenType::LessThanEqual;

        mp[L"!"] = TokenType::Exclamatory;
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

