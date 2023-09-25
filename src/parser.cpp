
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
    Parser(std::vector<Token>& tokens)
        : tokens(std::move(tokens)), token_pointer(0)
    {}


    std::unique_ptr<Expression> parse_expression() {
        return this->parse_binary_expression();
    }

    std::unique_ptr<Statement> parse_statement() {
        switch (this->get_current_token().get_type()) {
            case TokenType::VAR_KEYWORD:
                {
                    Token var_token = this->consume_token();
                    Token variable_name = this->expect_token(TokenType::NAME);
                    (void) this->expect_token(TokenType::EQUAL);
                    auto defining_expression = this->parse_expression();
                    (void) this->expect_token(TokenType::SEMI_COLON);
                    return std::make_unique<DefinitionStatement>(var_token.get_location(), variable_name, std::move(defining_expression));
                }

            case TokenType::OPEN_CURLY_BRACE:
                {
                    Token open_curly_brace_token = this->consume_token();
                    std::vector<std::unique_ptr<Statement>> sub_statements;
                    while (this->get_current_token().get_type() != TokenType::CLOSE_CURLY_BRACE) {
                        auto statement = this->parse_statement();
                        sub_statements.push_back(std::move(statement));
                    }
                    (void) this->expect_token(TokenType::CLOSE_CURLY_BRACE);
                    return std::make_unique<BlockStatement>(open_curly_brace_token.get_location(), sub_statements);
                }

            case TokenType::IF_KEYWORD:
                {
                    Token if_keyword_token = this->consume_token();
                    (void) this->expect_token(TokenType::OPEN_PARENTHESIS);
                    auto condition = this->parse_expression();
                    (void) this->expect_token(TokenType::CLOSE_PARENTHESIS);
                    auto then_body = this->parse_statement();
                    if (this->get_current_token().get_type() == TokenType::ELSE_KEYWORD) {
                        (void) this->consume_token();
                        auto else_body = this->parse_statement();
                        return std::make_unique<ElifStatement>(if_keyword_token.get_location(), std::move(condition), std::move(then_body), std::move(else_body));
                    } else {
                        return std::make_unique<IfStatement>(if_keyword_token.get_location(), std::move(condition), std::move(then_body));
                    }
                }
            
            case TokenType::WHILE_KEYWORD:
                {
                    Token while_keyword_token = this->consume_token();
                    (void) this->expect_token(TokenType::OPEN_PARENTHESIS);
                    auto condition = this->parse_expression();
                    (void) this->expect_token(TokenType::CLOSE_PARENTHESIS);
                    auto body = this->parse_statement();
                    return std::make_unique<WhileStatement>(while_keyword_token.get_location(), std::move(condition), std::move(body));
                }

            default:
                {
                    auto expression = this->parse_expression();
                    this->expect_token(TokenType::SEMI_COLON);
                    return std::make_unique<ExpressionStatement>(std::move(expression));
                }
            
        }
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
            case TokenType::FLOAT_LITERAL:
            case TokenType::STRING_LITERAL:
                left = std::make_unique<LiteralExpression>(this->consume_token());
                break;
            case TokenType::OPEN_PARENTHESIS:
                {
                    (void)this->consume_token();
                    auto inner_expression = this->parse_expression();
                    this->expect_token(TokenType::CLOSE_PARENTHESIS);
                    left = std::move(inner_expression);
                    break;
                }
            case TokenType::NAME:
                left = std::make_unique<VariableExpression>(this->consume_token());
                break;
            default:
                std::cerr << current_token.get_location() << ": PARSE_ERROR: Unexpected token of type <" << current_token.get_type() << "> at the beginning of a primary expression." << std::endl;
                std::exit(1);
        }

        // Parse suffixes like ...(...) or ...[...]
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

