
class Location {
private:
    size_t row, col;
    std::string file_name;
public:
    Location(size_t row, size_t col, const std::string& file_name)
        : row(row), col(col), file_name(file_name)
    {}

    void advance_line() {
        this->col = 1;
        this->row += 1;
    }

    void advance_col() {
        this->col += 1;
    }

    size_t get_row() const {
        return this->row;
    }
    
    size_t get_col() const {
        return this->col;
    }
    
    const std::string& get_file_name() const {
        return this->file_name;
    }

    ~Location() {}
};

std::ostream& operator<<(std::ostream& output_stream, const Location& location) {
    output_stream << location.get_file_name() << ":" << location.get_row() << ":" << location.get_col(); 
    return output_stream;
}

// I HECKIN LOVE X-MACROS

#define TOKEN_TYPE_LIST \
    TOKEN_TYPE_ENTRY(INT_LITERAL) \
    TOKEN_TYPE_ENTRY(FLOAT_LITERAL) \
    TOKEN_TYPE_ENTRY(STRING_LITERAL) \
    TOKEN_TYPE_ENTRY(CHAR_LITERAL) \
    TOKEN_TYPE_ENTRY(NAME) \
    \
    TOKEN_TYPE_ENTRY(PLUS) \
    TOKEN_TYPE_ENTRY(MINUS) \
    TOKEN_TYPE_ENTRY(STAR) \
    TOKEN_TYPE_ENTRY(SLASH) \
    TOKEN_TYPE_ENTRY(BANG) \
    TOKEN_TYPE_ENTRY(TILDE) \
    TOKEN_TYPE_ENTRY(PERCENT) \
    TOKEN_TYPE_ENTRY(LESS_LESS) \
    TOKEN_TYPE_ENTRY(GREATER_GREATER) \
    TOKEN_TYPE_ENTRY(LESS) \
    TOKEN_TYPE_ENTRY(LESS_EQUAL) \
    TOKEN_TYPE_ENTRY(GREATER) \
    TOKEN_TYPE_ENTRY(GREATER_EQUAL) \
    TOKEN_TYPE_ENTRY(EQUAL_EQUAL) \
    TOKEN_TYPE_ENTRY(BANG_EQUAL) \
    TOKEN_TYPE_ENTRY(AND) \
    TOKEN_TYPE_ENTRY(HAT) \
    TOKEN_TYPE_ENTRY(PIPE) \
    TOKEN_TYPE_ENTRY(AND_AND) \
    TOKEN_TYPE_ENTRY(PIPE_PIPE) \
    TOKEN_TYPE_ENTRY(EQUAL) \
    \
    TOKEN_TYPE_ENTRY(OPEN_PARENTHESIS) \
    TOKEN_TYPE_ENTRY(CLOSE_PARENTHESIS) \
    TOKEN_TYPE_ENTRY(OPEN_CURLY_BRACE) \
    TOKEN_TYPE_ENTRY(CLOSE_CURLY_BRACE) \
    TOKEN_TYPE_ENTRY(OPEN_SQUARE_BRACKET) \
    TOKEN_TYPE_ENTRY(CLOSE_SQUARE_BRACKET) \
    \
    TOKEN_TYPE_ENTRY(COMMA) \
    TOKEN_TYPE_ENTRY(SEMI_COLON) \
    TOKEN_TYPE_ENTRY(COLON) \
    TOKEN_TYPE_ENTRY(DOT) \
    \
    TOKEN_TYPE_ENTRY(TRUE_KEYWORD) \
    TOKEN_TYPE_ENTRY(FALSE_KEYWORD) \
    TOKEN_TYPE_ENTRY(VAR_KEYWORD) \
    TOKEN_TYPE_ENTRY(IF_KEYWORD) \
    TOKEN_TYPE_ENTRY(ELSE_KEYWORD) \
    TOKEN_TYPE_ENTRY(WHILE_KEYWORD) \
    TOKEN_TYPE_ENTRY(BREAK_KEYWORD) \
    TOKEN_TYPE_ENTRY(CONTINUE_KEYWORD) \
    TOKEN_TYPE_ENTRY(INT_KEYWORD) \
    TOKEN_TYPE_ENTRY(FLOAT_KEYWORD) \
    TOKEN_TYPE_ENTRY(BOOL_KEYWORD) \
    TOKEN_TYPE_ENTRY(STRING_KEYWORD) \
    TOKEN_TYPE_ENTRY(CHAR_KEYWORD) \
    \
    TOKEN_TYPE_ENTRY(END_OF_FILE) \

#define TOKEN_TYPE_ENTRY(TOKEN_TYPE) TOKEN_TYPE,

enum class TokenType {
    TOKEN_TYPE_LIST
};

#undef TOKEN_TYPE_ENTRY

#define TOKEN_TYPE_ENTRY(TOKEN_TYPE) case TokenType:: TOKEN_TYPE : output_stream << #TOKEN_TYPE ; break;

std::ostream& operator<<(std::ostream& output_stream, const TokenType& token_type) {
    switch (token_type) {
        TOKEN_TYPE_LIST
    }
    return output_stream;
}

#undef TOKEN_TYPE_ENTRY

class Token {
private:
    TokenType type;
    std::string text;
    Location location;
public:
    Token(TokenType type, const std::string& text, const Location& location)
        : type(type), text(text), location(location)
    {}

    TokenType get_type() const {
        return this->type;
    }

    const std::string& get_text() const {
        return this->text;
    }

    const Location& get_location() const {
        return this->location;
    }

};

std::ostream& operator<<(std::ostream& output_stream, const Token& token) {
    output_stream
        << "Token {"
        << "type: " << token.get_type() << ", "
        << "text: '" << token.get_text() << "', "
        << "location: " << token.get_location() << " }";
    return output_stream;
}


std::string read_file_as_string(const std::string& file_path) {
    std::ifstream file_stream(file_path);
    if (file_stream.fail()) {
        std::cerr << "IOError: Failed to load file " << file_path << std::endl;
        std::exit(1);
    }
    std::stringstream string_buffer;
    string_buffer << file_stream.rdbuf();
    file_stream.close();
    return string_buffer.str();
}

class Tokenizer {
private:
    const std::string source;
    Location current_location;
    size_t source_pointer;
    std::unordered_map<std::string, TokenType> keyword_table;

public:
    Tokenizer(const std::string& file_path)
        : source(read_file_as_string(file_path)), current_location(1,1, file_path), source_pointer(0)
    {
        this->keyword_table = {
            { "true", TokenType::TRUE_KEYWORD },
            { "false", TokenType::FALSE_KEYWORD },
            { "var", TokenType::VAR_KEYWORD },
            { "if", TokenType::IF_KEYWORD },
            { "else", TokenType::ELSE_KEYWORD },
            { "while", TokenType::WHILE_KEYWORD },
            { "break", TokenType::BREAK_KEYWORD },
            { "continue", TokenType::CONTINUE_KEYWORD },
            { "int", TokenType::INT_KEYWORD },
            { "float", TokenType::FLOAT_KEYWORD },
            { "bool", TokenType::BOOL_KEYWORD },
            { "string", TokenType::STRING_KEYWORD },
            { "char", TokenType::CHAR_KEYWORD },
        };
    }

private:
    char current_char() const {
        if (this->source_pointer > this->source.length()) {
            return '\0';
        }
        
        return this->source[this->source_pointer];
    }

    void advance_char() {
        if (this->current_char() == '\n') {
            this->current_location.advance_line();
        } else {
            this->current_location.advance_col();
        }
        this->source_pointer += 1;
    }

    static bool is_name_character(char c) {
        return std::isalnum(c) || std::isdigit(c) || c == '_';  
    }

    Token next_token() {
        while (std::isspace(this->current_char())) {
            this->advance_char();
        }
        
        switch (this->current_char()) {
            case '+':
                {
                    Token plus_token = Token(TokenType::PLUS, "+", this->current_location);
                    this->advance_char();
                    return plus_token;
                }
            
            case '-':
                {
                    Token minus_token = Token(TokenType::MINUS, "-", this->current_location);
                    this->advance_char();
                    return minus_token;
                }
            
            case '*':
                {
                    Token star_token = Token(TokenType::STAR, "*", this->current_location);
                    this->advance_char();
                    return star_token;
                }
            
            case '/':
                {
                    Token slash_token = Token(TokenType::SLASH, "/", this->current_location);
                    this->advance_char();
                    return slash_token;
                }
            
            case '!': 
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '=') {
                        this->advance_char();
                        return Token(TokenType::BANG_EQUAL, "!=", token_location);
                    } else {
                        return Token(TokenType::BANG, "!", token_location);
                    }
                }
            
            case '~':
                {
                    Token tilde_token = Token(TokenType::TILDE, "~", this->current_location);
                    this->advance_char();
                    return tilde_token;
                }
            
            case '%':
                {
                    Token percent_token = Token(TokenType::PERCENT, "%", this->current_location);
                    this->advance_char();
                    return percent_token;
                }
            
            case '<': 
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '<') {
                        this->advance_char();
                        return Token(TokenType::LESS_LESS, "<<", token_location);
                    } else if (this->current_char() == '=') {
                        this->advance_char();
                        return Token(TokenType::LESS_EQUAL, "<=", token_location);
                    } else {
                        return Token(TokenType::LESS, "<", token_location);
                    }
                }
            
            case '>':
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '>') {
                        this->advance_char();
                        return Token(TokenType::GREATER_GREATER, ">>", token_location);
                    } else if (this->current_char() == '=') {
                        this->advance_char();
                        return Token(TokenType::GREATER_EQUAL, ">=", token_location);
                    } else {
                        return Token(TokenType::GREATER, ">", token_location);
                    }
                }
            
            case '=':
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '=') {
                        this->advance_char();
                        return Token(TokenType::EQUAL_EQUAL, "==", token_location);
                    } else {
                        return Token(TokenType::EQUAL, "=", token_location);
                    }
                }
            
            case '&':
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '&') {
                        this->advance_char();
                        return Token(TokenType::AND_AND, "&&", token_location);
                    } else {
                        return Token(TokenType::AND, "&", token_location);
                    }
                }
            
            case '|':
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    if (this->current_char() == '|') {
                        this->advance_char();
                        return Token(TokenType::PIPE_PIPE, "||", token_location);
                    } else {
                        return Token(TokenType::PIPE, "|", token_location);
                    }
                }
            
            case '^':
                {
                    Location token_location = this->current_location;
                    this->advance_char();
                    return Token(TokenType::HAT, "^", token_location);
                }
            
            case ',':
                {
                    Token comma_token = Token(TokenType::COMMA, ",", this->current_location);
                    this->advance_char();
                    return comma_token;
                }
            
            case ';':
                {
                    Token semi_colon_token = Token(TokenType::SEMI_COLON, ";", this->current_location);
                    this->advance_char();
                    return semi_colon_token;
                }
            
            case ':':
                {
                    Token colon_token = Token(TokenType::COLON, ":", this->current_location);
                    this->advance_char();
                    return colon_token;
                }
            
            case '.':
                {
                    Token dot_token = Token(TokenType::DOT, ".", this->current_location);
                    this->advance_char();
                    return dot_token;
                }
            
            case '(':
                {
                    Token open_parenthesis_token = Token(TokenType::OPEN_PARENTHESIS, "(", this->current_location);
                    this->advance_char();
                    return open_parenthesis_token;
                }
            
            case ')':
                {
                    Token close_parenthesis_token = Token(TokenType::CLOSE_PARENTHESIS, "(", this->current_location);
                    this->advance_char();
                    return close_parenthesis_token;
                }
            
            case '{':
                {
                    Token open_curly_brace_token = Token(TokenType::OPEN_CURLY_BRACE, "{", this->current_location);
                    this->advance_char();
                    return open_curly_brace_token;
                }
            
            case '}':
                {
                    Token close_curly_brace_token = Token(TokenType::CLOSE_CURLY_BRACE, "}", this->current_location);
                    this->advance_char();
                    return close_curly_brace_token;
                }
            
            case '[':
                {
                    Token open_square_bracket_token = Token(TokenType::OPEN_SQUARE_BRACKET, "[", this->current_location);
                    this->advance_char();
                    return open_square_bracket_token;
                }
            
            case ']':
                {
                    Token close_square_bracket_token = Token(TokenType::CLOSE_SQUARE_BRACKET, "]", this->current_location);
                    this->advance_char();
                    return close_square_bracket_token;
                }

            case '\0':
                {
                    return Token(TokenType::END_OF_FILE, "", this->current_location);
                }

            case '"':
                {
                    Location start_location = this->current_location;
                    size_t start_pointer = this->source_pointer;
                    char last_char = this->current_char();
                    this->advance_char();
                    while ((this->current_char() != '"' || last_char == '\\') && this->current_char() != '\0' && this->current_char() != '\n') {
                        last_char = this->current_char();
                        this->advance_char();
                    }
                    
                    if (this->current_char() != '"') {
                        std::cerr << start_location << " LEX_ERROR: Unterminated string literal." << std::endl;
                        std::exit(1);
                    }

                    this->advance_char();
                    size_t end_pointer = this->source_pointer;
                    std::string string_literal_string = source.substr(start_pointer, end_pointer - start_pointer);
                    return Token(TokenType::STRING_LITERAL, string_literal_string, start_location);
                }
            
            case '\'':
                {
                    Location start_location = this->current_location;
                    size_t start_pointer = this->source_pointer;
                    char last_char = this->current_char();
                    this->advance_char();
                    while ((this->current_char() != '\'' || last_char == '\\') && this->current_char() != '\0' && this->current_char() != '\n') {
                        last_char = this->current_char();
                        this->advance_char();
                    }
                    
                    if (this->current_char() != '\'') {
                        std::cerr << start_location << " LEX_ERROR: Unterminated char literal." << std::endl;
                        std::exit(1);
                    }

                    this->advance_char();
                    size_t end_pointer = this->source_pointer;
                    std::string char_literal_string = source.substr(start_pointer, end_pointer - start_pointer);
                    return Token(TokenType::CHAR_LITERAL, char_literal_string, start_location);
                }
            
            default:
                {
                    if (std::isdigit(this->current_char())) {
                        size_t start_pointer = this->source_pointer;
                        Location start_location = this->current_location;
                        
                        while (std::isdigit(this->current_char())) {
                            this->advance_char();
                        }

                        if (this->current_char() == '.') {
                            this->advance_char();
                            size_t decimal_count = 0;
                            while (std::isdigit(this->current_char())) {
                                this->advance_char();
                                decimal_count += 1;
                            }

                            if (decimal_count == 0) {
                                std::cerr << start_location << ": LEX_ERROR: Float literal is expected to have at least one decimal." << std::endl;
                                std::exit(1);
                            }

                            size_t end_pointer = this->source_pointer;
                            std::string float_literal_string = this->source.substr(start_pointer, end_pointer-start_pointer);
                            return Token(TokenType::FLOAT_LITERAL, float_literal_string, start_location);
                        } else {
                            size_t end_pointer = this->source_pointer;
                            std::string int_literal_string = this->source.substr(start_pointer, end_pointer-start_pointer);
                            return Token(TokenType::INT_LITERAL, int_literal_string, start_location);
                        }
                    } else if (is_name_character(this->current_char())) {
                        size_t start_pointer = this->source_pointer;
                        Location start_location = this->current_location;
                        
                        while (is_name_character(this->current_char())) {
                            this->advance_char();
                        }
                        
                        size_t end_pointer = this->source_pointer;
                        std::string name_string = this->source.substr(start_pointer, end_pointer-start_pointer);
                        if (this->keyword_table.contains(name_string)) {
                            return Token(this->keyword_table[name_string], name_string, start_location);
                        } else {
                            return Token(TokenType::NAME, name_string, start_location);
                        }
                    } else {
                        std::cerr << this->current_location << ": LEX_ERROR: Unexpected character '" << this->current_char() << "'" << std::endl;
                        std::exit(1);
                    }
                }
        }
    }

public:
    std::vector<Token> collect_tokens() {
        std::vector<Token> token_vector;
        
        for(;;) {
            Token next = this->next_token();
            token_vector.push_back(next);
            if (next.get_type() == TokenType::END_OF_FILE)
                break;
        } 

        return token_vector;
    }

    ~Tokenizer() {}
};

