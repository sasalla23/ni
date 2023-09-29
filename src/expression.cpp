
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
            std::cerr << this->get_location() << ": TYPE_ERROR: Type <" << accessed_type->to_string() << "> does not have a field '" << field_name << "'." << std::endl;
            std::exit(1);
        }

        this->set_type(accessed_type->get_field_type(field_name));
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
            for (auto& element : this->element_initializers) {
                if (!element->get_type()->is_generic()) {
                    element_type = element->get_type();
                    break;
                }
            }

            for (size_t i = 0; i < this->element_initializers.size(); i++) {
                auto& element = this->element_initializers[i];
                element->type_check(type_checker);
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
                // TODO: Recursive type inference
                if (element->get_type()->is_generic()) {
                    element->set_type(element_type);
                }
            }
            this->set_type(std::make_shared<ListType>(element_type));
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

    ~IndexingExpression() {}
};

