
class Expression {
private:
    Location location;
    std::shared_ptr<Type> type;

public:
    Expression(const Location& location)
        : location(location),
          type(Type::NO)
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const = 0;

    virtual void type_check(TypeChecker& type_checker) = 0;
    virtual bool is_lvalue() const = 0;
    virtual void emit(CodeGenerator&) const = 0;
    virtual void emit_condition(CodeGenerator& code_generator, size_t jump_if_false, size_t jump_if_true) const = 0 ;

    std::shared_ptr<Type> get_type() const {
        return this->type;
    }
    
    void set_type(std::shared_ptr<Type> type) {
        this->type = type;
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

    //bool is_comparison_operator() const {
    //    switch (this->operator_token->get_type()) {
    //        case TokenType::LESS:
    //        case TokenType::LESS_EQUAL:
    //        case TokenType::GREATER:
    //        case TokenType::GREATER_EQUAL:
    //        case TokenType::EQUAL_EQUAL:
    //        case TokenType::BANG_EQUAL:
    //            return true;
    //        default:
    //            return false;
    //    }
    //}

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

        if (this->operator_token.get_type() == TokenType::EQUAL) { // Assignment
            if (!this->left->is_lvalue()) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Left expression of assignment is not assignable." << std::endl;
                std::exit(1);
            }

            if (!right_type->fits(left_type)) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Cannot assign value of type <" << right_type->to_string() << "> to expression of type <" << left_type->to_string() << ">." << std::endl;
                std::exit(1);
            }

            // This is questionable
            this->set_type(left_type);
        } else { // Regular Operator
            for (size_t i = 0; i < BINARY_OPERATOR_COUNT; i++) {
                const BinaryOperator& binary_operator = BinaryOperator::OPERATORS[i];
                if (binary_operator.fits_criteria(this->operator_token.get_type(), left_type, right_type)) {
                    this->set_type(binary_operator.get_return_type());
                    return;
                }
            }
            
            std::cerr <<
                this->get_location() <<
                ": TYPE_ERROR: Operator '" <<
                this->operator_token.get_text() <<
                "' is not defined for types <" <<
                left_type->to_string() <<
                "> and <" <<
                right_type->to_string() << ">." << std::endl;
            std::exit(1);
        }
    }

    virtual void emit(CodeGenerator& code_generator) const override {

        if (this->operator_token.get_type() == TokenType::EQUAL) {
            assert(false && "TODO");
        } else if (this->get_type()->fits(Type::BOOL)) {
            size_t false_label = code_generator.generate_label();
            size_t true_label = code_generator.generate_label();
            size_t end_label = code_generator.generate_label();
            this->emit_condition(code_generator, false_label, true_label);
            
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) true_label }));
            code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 1 }));
            code_generator.push_instruction(Instruction(InstructionType::JUMP, Word { .as_int = (int64_t) end_label }));
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) false_label }));
            code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 0 }));
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) end_label }));
        } else {
            this->left->emit(code_generator);
            this->right->emit(code_generator);
            auto left_type = this->left->get_type();

            if (left_type->fits(Type::INT)) {
                switch (this->operator_token.get_type()) {
                    case TokenType::PLUS:
                        code_generator.push_instruction(Instruction(InstructionType::IADD));
                        break;
                    case TokenType::MINUS:
                        code_generator.push_instruction(Instruction(InstructionType::ISUB));
                        break;
                    case TokenType::STAR:
                        code_generator.push_instruction(Instruction(InstructionType::IMUL));
                        break;
                    case TokenType::SLASH:
                        code_generator.push_instruction(Instruction(InstructionType::IDIV));
                        break;

                    case TokenType::LESS_LESS:
                        code_generator.push_instruction(Instruction(InstructionType::ISHL));
                        break;
                    case TokenType::GREATER_GREATER:
                        code_generator.push_instruction(Instruction(InstructionType::ISHR));
                        break;
                    case TokenType::AND:
                        code_generator.push_instruction(Instruction(InstructionType::IAND));
                        break;
                    case TokenType::PIPE:
                        code_generator.push_instruction(Instruction(InstructionType::IOR));
                        break;
                    case TokenType::HAT:
                        code_generator.push_instruction(Instruction(InstructionType::IXOR));
                        break;

                    case TokenType::PERCENT:
                        code_generator.push_instruction(Instruction(InstructionType::IMOD));
                        break;

                    default:
                        assert(false && "not implemented");
                }
            } else if (left_type->fits(Type::FLOAT)) {
                switch (this->operator_token.get_type()) {
                    case TokenType::PLUS:
                        code_generator.push_instruction(Instruction(InstructionType::FADD));
                        break;
                    case TokenType::MINUS:
                        code_generator.push_instruction(Instruction(InstructionType::FSUB));
                        break;
                    case TokenType::STAR:
                        code_generator.push_instruction(Instruction(InstructionType::FMUL));
                        break;
                    case TokenType::SLASH:
                        code_generator.push_instruction(Instruction(InstructionType::FDIV));
                        break;
                    default:
                        assert(false && "not implemented");
                }
            }

        }
    }
    
    virtual void emit_condition(CodeGenerator& code_generator, size_t jump_if_false, size_t jump_if_true) const {
        assert(this->get_type()->fits(Type::BOOL));

        bool is_float = this->get_type()->fits(Type::FLOAT);

        switch (this->operator_token.get_type()) {

#define COMPARISON_INSTRUCTIONS(TOKEN_TYPE, INSTRUCTION_TYPE) \
            case TokenType:: TOKEN_TYPE : \
                this->left->emit(code_generator); \
                this->right->emit(code_generator); \
                code_generator.push_instruction(Instruction((INSTRUCTION_TYPE), Word { .as_int = (int64_t) jump_if_false })); \
                code_generator.push_instruction(Instruction(InstructionType::JUMP, Word { .as_int = (int64_t) jump_if_true })); \
                break;

            COMPARISON_INSTRUCTIONS(EQUAL_EQUAL, InstructionType::JNEQ)
            COMPARISON_INSTRUCTIONS(BANG_EQUAL, InstructionType::JEQ)
            COMPARISON_INSTRUCTIONS(LESS, is_float ? InstructionType::JFGE : InstructionType::JIGE)
            COMPARISON_INSTRUCTIONS(LESS_EQUAL, is_float ? InstructionType::JFGT : InstructionType::JIGT)
            COMPARISON_INSTRUCTIONS(GREATER, is_float ? InstructionType::JFLE : InstructionType::JILE)
            COMPARISON_INSTRUCTIONS(GREATER_EQUAL, is_float ? InstructionType::JFLT : InstructionType::JILT)

            case TokenType::AND_AND:
                {
                    size_t mid_label = code_generator.generate_label();
                    this->left->emit_condition(code_generator, jump_if_false, mid_label);
                    code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) mid_label }));
                    this->right->emit_condition(code_generator, jump_if_false, jump_if_true);
                    break;
                }
            case TokenType::PIPE_PIPE:
                {
                    size_t mid_label = code_generator.generate_label();
                    this->left->emit_condition(code_generator, mid_label, jump_if_true);
                    code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) mid_label }));
                    this->right->emit_condition(code_generator, jump_if_false, jump_if_true);
                    break;
                }
            default:
                assert(false && "unreachable");
        }

    }
    
    virtual bool is_lvalue() const override {
        return false;
    }

    ~BinaryExpression() {}
};

// TODO: improve error reporting
bool parse_escaped_string(const std::string& input_string, std::string& output) {
    output.reserve(input_string.size());
    std::stringstream stream(output);
    for (size_t i = 0; i < input_string.size(); i++) {
        if (input_string[i] == '\\') {
            if (i >= input_string.size() - 1)
                return false;

            i++;
            switch (input_string[i]) {
                case '\'':
                    stream << '\'';
                    break;
                case '\"':
                    stream << '\"';
                    break;
                case '\\':
                    stream << '\\';
                    break;
                case 'a':
                    stream << '\a';
                    break;
                case 'b':
                    stream << '\b';
                    break;
                case 'f':
                    stream << '\f';
                    break;
                case 'n':
                    stream << '\n';
                    break;
                case 'r':
                    stream << '\r';
                    break;
                case 't':
                    stream << '\t';
                    break;
                case 'v':
                    stream << '\v';
                    break;
                default:
                    return false;
            }
        } else {
            stream << input_string[i];
        }
    }
    output = stream.str();
    return true;
}

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
        switch (this->literal_token.get_type()) {
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
    
    virtual void emit(CodeGenerator& code_generator) const override {
        const std::string& literal_string = this->literal_token.get_text();
        switch (this->literal_token.get_type()) {
            case TokenType::INT_LITERAL:
                {
                    int64_t parsed;
                    try {
                        parsed = std::stol(literal_string);
                    } catch(std::exception& e) {
                        std::cout << this->get_location() << ": GENERATION_ERROR: Could not parse integer literal '" << literal_string << "'." << std::endl;
                        std::exit(1);
                    }
                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = parsed }));
                }
                break;
            
            case TokenType::STRING_LITERAL:
                {
                    std::string parsed_string;
                    assert(literal_string.size() >= 2);
                    
                    if (!parse_escaped_string(literal_string.substr(1,literal_string.size()-2), parsed_string)) {
                        std::cerr << this->get_location() << ": Char literal contains invalid escape characters: " << literal_string << "." << std::endl;
                        std::exit(1);
                    }

                    // allocate string data as static memory
                    size_t static_offset = code_generator.allocate_static_objects(ObjectLayout::predefined_layouts[CHAR_LAYOUT], parsed_string.size());
                    std::memcpy(code_generator.get_static_data_pointer(static_offset), parsed_string.data(), sizeof(char) * parsed_string.size());

                    // allocate string object on the heap
                    // - first push 1 on the stack (1 string object will be allocated)
                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 1 }));
                    // - use allocate instruction to allocate string object on the heap (1 * sizeof(string) bytes)
                    code_generator.push_instruction(Instruction(InstructionType::HALLOC, Word { .as_int = STRING_LAYOUT }));
                    code_generator.push_instruction(Instruction(InstructionType::DUP));

                    // write string size into first field of string object
                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = (int64_t)parsed_string.size() }));
                    code_generator.push_instruction(Instruction(InstructionType::WRITEW));
                    
                    // duplicate pointer value because it was consumed by WRITEW
                    code_generator.push_instruction(Instruction(InstructionType::DUP));
                    // offset pointer to the data field of the string object
                    // - push offset (in this case there is one word before the data pointer)
                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = (int64_t)sizeof(Word) }));
                    // - use pointer add instruction to offset the pointer
                    code_generator.push_instruction(Instruction(InstructionType::PADD));
                    // - get the absolute pointer to the string data in the static memory
                    code_generator.push_instruction(Instruction(InstructionType::SPTR, Word { .as_int = (int64_t)static_offset }));
                    // - write the pointer address into the data field of the string object
                    code_generator.push_instruction(Instruction(InstructionType::WRITEW));
                }
                break;

            case TokenType::CHAR_LITERAL:
                {
                    std::string parsed_string;
                    assert(literal_string.size() >= 2);
                    
                    if (!parse_escaped_string(literal_string.substr(1,literal_string.size()-2), parsed_string)) {
                        std::cerr << this->get_location() << ": Char literal contains invalid escape characters: " << literal_string << "." << std::endl;
                        std::exit(1);
                    }

                    if (parsed_string.size() != 1) {
                        std::cerr << this->get_location() << ": Char literal must have exactly one character, instead got " << parsed_string.size() << "." << std::endl;
                        std::exit(1);
                    }

                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = (int64_t)parsed_string[0] }));
                }
                break;
            
            case TokenType::FLOAT_LITERAL:
                {
                    double parsed;
                    try {
                        parsed = std::stod(literal_string);
                    } catch(std::exception& e) {
                        std::cout << this->get_location() << ": GENERATION_ERROR: Could not parse float literal '" << literal_string << "'." << std::endl;
                        std::exit(1);
                    }
                    code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_float = parsed }));
                }
                break;
            
            case TokenType::FALSE_KEYWORD:
                code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 0 }));
                break;

            case TokenType::TRUE_KEYWORD:
                code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 1 }));
                break;

            default:
                assert(false && "unreachable");
        }
    }
    
    virtual void emit_condition(CodeGenerator& code_generator, size_t jump_if_false, size_t jump_if_true) const {
        assert(this->get_type()->fits(Type::BOOL));
        size_t jump_address;
        if (this->literal_token.get_type() == TokenType::FALSE_KEYWORD) {
            jump_address = jump_if_false;
        } else if (this->literal_token.get_type() == TokenType::TRUE_KEYWORD) {
            jump_address = jump_if_true;
        } else {
            assert(false && "unreachable");
        }

        code_generator.push_instruction(Instruction(InstructionType::JUMP, Word { .as_int = (int64_t) jump_address }));
    }
    
    virtual bool is_lvalue() const override {
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        const std::string& name_string = this->variable_name.get_text();
        if (!type_checker.symbol_exists(name_string)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Undefined reference to variable '" << name_string << "'." << std::endl;
            std::exit(1);
        }

        const auto& symbol = type_checker.get_symbol(name_string);
        if (symbol->get_symbol_type() != SymbolType::VARIABLE) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Symbol '" << name_string << "' is not a variable." << std::endl;
            std::exit(1);
        }

        const auto& variable_symbol = *dynamic_cast<VariableSymbol *>(symbol.get());
        this->set_type(variable_symbol.get_type());
    }

    const Token& get_variable_name() {
        return this->variable_name;
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
    
    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }
    
    virtual bool is_lvalue() const override {
        return true;
    }

    ~VariableExpression() {}
};

class MemberAccessExpression : public Expression {
friend class CallExpression;
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

    const Token& get_member_name() const {
        return this->member_name;
    }
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->accessed->type_check(type_checker);
        const auto& accessed_type = this->accessed->get_type();
        const std::string& field_name = this->member_name.get_text();
        
        if (!accessed_type->has_field(field_name)) {
            std::cerr <<
                this->get_location() <<
                ": TYPE_ERROR: Type <" <<
                accessed_type->to_string() <<
                "> does not have a field '" <<
                field_name <<
                "'." <<
                std::endl;
            std::exit(1);
        }

        this->set_type(accessed_type->get_field_type(field_name));
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
   

    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }
    
    // TODO: Maybe add notion of a constant/mutable field
    virtual bool is_lvalue() const override {
        return false;
    }

    ~MemberAccessExpression() {}
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        // We are 'abusing' the fact that dynamic_cast returns nullptr if the pointer types don't match
        auto as_regular_function_call = dynamic_cast<VariableExpression *>(this->called.get());
        auto as_method_call = dynamic_cast<MemberAccessExpression *>(this->called.get());

        auto get_function_symbol = [&](const std::string& function_name) -> const FunctionSymbol& {
            if (!type_checker.symbol_exists(function_name)) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Undefined ('" << function_name << "') is not a function." << std::endl;
                std::exit(1);
            }
            
            const auto& symbol = type_checker.get_symbol(function_name);
            if (symbol->get_symbol_type() != SymbolType::FUNCTION) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Defined ('" << function_name << "') is not a function." << std::endl;
                std::exit(1);
            }

            return *dynamic_cast<FunctionSymbol *>(symbol.get());
        };

        if (as_regular_function_call != nullptr) {
            const std::string& function_name = as_regular_function_call->get_variable_name().get_text();

            const auto& function_symbol = get_function_symbol(function_name);

            std::vector<std::shared_ptr<Type>> argument_types;
            for (auto& argument : this->arguments) {
                argument->type_check(type_checker);
                argument_types.push_back(argument->get_type());
            }
            
            if (!function_symbol.do_args_fit(argument_types)) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Arguments for function '" << function_name << "' do not fit." << std::endl;
                std::exit(1);
            }

            this->set_type(function_symbol.get_return_type());
        } else if (as_method_call != nullptr) {
            as_method_call->accessed->type_check(type_checker);
            const std::string& function_name = as_method_call->get_member_name().get_text();

            const auto& function_symbol = get_function_symbol(function_name);
            
            std::vector<std::shared_ptr<Type>> argument_types;
            argument_types.push_back(as_method_call->accessed->get_type());

            for (const auto& argument : this->arguments) {
                argument->type_check(type_checker);
                argument_types.push_back(argument->get_type());
            }
            
            if (!function_symbol.do_args_fit(argument_types)) {
                std::cerr << this->get_location() << ": TYPE_ERROR: Arguments for function '" << function_name << "' do not fit." << std::endl;
                std::exit(1);
            }

            this->set_type(function_symbol.get_return_type());

        } else {
            std::cerr << this->get_location() << ": TYPE_ERROR: The given expression is not callable." << std::endl;
            std::exit(1);
        }
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
    
    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }
    
    virtual bool is_lvalue() const override {
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->operand->type_check(type_checker);
        auto operand_type = this->operand->get_type();
        
        for (size_t i = 0; i < UNARY_OPERATOR_COUNT; i++) {
            const auto& unary_operator = UnaryOperator::OPERATORS[i];
            if (unary_operator.fits_criteria(this->operator_token.get_type(), operand_type)) {
                this->set_type(unary_operator.get_return_type());
                return;
            }
        }

        std::cerr << this->get_location() << ": TYPE_ERROR: Unary operator '"
            << this->operator_token.get_text()
            << "' is not defined for type <"
            << operand_type->to_string()
            << ">." << std::endl;
        std::exit(1);
    }
    
    virtual void emit(CodeGenerator& code_generator) const override {
        
        if (this->get_type()->fits(Type::BOOL)) {
            size_t false_label = code_generator.generate_label();
            size_t true_label = code_generator.generate_label();
            size_t end_label = code_generator.generate_label();
            this->emit_condition(code_generator, false_label, true_label);
            
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) true_label }));
            code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 1 }));
            code_generator.push_instruction(Instruction(InstructionType::JUMP, Word { .as_int = (int64_t) end_label }));
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) false_label }));
            code_generator.push_instruction(Instruction(InstructionType::PUSH, Word { .as_int = 0 }));
            code_generator.push_instruction(Instruction(InstructionType::LABEL, Word { .as_int = (int64_t) end_label }));
        } else {
            this->operand->emit(code_generator); 
            switch (this->operator_token.get_type()) {
                case TokenType::TILDE:
                    code_generator.push_instruction(Instruction(InstructionType::IBNEG));
                    break;
                case TokenType::PLUS:
                    break;
                case TokenType::MINUS:
                    if (this->operand->get_type()->fits(Type::FLOAT)) {
                        code_generator.push_instruction(Instruction(InstructionType::FNEG));
                    } else if (this->operand->get_type()->fits(Type::INT)) {
                        code_generator.push_instruction(Instruction(InstructionType::INEG));
                    } else {
                        assert(false && "unreachable");
                    }
                    break;
                //case TokenType::BANG:
                //    code_generator.push_instruction(Instruction(InstructionType::LNEG));
                //    break;
                default:
                    assert(false && "unreachable");
            }
        }
    }
    
    virtual void emit_condition(CodeGenerator& code_generator, size_t jump_if_false, size_t jump_if_true) const {
        assert(this->get_type()->fits(Type::BOOL));
        if (this->operator_token.get_type() == TokenType::BANG) {
            this->operand->emit_condition(code_generator, jump_if_true, jump_if_false);
        } else {
            assert(false && "unreachable");
        }
    }
    
    virtual bool is_lvalue() const override {
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        if (this->element_initializers.size() == 0) {
            this->set_type(std::make_shared<ListType>(Type::GENERIC));
        } else {
            for (auto& element : this->element_initializers) {
                element->type_check(type_checker);
            }

            auto element_type = this->element_initializers[0]->get_type();

            if (element_type->fits(Type::VOID)) {
                std::cerr << this->get_location() << ": TYPE_ERROR: List cannot have content type void." << std::endl;
                std::exit(1);
            }

            for (size_t i = 1; i < this->element_initializers.size(); i++) {
                auto& element = this->element_initializers[i];
                if (!element->get_type()->fits(element_type)) {
                    std::cerr
                        << this->get_location()
                        << ": TYPE_ERROR: Inconsistend type inside of list literal: The first element has type <"
                        << element_type->to_string()
                        << "> while the element at index "
                        << i
                        << " has type <"
                        << element->get_type()->to_string()
                        << ">." << std::endl;
                    std::exit(1);
                }
            }

            this->set_type(std::make_shared<ListType>(element_type));
        }
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
    
    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }
    
    virtual bool is_lvalue() const override {
        return false;
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
    
    virtual void type_check(TypeChecker& type_checker) override {
        this->operand->type_check(type_checker);

        // TODO: Maybe create global constant for generic lists
        auto operand_type = this->operand->get_type();
        auto inner_type = Type::NO;
        if (operand_type->fits(std::make_shared<ListType>(Type::GENERIC))) {
            auto operand_type_as_list_type = dynamic_cast<ListType*>(operand_type.get());
            inner_type = operand_type_as_list_type->get_inner_type();
        } else if (operand_type->fits(Type::STRING)) {
            inner_type = Type::CHAR;
        } else {
            std::cerr << this->get_location() << ": TYPE_ERROR: Type <" << operand_type->to_string() << "> is not indexable." << std::endl;
            std::exit(1);
        }

        this->index->type_check(type_checker);
        if (!this->index->get_type()->fits(Type::INT)) {
            std::cerr << this->get_location() << ": TYPE_ERROR: Index must be an integer." << std::endl;
            std::exit(1);
        }

        this->set_type(inner_type);
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
    
    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }

    // TODO: Make this more general (strings are immutable; this should probably be handled like fields)
    virtual bool is_lvalue() const override {
        return this->operand->is_lvalue() && !this->operand->get_type()->fits(Type::STRING);
    }

    ~IndexingExpression() {}
};

class CastExpression : public Expression {
private:
    std::unique_ptr<TypeAnnotation> type_annotation;
    std::unique_ptr<Expression> casted;
public:
    CastExpression(const Location& start_location, std::unique_ptr<TypeAnnotation> type_annotation, std::unique_ptr<Expression> casted)
        : Expression(start_location), type_annotation(std::move(type_annotation)), casted(std::move(casted))
    {}

    virtual void append_to_output_stream(std::ostream& output_stream, size_t layer = 0) const override {
        indent_layer(output_stream, layer);
        output_stream << "CastExpression(" << this->type_annotation->to_string() << ")" << std::endl;
        this->casted->append_to_output_stream(output_stream, layer + 1);
    }

    virtual void type_check(TypeChecker& type_checker) {
        auto destination_type = this->type_annotation->to_type();
        this->casted->type_check(type_checker);
        auto source_type = this->casted->get_type();

        if (source_type->fits(destination_type)) {
            if (source_type->is_generic()) {
                this->casted->set_type(destination_type);
            }
        } else {
            bool found = false;
            
            for (size_t i = 0; i < ALLOWED_TYPE_CAST_COUNT; i++) {
                const auto& type_pair = TypeChecker::ALLOWED_TYPE_CASTS[i];
                if (type_pair.first->fits(source_type) && type_pair.second->fits(destination_type)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr <<
                    this->get_location() <<
                    ": TYPE_ERROR: Cannot cast from <"
                    << source_type->to_string() <<
                    "> to <"
                    << destination_type->to_string() <<
                    ">." << std::endl;
                std::exit(1);
            }
        }
        
        this->set_type(destination_type);
    }
    
    virtual void emit(CodeGenerator&) const override {
        assert(false && "TODO");
    }
    
    virtual void emit_condition(CodeGenerator&, size_t, size_t) const {
        assert(false && "TODO");
    }
    
    virtual bool is_lvalue() const override {
        return false;
    }

    ~CastExpression() {}
};
