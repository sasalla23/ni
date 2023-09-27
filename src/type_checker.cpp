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
    ARITHMETIC_OPERATORS(Types::INT),
    BIN_OPERATORS(Types::INT),
    ORDERING_OPERATORS(Types::INT),

    BinaryOperator(TokenType::PERCENT, Type::INT, Type::INT, Type::INT),

    ARITHMETIC_OPERATORS(Types::FLOAT),
    ORDERING_OPERATORS(Types::FLOAT),

    BinaryOperator(TokenType::EQUAL_EQUAL, Type::GENERIC, Type::GENERIC, Type::BOOL),
    BinaryOperator(TokenType::BANG_EQUAL, Type::GENERIC, Type::GENERIC, Type::BOOL),

    LOGICAL_OPERATORS(Type::BOOL),
};

constexpr size_t BINARY_OPERATOR_COUNT (sizeof(OPERATORS) / sizeof(BinaryOperator))

class TypeChecker {

};
