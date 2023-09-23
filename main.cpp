#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <unordered_map>

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
    \
    TOKEN_TYPE_ENTRY(COMMA) \
    \
    TOKEN_TYPE_ENTRY(TRUE_KEYWORD) \
    TOKEN_TYPE_ENTRY(FALSE_KEYWORD) \
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
            { "false", TokenType::FALSE_KEYWORD }
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
            
            case '!': {
                Location token_location = this->current_location;
                this->advance_char();
                if (this->current_char() == '=') {
                    this->advance_char();
                    return Token(TokenType::BANG_EQUAL, "!=", token_location);
                } else {
                    return Token(TokenType::BANG, "!", token_location);
                }
            }
            
            case '~': {
                Token tilde_token = Token(TokenType::TILDE, "~", this->current_location);
                this->advance_char();
                return tilde_token;
            }
            
            case '%': {
                Token percent_token = Token(TokenType::PERCENT, "%", this->current_location);
                this->advance_char();
                return percent_token;
            }
            
            case '<': {
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
            
            case '>': {
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
            
            case '=': {
                Location token_location = this->current_location;
                this->advance_char();
                if (this->current_char() == '=') {
                    this->advance_char();
                    return Token(TokenType::EQUAL_EQUAL, "==", token_location);
                } else {
                    return Token(TokenType::EQUAL, "=", token_location);
                }
            }
            
            case '&': {
                Location token_location = this->current_location;
                this->advance_char();
                if (this->current_char() == '&') {
                    this->advance_char();
                    return Token(TokenType::AND_AND, "&&", token_location);
                } else {
                    return Token(TokenType::AND, "&", token_location);
                }
            }
            
            case '|': {
                Location token_location = this->current_location;
                this->advance_char();
                if (this->current_char() == '|') {
                    this->advance_char();
                    return Token(TokenType::PIPE_PIPE, "||", token_location);
                } else {
                    return Token(TokenType::PIPE, "|", token_location);
                }
            }
            
            case '^': {
                Location token_location = this->current_location;
                this->advance_char();
                return Token(TokenType::HAT, "^", token_location);
            }
            
            case ',': {
                Token comma_token = Token(TokenType::COMMA, ",", this->current_location);
                this->advance_char();
                return comma_token;
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
                if (std::isdigit(this->current_char())) {
                    size_t start_pointer = this->source_pointer;
                    Location start_location = this->current_location;
                    
                    while (std::isdigit(this->current_char())) {
                        this->advance_char();
                    }
                    
                    size_t end_pointer = this->source_pointer;
                    std::string int_literal_string = this->source.substr(start_pointer, end_pointer-start_pointer);
                    return Token(TokenType::INT_LITERAL, int_literal_string, start_location);
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
    BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, const Token& operator_token) :
        Expression(left->get_location()),
        left(std::move(left)),
        right(std::move(right)),
        operator_token(operator_token)
    {}
    
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

class VariableExpression : public Expression {
private:
    Token variable_name;
public:
    VariableExpression(const Token& variable_name)
        : Expression(variable_name.get_location()), variable_name(variable_name) 
    {}
    
    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        for (size_t i = 0; i < layer; i++) {
            output_stream << "\t";
        }
        output_stream << "VariableExpression(" << this->variable_name.get_text() << ")" << std::endl;
    }

    ~VariableExpression() {}
};

class CallExpression : public Expression {
private:
    std::unique_ptr<Expression> called;
    std::vector<std::unique_ptr<Expression>> arguments;
public:
    CallExpression(std::unique_ptr<Expression> called, std::vector<std::unique_ptr<Expression>>& arguments)
        : Expression(called->get_location()), called(std::move(called)), arguments(std::move(arguments)) 
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        for (size_t i = 0; i < layer; i++) {
            output_stream << "\t";
        }
        output_stream << "CallExpression" << std::endl;
        this->called->append_to_output_stream(output_stream, layer+1);
        for (const auto& argument : this->arguments) {
            argument->append_to_output_stream(output_stream, layer+2);
        }
    }

    ~CallExpression() {}
};

class UnaryExpression : public Expression {
private:
    Token operator_token;
    std::unique_ptr<Expression> operand;
public:
    UnaryExpression(const Token& operator_token, std::unique_ptr<Expression> operand)
        : Expression(operator_token.get_location()), operator_token(operator_token), operand(std::move(operand))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        for (size_t i = 0; i < layer; i++) {
            output_stream << "\t";
        }
        output_stream << "UnaryExpression(" << this->operator_token.get_text() << ")" << std::endl;
        this->operand->append_to_output_stream(output_stream, layer+1);
    }

    ~UnaryExpression() {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t token_pointer;
    
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

    // Refer to https://en.cppreference.com/w/c/language/operator_precedence
    // Precedences are handled as:
    //      precedence(+) > precedence(*)
    //      precedence(SOMETHING_THAT_IS_NOT_AN_OPERATOR) = 0
    int get_binary_precedence(TokenType token_type) {
        switch (token_type) {
            case TokenType::STAR:
            case TokenType::SLASH:
            case TokenType::PERCENT:
                return 3;
            case TokenType::PLUS:
            case TokenType::MINUS:
                return 4;
            case TokenType::LESS_LESS:
            case TokenType::GREATER_GREATER:
                return 5;
            case TokenType::LESS:
            case TokenType::GREATER:
            case TokenType::LESS_EQUAL:
            case TokenType::GREATER_EQUAL:
                return 6;
            case TokenType::EQUAL_EQUAL:
            case TokenType::BANG_EQUAL:
                return 7;
            case TokenType::AND:
                return 8;
            case TokenType::HAT:
                return 9;
            case TokenType::PIPE:
                return 10;
            case TokenType::AND_AND:
                return 11;
            case TokenType::PIPE_PIPE:
                return 12;
            case TokenType::EQUAL:
                return 13;
            default:
                return 0; // Not an operator
        }
    }
public:
    Parser(std::vector<Token> tokens)
        : tokens(tokens), token_pointer(0)
    {}


    std::unique_ptr<Expression> parse_expression() {
        return this->parse_binary_expression();
    }

private:
    std::unique_ptr<Expression> parse_binary_expression(int parent_precedence=-1) {
        auto left = this->parse_primary_expression();
        Token next_operator = this->get_current_token();
        int operator_precedence = this->get_binary_precedence(next_operator.get_type());
        while (
                (parent_precedence == -1 && operator_precedence != 0) || // A parent precedence of -1 indicates that we are at the top level of a binary expression
                (0 < operator_precedence && operator_precedence < parent_precedence)
        ) {
            (void)this->consume_token();
            auto right = this->parse_binary_expression(operator_precedence);
            left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), next_operator);
            next_operator = this->get_current_token();
            operator_precedence = this->get_binary_precedence(next_operator.get_type());
        }
        return left;
    }

    std::unique_ptr<Expression> parse_unary_expression() {
        Token current_token = this->get_current_token();
        switch (current_token.get_type()) {
            // Unary operators
            case TokenType::TILDE:
            case TokenType::BANG:
            case TokenType::MINUS:
            case TokenType::PLUS: {
                Token operator_token = this->consume_token();
                auto operand = this->parse_unary_expression();
                return std::make_unique<UnaryExpression>(operator_token, std::move(operand));
            }
            default:
                return this->parse_primary_expression();
        }
    }

    std::unique_ptr<Expression> parse_primary_expression() {
        Token current_token = this->get_current_token();
        std::unique_ptr<Expression> left;
        switch (current_token.get_type()) {
            case TokenType::TRUE_KEYWORD:
            case TokenType::FALSE_KEYWORD:
            case TokenType::INT_LITERAL:
                left = std::make_unique<LiteralExpression>(this->consume_token());
                break;
            case TokenType::OPEN_PARENTHESIS: {
                (void)this->consume_token();
                auto inner_expression = this->parse_expression();
                this->expect_token(TokenType::CLOSE_PARENTHESIS);
                left = std::move(inner_expression);
                break;
            }
            case TokenType::NAME: {
                left = std::make_unique<VariableExpression>(this->consume_token());
                break;
            }
            default:
                std::cerr << current_token.get_location() << ": PARSE_ERROR: Unexpected token of type <" << current_token.get_type() << "> at the beginning of a primary expression." << std::endl;
                std::exit(1);
        }

        for (;;) {
            current_token = this->get_current_token();
            if (current_token.get_type() == TokenType::OPEN_PARENTHESIS) {
                (void)this->consume_token();
                std::vector<std::unique_ptr<Expression>> arguments;
                if (this->get_current_token().get_type() != TokenType::CLOSE_PARENTHESIS) {
                    for (;;) {
                        auto argument_expression = this->parse_expression();
                        arguments.push_back(std::move(argument_expression));
                        if (this->get_current_token().get_type() == TokenType::COMMA) {
                            (void)this->consume_token();
                        } else {
                            break;
                        }
                    }
                }
                this->expect_token(TokenType::CLOSE_PARENTHESIS);
                left = std::make_unique<CallExpression>(std::move(left), arguments);
            } else {
                return left;
            }
        }
    }

public:
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
