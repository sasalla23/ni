class Statement {
private:
    Location location;
public:
    Statement(const Location& location)
        : location(location)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    virtual void type_check(TypeChecker&) = 0; 
    virtual bool is_definite_return(std::shared_ptr<Type>) const = 0;

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

    // TODO: Maybe add warning if output type is non void?
    virtual void type_check(TypeChecker& type_checker) override {
        this->expression->type_check(type_checker);
    }

    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false;
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

    virtual void type_check(TypeChecker& type_checker) override {
        const std::string& name_string = this->variable_name.get_text();
        if (type_checker.symbol_exists(name_string)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Symbol '" << name_string << "' already exists." << std::endl;
            std::exit(1);
        }
        this->defining_expression->type_check(type_checker);
        type_checker.add_variable_symbol(name_string, this->defining_expression->get_type());
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false;
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

    virtual void type_check(TypeChecker& type_checker) override {
        const std::string& name_string = this->variable_name.get_text();

        if (type_checker.symbol_exists(name_string)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Symbol '" << name_string << "' already exists." << std::endl;
            std::exit(1);
        }

        this->defining_expression->type_check(type_checker);
        
        auto variable_type = this->defining_expression->get_type();
        auto annotated_type = this->type_annotation->to_type();

        if (!variable_type->fits(annotated_type)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Type of defining expression <"
                << variable_type->to_string()
                << "> for variable '" << name_string << "' does not fit annotated type <"
                << annotated_type->to_string() << ">.";
            std::exit(1);
        }

        type_checker.add_variable_symbol(name_string, this->defining_expression->get_type());
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false;
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
    virtual void type_check(TypeChecker& type_checker) override {
        type_checker.push_scope();

        for (auto& statement : this->sub_statements) {
            statement->type_check(type_checker);
        }

        type_checker.pop_scope();
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type> return_type) const override {
        for (size_t j = sub_statements.size(); j >= 1; j--) {
            size_t i = j - 1;
            // TODO: Print warning if there are statements below a definite return
            if (this->sub_statements[i]->is_definite_return(return_type)) {
                return true;
            }
        }
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->condition->type_check(type_checker);
        auto condition_type = this->condition->get_type();
        
        if (!condition_type->fits(Type::BOOL)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Condition of if statement must be a boolean, instead got <" << condition_type->to_string() << ">." << std::endl; 
            std::exit(1);
        }

        auto as_definition_statement = dynamic_cast<DefinitionStatement *>(this->body.get());
        auto as_typed_definition_statement = dynamic_cast<TypedDefinitionStatement *>(this->body.get());

        if (as_definition_statement != nullptr || as_typed_definition_statement != nullptr)  {
            std::cerr << this->get_location() << ": TYPE_ERROR: Body of if statement cannot be a definition" << std::endl;
            std::exit(1);
        }

        this->body->type_check(type_checker);
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->condition->type_check(type_checker);
        auto condition_type = this->condition->get_type();
        
        if (!condition_type->fits(Type::BOOL)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Condition of if statement must be a boolean, instead got <" << condition_type->to_string() << ">." << std::endl; 
            std::exit(1);
        }

        auto as_definition_statement = dynamic_cast<DefinitionStatement *>(this->then_body.get());
        auto as_typed_definition_statement = dynamic_cast<TypedDefinitionStatement *>(this->then_body.get());

        if (as_definition_statement != nullptr || as_typed_definition_statement != nullptr)  {
            std::cerr << this->get_location() << ": TYPE_ERROR: Body of if statement cannot be a definition" << std::endl;
            std::exit(1);
        }
        
        as_definition_statement = dynamic_cast<DefinitionStatement *>(this->else_body.get());
        as_typed_definition_statement = dynamic_cast<TypedDefinitionStatement *>(this->else_body.get());

        if (as_definition_statement != nullptr || as_typed_definition_statement != nullptr)  {
            std::cerr << this->get_location() << ": TYPE_ERROR: Body of if statement cannot be a definition" << std::endl;
            std::exit(1);
        }

        this->then_body->type_check(type_checker);
        this->else_body->type_check(type_checker);
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type> return_type) const override {
        return this->then_body->is_definite_return(return_type) && this->else_body->is_definite_return(return_type);
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->condition->type_check(type_checker);
        auto condition_type = this->condition->get_type();
        
        if (!condition_type->fits(Type::BOOL)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Condition of while statement must be a boolean, instead got <" << condition_type->to_string() << ">." << std::endl; 
            std::exit(1);
        }

        type_checker.push_while_statement();

        this->body->type_check(type_checker);

        type_checker.pop_while_statement();
    }


    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false; 
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        if (!type_checker.is_in_while_statement()) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Break statements are not allowed outside of while statements." << std::endl;
            std::exit(1);
        }
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false; 
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        if (!type_checker.is_in_while_statement()) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Continue statements are not allowed outside of while statements." << std::endl;
            std::exit(1);
        }
    }

    virtual bool is_definite_return(std::shared_ptr<Type>) const override {
        return false; 
    }

    ~ContinueStatement() {}
};

class ReturnStatement : public Statement {
private:
    std::unique_ptr<Expression> return_value;
public:
    ReturnStatement(const Location& start_location, std::unique_ptr<Expression> return_value)
        : Statement(start_location), return_value(std::move(return_value))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ReturnStatement" << std::endl;
        this->return_value->append_to_output_stream(output_stream, layer + 1);
    }

    virtual void type_check(TypeChecker& type_checker) override {
        this->return_value->type_check(type_checker);
    }
    
    virtual bool is_definite_return(std::shared_ptr<Type> return_type) const override {
        return this->return_value->get_type()->fits(return_type); 
    }
    
    ~ReturnStatement() {}
};

class VoidReturnStatement : public Statement {
public:
    VoidReturnStatement(const Location& start_location)
        : Statement(start_location)
    {}
    
    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "ReturnStatement" << std::endl;
    }
    
    virtual void type_check(TypeChecker&) override {}
    
    virtual bool is_definite_return(std::shared_ptr<Type> return_type) const override {
        return Type::VOID->fits(return_type);
    }

    ~VoidReturnStatement() {}
};
