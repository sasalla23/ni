class Statement {
private:
    Location location;
public:
    Statement(const Location& location)
        : location(location)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    const Location& get_location() const {
        return this->location;
    }

    virtual ~Statement() {}
};

std::ostream& operator<<(std::ostream& output_stream, const Statement& statement) {
    statement.append_to_output_stream(output_stream);
    return output_stream;
}

class ExpressionStatement : public Statement {
private:
    std::unique_ptr<Expression> expression;
public:
    ExpressionStatement(std::unique_ptr<Expression> expression)
        : Statement(expression->get_location()), expression(std::move(expression))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ExpressionStatement" << std::endl;
        this->expression->append_to_output_stream(output_stream, layer + 1);
    }

    ~ExpressionStatement() {}
};

class DefinitionStatement : public Statement {
private:
    Token variable_name;
    std::unique_ptr<Expression> defining_expression;
public:
    DefinitionStatement(const Location& start_location, Token variable_name, std::unique_ptr<Expression> defining_expression)
        : Statement(start_location), variable_name(variable_name), defining_expression(std::move(defining_expression))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "DefinitionStatement(" << variable_name.get_text() << ")" << std::endl;
        this->defining_expression->append_to_output_stream(output_stream, layer + 1);
    }

    ~DefinitionStatement() {}
};

class BlockStatement : public Statement {
private:
    std::vector<std::unique_ptr<Statement>> sub_statements;
public:
    BlockStatement(Location start_location, std::vector<std::unique_ptr<Statement>>& sub_statements)
        : Statement(start_location), sub_statements(std::move(sub_statements))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "BlockStatement" << std::endl;
        
        for (size_t i = 0; i < this->sub_statements.size(); i++) {
            this->sub_statements[i]->append_to_output_stream(output_stream, layer + 1);
        }
    }
};

// TODO: BlockStatement, IfStatement, WhileStatement

