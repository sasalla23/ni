class GlobalDefinition {
private:
    Location location;
public:
    GlobalDefinition(const Location& location) : location(location) {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    virtual void type_check(TypeChecker& type_checker) = 0;

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
public:
    FunctionDefinition(const Location& start_location, const Token& name, std::vector<std::unique_ptr<ArgumentDefinition>> arguments, std::unique_ptr<TypeAnnotation> return_type, std::unique_ptr<Statement> body)
        : GlobalDefinition(start_location), name(name), arguments(std::move(arguments)), return_type(std::move(return_type)), body(std::move(body))
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

    virtual void type_check(TypeChecker& type_checker) override {
        const std::string& function_name = this->name.get_text();

        if (type_checker.symbol_exists(function_name)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Symbol '" << function_name << "' already exists." << std::endl;
            std::exit(1);
        }

        auto parsed_return_type = this->return_type->to_type();

        type_checker.push_scope();
        std::vector<std::shared_ptr<Type>> argument_types;

        for (const auto& argument : this->arguments) {
            auto argument_type = argument->get_type()->to_type();
            auto argument_name = argument->get_name().get_text();

            if (type_checker.symbol_exists(argument_name)) {
                std::cerr << argument->get_location() << ": TYPE_ERROR: Symbol '" << argument_name << "' already exists." << std::endl;
                std::exit(1);
            }

            type_checker.add_variable_symbol(argument_name, argument_type);
            argument_types.push_back(argument_type);
        }

        this->body->type_check(type_checker);

        if (!parsed_return_type->fits(Type::VOID) && !this->body->is_definite_return(parsed_return_type)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Function '" << function_name << "' does not definitely return a value." << std::endl;
            std::exit(1);
        }

        type_checker.pop_scope();
        type_checker.add_function_symbol(function_name, parsed_return_type, std::move(argument_types));
    }

    ~FunctionDefinition() {}
};
