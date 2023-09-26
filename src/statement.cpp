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
    DefinitionStatement(const Location& start_location, const Token& variable_name, std::unique_ptr<Expression> defining_expression)
        : Statement(start_location), variable_name(variable_name), defining_expression(std::move(defining_expression))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "DefinitionStatement(" << variable_name.get_text() << ")" << std::endl;
        this->defining_expression->append_to_output_stream(output_stream, layer + 1);
    }

    ~DefinitionStatement() {}
};

class TypedDefinitionStatement : public Statement {
private:
    Token variable_name;
    std::unique_ptr<TypeAnnotation> type_annotation;
    std::unique_ptr<Expression> defining_expression;
public:
    TypedDefinitionStatement(const Location& start_location, const Token& variable_name, std::unique_ptr<TypeAnnotation> type_annotation, std::unique_ptr<Expression> defining_expression)
        : Statement(start_location),
          variable_name(variable_name),
          type_annotation(std::move(type_annotation)),
          defining_expression(std::move(defining_expression))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "TypedDefinitionStatement(" << this->variable_name.get_text() << " : " << this->type_annotation->to_string() << ")" << std::endl;
        this->defining_expression->append_to_output_stream(output_stream, layer + 1);
    }

    ~TypedDefinitionStatement() {}
};

class BlockStatement : public Statement {
private:
    std::vector<std::unique_ptr<Statement>> sub_statements;
public:
    BlockStatement(const Location& start_location, std::vector<std::unique_ptr<Statement>> sub_statements)
        : Statement(start_location), sub_statements(std::move(sub_statements))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "BlockStatement" << std::endl;
        
        for (size_t i = 0; i < this->sub_statements.size(); i++) {
            this->sub_statements[i]->append_to_output_stream(output_stream, layer + 1);
        }
    }

    ~BlockStatement() {}
};

class IfStatement : public Statement {
private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
public:
    IfStatement(const Location& start_location, std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : Statement(start_location), condition(std::move(condition)), body(std::move(body))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "IfStatement" << std::endl;
        this->condition->append_to_output_stream(output_stream, layer + 1);
        this->body->append_to_output_stream(output_stream, layer + 1);
    }

    ~IfStatement() {}
};

class ElifStatement : public Statement {
private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> then_body;
    std::unique_ptr<Statement> else_body;
public:
    ElifStatement(const Location& start_location, std::unique_ptr<Expression> condition, std::unique_ptr<Statement> then_body, std::unique_ptr<Statement> else_body)
        : Statement(start_location), condition(std::move(condition)), then_body(std::move(then_body)), else_body(std::move(else_body))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ElifStatement" << std::endl;
        this->condition->append_to_output_stream(output_stream, layer + 1);
        this->then_body->append_to_output_stream(output_stream, layer + 1);
        this->else_body->append_to_output_stream(output_stream, layer + 1);
    }

    ~ElifStatement() {}
};

class WhileStatement : public Statement {
private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
public:
    WhileStatement(const Location& start_location, std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : Statement(start_location), condition(std::move(condition)), body(std::move(body))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "WhileStatement" << std::endl;
        this->condition->append_to_output_stream(output_stream, layer + 1);
        this->body->append_to_output_stream(output_stream, layer + 1);
    }
    
    ~WhileStatement() {}
};

class BreakStatement : public Statement {
public:
    BreakStatement(const Location& start_location) : Statement(start_location) {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "BreakStatement" << std::endl;
    }

    ~BreakStatement() {}
};

class ContinueStatement : public Statement {
public:
    ContinueStatement(const Location& start_location) : Statement(start_location) {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ContinueStatement" << std::endl;
    }

    ~ContinueStatement() {}
};

