class GlobalDefinition {
private:
    Location location;
public:
    GlobalDefinition(const Location& location) : location(location) {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    virtual void type_check(TypeChecker& type_checker) = 0;
    virtual void first_pass(TypeChecker& type_checker) = 0;
    virtual void emit(CodeGenerator&) const = 0;

    const Location& get_location() const {
        return this->location;
    }

    virtual ~GlobalDefinition() {};
};

std::ostream& operator<<(std::ostream& output_stream, const GlobalDefinition& global_definition) {
    global_definition.append_to_output_stream(output_stream);
    return output_stream;
}

class ArgumentDefinition {
private:
    Token name;
    std::unique_ptr<TypeAnnotation> type;
    Location location;
public:
    ArgumentDefinition(const Token& name, std::unique_ptr<TypeAnnotation> type)
        : name(name), type(std::move(type)), location(name.get_location())
    {}
    
    const std::unique_ptr<TypeAnnotation>& get_type() const {
        return this->type;
    }

    const Token& get_name() const {
        return this->name;
    }

    const Location& get_location() const {
        return this->location;
    }

    ~ArgumentDefinition() {}
};

std::ostream& operator<<(std::ostream& output_stream, const ArgumentDefinition& argument_definition) {
    return output_stream << argument_definition.get_name().get_text() << " : " << argument_definition.get_type()->to_string();
}

class FunctionDefinition : public GlobalDefinition {
private:
    Token name;
    std::vector<std::unique_ptr<ArgumentDefinition>> arguments;
    std::unique_ptr<TypeAnnotation> return_type;
    std::unique_ptr<Statement> body;
    size_t id;
public:
    FunctionDefinition(const Location& start_location, const Token& name, std::vector<std::unique_ptr<ArgumentDefinition>> arguments, std::unique_ptr<TypeAnnotation> return_type, std::unique_ptr<Statement> body)
        : GlobalDefinition(start_location), name(name), arguments(std::move(arguments)), return_type(std::move(return_type)), body(std::move(body)), id(0)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "FunctionDefinition(" << this->name.get_text() << ")" << std::endl;
        for (auto& argument : this->arguments) {
            indent_layer(output_stream, layer + 1);
            output_stream << *argument << std::endl;
        }
        indent_layer(output_stream, layer+1);
        output_stream << return_type->to_string() << std::endl;
        this->body->append_to_output_stream(output_stream, layer + 1);
    }

    virtual void first_pass(TypeChecker& type_checker) override {
        const std::string& function_name = this->name.get_text();

        if (type_checker.symbol_exists(function_name)) {
            TYPE_ERROR("Symbol '" << function_name << "' already exists.");
        }

        auto parsed_return_type = this->return_type->to_type();
        std::vector<std::shared_ptr<Type>> argument_types;
        
        for (const auto& argument : this->arguments) {
            auto argument_type = argument->get_type()->to_type();
            argument_types.push_back(argument_type);
        }
        
        this->id = type_checker.add_function_symbol(function_name, parsed_return_type, std::move(argument_types));
    }

    virtual void type_check(TypeChecker& type_checker) override {
        // TODO: somehow save the result from the first pass
        auto parsed_return_type = this->return_type->to_type();
        const std::string& function_name = this->name.get_text();
        type_checker.set_current_return_type(parsed_return_type);

        type_checker.push_scope();

        for (const auto& argument : this->arguments) {
            auto argument_type = argument->get_type()->to_type();
            auto argument_name = argument->get_name().get_text();

            if (type_checker.symbol_exists(argument_name)) {
                TYPE_ERROR("Symbol '" << argument_name << "' already exists.");
            }

            type_checker.add_variable_symbol(argument_name, argument_type);
        }

        this->body->type_check(type_checker);

        if (!parsed_return_type->fits(Type::VOID) && !this->body->is_definite_return()) {
            TYPE_ERROR("Function '" << function_name << "' does not definitely return a value.");
        }

        type_checker.pop_scope();
    }

    virtual void emit(CodeGenerator& code_generator) const override {
        bool is_main = this->name.get_text() == "main";
        if (is_main) {
            code_generator.set_main_label(this->id);
        }
        INT_INST(LABEL, this->id);
        for (size_t i = 0; i < this->arguments.size(); i++) {
            size_t id = this->arguments.size() - (i+1);
            INT_INST(VWRITE, id);
        }
        this->body->emit(code_generator);
        // TODO: do this only if necessary
        if (is_main) {
            INST(HALT);
        } else {
            INST(RET);
        }
    }

    ~FunctionDefinition() {}
};
