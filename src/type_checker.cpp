class BinaryOperator {
private:
    TokenType operator_token_type;
    std::shared_ptr<Type> left_type, right_type;
    std::shared_ptr<Type> return_type;
public:
    BinaryOperator(
            TokenType operator_token_type,
            std::shared_ptr<Type> left_type,
            std::shared_ptr<Type> right_type,
            std::shared_ptr<Type> return_type)
        :
            operator_token_type(operator_token_type),
            left_type(left_type),
            right_type(right_type),
            return_type(return_type)
    {}

    bool fits_criteria(TokenType operator_token_type, std::shared_ptr<Type> left_type, std::shared_ptr<Type> right_type) const {
        return this->operator_token_type == operator_token_type && left_type->fits(this->left_type) && right_type->fits(this->right_type);
    }

    std::shared_ptr<Type> get_return_type() const {
        return this->return_type;
    }

    static BinaryOperator OPERATORS[];
    ~BinaryOperator() {}
};

#define ARITHMETIC_OPERATORS(T) \
    BinaryOperator(TokenType::PLUS,  T, T, T), \
    BinaryOperator(TokenType::STAR,  T, T, T), \
    BinaryOperator(TokenType::MINUS, T, T, T), \
    BinaryOperator(TokenType::SLASH, T, T, T)

#define BIN_OPERATORS(T) \
    BinaryOperator(TokenType::LESS_LESS, T, T, T), \
    BinaryOperator(TokenType::GREATER_GREATER, T, T, T), \
    BinaryOperator(TokenType::AND, T, T, T), \
    BinaryOperator(TokenType::HAT, T, T, T), \
    BinaryOperator(TokenType::PIPE, T, T, T) 

#define ORDERING_OPERATORS(T) \
    BinaryOperator(TokenType::LESS, T, T, Type::BOOL), \
    BinaryOperator(TokenType::LESS_EQUAL, T, T, Type::BOOL), \
    BinaryOperator(TokenType::GREATER, T, T, Type::BOOL), \
    BinaryOperator(TokenType::GREATER_EQUAL, T, T, Type::BOOL)

#define LOGICAL_OPERATORS(T) \
    BinaryOperator(TokenType::AND_AND, T, T, Type::BOOL), \
    BinaryOperator(TokenType::PIPE_PIPE, T, T, Type::BOOL)

BinaryOperator BinaryOperator::OPERATORS[] = {
    ARITHMETIC_OPERATORS(Type::INT),
    BIN_OPERATORS(Type::INT),
    ORDERING_OPERATORS(Type::INT),

    BinaryOperator(TokenType::PERCENT, Type::INT, Type::INT, Type::INT),

    ARITHMETIC_OPERATORS(Type::FLOAT),
    ORDERING_OPERATORS(Type::FLOAT),

    BinaryOperator(TokenType::EQUAL_EQUAL, Type::GENERIC, Type::GENERIC, Type::BOOL),
    BinaryOperator(TokenType::BANG_EQUAL, Type::GENERIC, Type::GENERIC, Type::BOOL),

    LOGICAL_OPERATORS(Type::BOOL),
};

constexpr size_t BINARY_OPERATOR_COUNT = (sizeof(BinaryOperator::OPERATORS) / sizeof(BinaryOperator));

class UnaryOperator {
private:
    TokenType operator_token_type;
    std::shared_ptr<Type> operand_type;
    std::shared_ptr<Type> return_type;
public:
    UnaryOperator(TokenType operator_token_type, std::shared_ptr<Type> operand_type, std::shared_ptr<Type> return_type)
        : operator_token_type(operator_token_type), operand_type(operand_type), return_type(return_type)
    {}

    bool fits_criteria(TokenType operator_token_type, std::shared_ptr<Type> operand_type) const {
        return operator_token_type == this->operator_token_type && operand_type->fits(this->operand_type);
    }

    std::shared_ptr<Type> get_return_type() const {
        return this->return_type;
    }

    static UnaryOperator OPERATORS[];
    ~UnaryOperator() {}
};

UnaryOperator UnaryOperator::OPERATORS[] = {
    UnaryOperator(TokenType::TILDE, Type::INT, Type::INT),
    UnaryOperator(TokenType::PLUS, Type::INT, Type::INT),
    UnaryOperator(TokenType::MINUS, Type::INT, Type::INT),
    
    UnaryOperator(TokenType::BANG, Type::BOOL, Type::BOOL),
};

constexpr size_t UNARY_OPERATOR_COUNT = (sizeof(UnaryOperator::OPERATORS) / sizeof(UnaryOperator));

enum class SymbolType {
    VARIABLE,
    FUNCTION
};

class Symbol {
private:
    size_t layer;
    SymbolType symbol_type;
public:
    Symbol(size_t layer, SymbolType symbol_type) : layer(layer), symbol_type(symbol_type) {}

    SymbolType get_symbol_type() const {
        return this->symbol_type;
    }

    size_t get_layer() const { return this->layer; }

    virtual ~Symbol() {}
};

class VariableSymbol : public Symbol {
private:
    std::shared_ptr<Type> type;
public:
    VariableSymbol(size_t layer, std::shared_ptr<Type> type) : Symbol(layer, SymbolType::VARIABLE), type(type) {}

    std::shared_ptr<Type> get_type() const {
        return this->type;
    }

    ~VariableSymbol() {}
};

// TODO: Maybe add overloads
class FunctionSymbol : public Symbol {
private:
    std::shared_ptr<Type> return_type;
    std::vector<std::shared_ptr<Type>> argument_types;
public:
    FunctionSymbol(size_t layer, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types)
        : Symbol(layer, SymbolType::FUNCTION), return_type(return_type), argument_types(std::move(argument_types))
    {}

    // TODO: Have seperate error messages
    bool do_args_fit(const std::vector<std::shared_ptr<Type>>& given_types) const {
        if (given_types.size() != this->argument_types.size()) {
            return false;
        }

        // TODO: Handle type inference
        for (size_t i = 0; i < given_types.size(); i++) {
            if (!given_types[i]->fits(this->argument_types[i])) {
                return false;
            }
        }

        return true;
    }

    std::shared_ptr<Type> get_return_type() const {
        return this->return_type;
    }

    ~FunctionSymbol() {}
};

class TypeChecker {
private:
    // TODO: Decide whether variable shadowing should be a thing
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbol_table;
    size_t current_layer;
    size_t while_statement_layer = 0;
    std::shared_ptr<Type> current_return_type;
public:
    static std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>> ALLOWED_TYPE_CASTS[];

    TypeChecker() : symbol_table(), current_layer(0), while_statement_layer(0), current_return_type(Type::NO) {
        this->symbol_table["print"] = std::make_unique<FunctionSymbol>(this->current_layer, Type::VOID, std::vector<std::shared_ptr<Type>> { Type::STRING });
        this->symbol_table["print_line"] = std::make_unique<FunctionSymbol>(this->current_layer, Type::VOID, std::vector<std::shared_ptr<Type>> { Type::STRING });
    }

    std::shared_ptr<Type> get_current_return_type() const {
        return this->current_return_type;
    }
    

    void set_current_return_type(std::shared_ptr<Type> return_type) {
        this->current_return_type = return_type;
    }

    bool symbol_exists(const std::string& name) {
        return this->symbol_table.contains(name);
    } 

    const std::unique_ptr<Symbol>& get_symbol(const std::string& name) {
        assert(this->symbol_exists(name) && "Symbol must exist to call this function");
        return this->symbol_table[name];
    }

    void add_variable_symbol(const std::string& name, std::shared_ptr<Type> variable_type) {
        this->symbol_table[name] = std::make_unique<VariableSymbol>(this->current_layer, variable_type);
    }
    
    void add_function_symbol(const std::string& name, std::shared_ptr<Type> return_type, std::vector<std::shared_ptr<Type>> argument_types) {
        this->symbol_table[name] = std::make_unique<FunctionSymbol>(this->current_layer, return_type, std::move(argument_types));
    }

    void push_while_statement() {
        this->while_statement_layer += 1;
    }

    bool is_in_while_statement() {
        return this->while_statement_layer > 0;
    }
    
    void pop_while_statement() {
        assert(this->while_statement_layer > 0);
        this->while_statement_layer -= 1;
    }

    void push_scope() {
        this->current_layer += 1;
    }

    void pop_scope() {
        assert(this->current_layer > 0);
        std::vector<std::string> to_remove;
        for (const auto& symbol_entry : this->symbol_table) {
            if (symbol_entry.second->get_layer() == this->current_layer) {
                to_remove.push_back(symbol_entry.first);
            }
        }
        
        for (const auto& popped_symbols : to_remove) {
            this->symbol_table.erase(popped_symbols);
        }

        this->current_layer -= 1;

    }

    ~TypeChecker() {}
};


// SOURCE, DEST
std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>> TypeChecker::ALLOWED_TYPE_CASTS[] = {
    { Type::INT, Type::CHAR },
    { Type::INT, Type::STRING },
    { Type::INT, Type::FLOAT },
    { Type::CHAR, Type::INT },
    { Type::CHAR, Type::STRING },
    { Type::STRING, std::make_shared<ListType>(Type::CHAR) },
    { std::make_shared<ListType>(Type::CHAR), Type::STRING },
    { Type::FLOAT, Type::INT },
    { Type::FLOAT, Type::STRING },
    { Type::BOOL, Type::STRING },
    { Type::BOOL, Type::INT },
};

constexpr size_t ALLOWED_TYPE_CAST_COUNT = (sizeof(TypeChecker::ALLOWED_TYPE_CASTS) / sizeof(TypeChecker::ALLOWED_TYPE_CASTS[0]));
