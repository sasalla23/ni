#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

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
    TOKEN_TYPE_ENTRY(PLUS) \
    TOKEN_TYPE_ENTRY(MINUS) \
    TOKEN_TYPE_ENTRY(STAR) \
    TOKEN_TYPE_ENTRY(SLASH) \
    TOKEN_TYPE_ENTRY(OPEN_PARENTHESIS) \
    TOKEN_TYPE_ENTRY(CLOSE_PARENTHESIS) \
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

public:
    Tokenizer(const std::string& file_path)
        : source(read_file_as_string(file_path)), current_location(1,1, file_path), source_pointer(0)
    {}


    char curr_char() const {
        if (this->source_pointer > this->source.length()) {
            return '\0';
        }
        
        return this->source[this->source_pointer];
    }

    void advance_char() {
        if (this->curr_char() == '\n') {
            this->current_location.advance_line();
        } else {
            this->current_location.advance_col();
        }
        this->source_pointer += 1;
    }

    Token next_token() {
        while (std::isspace(this->curr_char())) {
            this->advance_char();
        }
        
        switch (this->curr_char()) {
            case '+': {
                Token plus_token = Token(TokenType::PLUS, "+", this->current_location);
                this->advance_char();
                return plus_token;
            }
            
            case '-': {
                Token minus_token = Token(TokenType::MINUS, "-", this->current_location);
                this->advance_char();
                return minus_token;
            }
            
            case '*': {
                Token star_token = Token(TokenType::STAR, "*", this->current_location);
                this->advance_char();
                return star_token;
            }
            
            case '/': {
                Token slash_token = Token(TokenType::SLASH, "/", this->current_location);
                this->advance_char();
                return slash_token;
            }
            
            case '(': {
                Token open_parenthesis_token = Token(TokenType::OPEN_PARENTHESIS, "(", this->current_location);
                this->advance_char();
                return open_parenthesis_token;
            }
            
            case ')': {
                Token close_parenthesis_token = Token(TokenType::CLOSE_PARENTHESIS, "(", this->current_location);
                this->advance_char();
                return close_parenthesis_token;
            }

            case '\0': {
                return Token(TokenType::END_OF_FILE, "", this->current_location);
            }
            
            default: {
                if (std::isdigit(this->curr_char())) {
                    size_t start_pointer = this->source_pointer;
                    Location start_location(this->current_location);
                    
                    while (std::isdigit(this->curr_char())) {
                        this->advance_char();
                    }
                    
                    size_t end_pointer = this->source_pointer;
                    std::string int_literal_string = this->source.substr(start_pointer, end_pointer-start_pointer);
                    return Token(TokenType::INT_LITERAL, int_literal_string, start_location);
                } else {
                    std::cerr << this->current_location << ": LEX_ERROR: Unexpected character '" << this->curr_char() << "'" << std::endl;
                    std::exit(1);
                }
            }
        }
    }

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

class Expression {
private:
    Location location;

public:
    Expression(const Location& location) : location(location)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    const Location& get_location() const {
        return this->location;
    }

    virtual ~Expression() {};
};

std::ostream& operator<<(std::ostream& output_stream, const Expression& expression) {
    expression.append_to_output_stream(output_stream);
    return output_stream;
}

class BinaryExpression : public Expression {
private:
    std::unique_ptr<Expression> left, right;
    Token operator_token;
public:

    BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, const Token& operator_token)
        : Expression(left->get_location()), operator_token(operator_token)
    {
        this->left = std::move(left);
        this->right = std::move(right);
    }
    
    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        for (size_t i = 0; i < layer; i++) {
            output_stream << "\t";
        }
        output_stream << "BinaryExpression(" << this->operator_token.get_text() << ")" << std::endl;
        this->left->append_to_output_stream(output_stream, layer+1);
        this->right->append_to_output_stream(output_stream, layer+1);
    }

    ~BinaryExpression() {}
};

class LiteralExpression : public Expression {
private:
    Token literal_token;
public:
    LiteralExpression(const Token& literal_token)
        : Expression(literal_token.get_location()), literal_token(literal_token)
    {}
    
    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        for (size_t i = 0; i < layer; i++) {
            output_stream << "\t";
        }
        output_stream << "LiteralExpression(" << this->literal_token.get_text() << ")" << std::endl;
    }

    ~LiteralExpression() {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t token_pointer;
public:
    Parser(std::vector<Token> tokens)
        : tokens(tokens), token_pointer(0)
    {}

    Token get_current_token() const {
        assert(tokens.size() > 0);
        if (this->token_pointer >= this->tokens.size()) {
            return this->tokens[this->tokens.size() - 1];
        }
        return this->tokens[this->token_pointer];
    }

    Token consume_token() {
        Token output_token = this->get_current_token();
        this->token_pointer += 1;
        return output_token;
    }
    
    Token expect_token(TokenType type) {
        Token current_token = this->consume_token();
        if (current_token.get_type() != type) {
            std::cerr << current_token.get_location() << ": PARSE_ERROR: Unexpected token of type " << '<' << current_token.get_type() << '>' << ", expected " << '<' << type << '>' << "." << std::endl;
            std::exit(1);
        }
        return current_token;
    }

    std::unique_ptr<Expression> parse_expression() {
        return parse_additive_expression();
    }

    std::unique_ptr<Expression> parse_additive_expression() {
        auto left = this->parse_multiplicative_expression();
        Token next_operator = get_current_token();
        while (next_operator.get_type() == TokenType::PLUS || next_operator.get_type() == TokenType::MINUS) {
            (void)this->consume_token();
            auto right = this->parse_multiplicative_expression();
            left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), next_operator);
            next_operator = this->get_current_token();
        }
        return left;
    }

    std::unique_ptr<Expression> parse_multiplicative_expression() {
        auto left = this->parse_primary_expression();
        Token next_operator = this->get_current_token();
        while (next_operator.get_type() == TokenType::STAR || next_operator.get_type() == TokenType::SLASH) {
            (void)this->consume_token();
            auto right = this->parse_primary_expression();
            left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), next_operator);
            next_operator = this->get_current_token();
        }
        return left;
    }

    std::unique_ptr<Expression> parse_primary_expression() {
        Token current_token = this->get_current_token();
        if (current_token.get_type() == TokenType::INT_LITERAL) {
            return std::make_unique<LiteralExpression>(this->consume_token());
        } else if (current_token.get_type() == TokenType::OPEN_PARENTHESIS) {
            (void)this->consume_token();
            auto inner_expression = parse_expression();
            this->expect_token(TokenType::CLOSE_PARENTHESIS);
            return inner_expression;
        } else {
            std::cerr << current_token.get_location() << ": PARSE_ERROR: Unexpected token of type <" << current_token.get_type() << "> at the beginning of a primary expression." << std::endl;
            std::exit(1);
        }
    }

    ~Parser() {}
};

int main(void) {
    Tokenizer tokenizer("test.ni");
    auto tokens = tokenizer.collect_tokens();
    Parser parser(tokens);
    auto expr = parser.parse_expression();
    std::cout << *expr;

    return 0;
}
