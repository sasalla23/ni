
class Expression {
private:
    Location location;
    std::shared_ptr<Type> type;

protected:
    void set_type(std::shared_ptr<Type> type) {
        this->type = type;
    }

public:
    Expression(const Location& location)
        : location(location),
          type(Type::NO)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    virtual void type_check(TypeChecker& type_checker) = 0;

    virtual std::shared_ptr<Type> get_type() const {
        return this->type;
    }

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
        indent_layer(output_stream, layer);
        output_stream << "BinaryExpression(" << this->operator_token.get_text() << ")" << std::endl;
        this->left->append_to_output_stream(output_stream, layer+1);
        this->right->append_to_output_stream(output_stream, layer+1);
    }

    virtual void type_check(TypeChecker& type_checker) {
        this->left->type_check(type_checker);
        this->right->type_check(type_checker);
        
        auto left_type = this->left->get_type();
        auto right_type = this->right->get_type();
        
        for (size_t i = 0; i < BINARY_OPERATOR_COUNT; i++) {
            const BinaryOperator& binary_operator = BinaryOperator::OPERATORS[i];
            if (binary_operator.fits_criteria(this->operator_token.get_type(), left_type, right_type)) {
                this->set_type(binary_operator.get_return_type());
                return;
            }
        }
        
        std::cerr << this->get_location() << ": Operator '" << this->operator_token.get_text() << "' is not defined for types <" << left_type->to_string() << "> and <" << right_type->to_string() << ">." << std::endl;
        std::exit(1);
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
        indent_layer(output_stream, layer);
        output_stream << "LiteralExpression(" << this->literal_token.get_text() << ")" << std::endl;
    }
    
    virtual void type_check(TypeChecker&) override {
        switch (literal_token.get_type()) {
            case TokenType::INT_LITERAL:
                this->set_type(Type::INT);
                break;
            
            case TokenType::STRING_LITERAL:
                this->set_type(Type::STRING);
                break;

            case TokenType::CHAR_LITERAL:
                this->set_type(Type::CHAR);
                break;
            
            case TokenType::FLOAT_LITERAL:
                this->set_type(Type::FLOAT);
                break;
            
            case TokenType::FALSE_KEYWORD:
            case TokenType::TRUE_KEYWORD:
                this->set_type(Type::BOOL);
                break;

            default:
                assert(false && "unreachable");
        }
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
        indent_layer(output_stream, layer);
        output_stream << "VariableExpression(" << this->variable_name.get_text() << ")" << std::endl;
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
    }

    ~VariableExpression() {}
};

class CallExpression : public Expression {
private:
    std::unique_ptr<Expression> called;
    std::vector<std::unique_ptr<Expression>> arguments;
public:
    CallExpression(std::unique_ptr<Expression> called, std::vector<std::unique_ptr<Expression>> arguments)
        : Expression(called->get_location()), called(std::move(called)), arguments(std::move(arguments)) 
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "CallExpression" << std::endl;
        this->called->append_to_output_stream(output_stream, layer+1);
        for (const auto& argument : this->arguments) {
            argument->append_to_output_stream(output_stream, layer+1);
        }
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
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
        indent_layer(output_stream, layer);
        output_stream << "UnaryExpression(" << this->operator_token.get_text() << ")" << std::endl;
        this->operand->append_to_output_stream(output_stream, layer+1);
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
    }

    ~UnaryExpression() {}
};

class ListLiteralExpression : public Expression {
private:
    std::vector<std::unique_ptr<Expression>> element_initializers;
public:
    ListLiteralExpression(const Location& start_location, std::vector<std::unique_ptr<Expression>> element_initializers)
        : Expression(start_location), element_initializers(std::move(element_initializers))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ListLiteralExpression" << std::endl;
        for (size_t i = 0; i < this->element_initializers.size(); i++) {
            this->element_initializers[i]->append_to_output_stream(output_stream, layer + 1);
        }
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
    }

    ~ListLiteralExpression() {}
};

class IndexingExpression : public Expression {
private:
    std::unique_ptr<Expression> operand;
    std::unique_ptr<Expression> index;
public:
    IndexingExpression(std::unique_ptr<Expression> operand, std::unique_ptr<Expression> index)
        : Expression(operand->get_location()), operand(std::move(operand)), index(std::move(index))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "IndexingExpression" << std::endl;
        this->operand->append_to_output_stream(output_stream, layer + 1);
        this->index->append_to_output_stream(output_stream, layer + 1);
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
    }

    ~IndexingExpression() {}
};

class MemberAccessExpression : public Expression {
private:
    std::unique_ptr<Expression> accessed;
    Token member_name;
public:
    MemberAccessExpression(std::unique_ptr<Expression> accessed, const Token& member_name)
        : Expression(accessed->get_location()), accessed(std::move(accessed)), member_name(member_name)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "MemberAccessExpression(" << member_name.get_text() << ")" << std::endl;
        this->accessed->append_to_output_stream(output_stream, layer + 1);
    }
    
    virtual void type_check(TypeChecker&) override {
        assert(false && "TODO");
    }

    ~MemberAccessExpression() {}
};
