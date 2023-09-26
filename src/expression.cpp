
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
        indent_layer(output_stream, layer);
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
        indent_layer(output_stream, layer);
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
        indent_layer(output_stream, layer);
        output_stream << "VariableExpression(" << this->variable_name.get_text() << ")" << std::endl;
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

    ~MemberAccessExpression() {}
};
