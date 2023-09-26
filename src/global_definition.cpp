class GlobalDefinition {
private:
    Location location;
public:
    GlobalDefinition(const Location& location) : location(location) {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

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
public:
    ArgumentDefinition(const Token& name, std::unique_ptr<TypeAnnotation> type)
        : name(name), type(std::move(type))
    {}
    
    const std::unique_ptr<TypeAnnotation>& get_type() const {
        return this->type;
    }


    const Token& get_name() const {
        return this->name;
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

    ~FunctionDefinition() {}
};
