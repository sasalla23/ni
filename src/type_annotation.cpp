class TypeAnnotation {
private:
    Location location;
public:
    TypeAnnotation(const Location& location) : location(location)
    {}

    virtual std::string to_string() const = 0;
    virtual std::shared_ptr<Type> to_type() const = 0;

    const Location& get_location() const {
        return this->location;
    }

    virtual ~TypeAnnotation() {};
};

class PrimitiveTypeAnnotation : public TypeAnnotation {
private:
    Token name_token;
public:
    PrimitiveTypeAnnotation(const Token& name_token)
        : TypeAnnotation(name_token.get_location()), name_token(name_token)
    {}

    virtual std::string to_string() const override {
        return this->name_token.get_text();
    }

    virtual std::shared_ptr<Type> to_type() const override {
        switch (this->name_token.get_type()) {
            case TokenType::INT_KEYWORD: return Type::INT;
            case TokenType::VOID_KEYWORD: return Type::VOID;
            case TokenType::FLOAT_KEYWORD: return Type::FLOAT;
            case TokenType::STRING_KEYWORD: return Type::STRING;
            case TokenType::CHAR_KEYWORD: return Type::CHAR;
            case TokenType::BOOL_KEYWORD: return Type::BOOL;
            default: assert(false && "unreachable"); 
        }
    }

    ~PrimitiveTypeAnnotation() {}
};

class ListTypeAnnotation : public TypeAnnotation {
private:
    std::unique_ptr<TypeAnnotation> inner_type;
public:
    ListTypeAnnotation(const Location& start_location, std::unique_ptr<TypeAnnotation> inner_type)
        : TypeAnnotation(start_location), inner_type(std::move(inner_type))
    {}

    virtual std::string to_string() const override {
        return "[" + this->inner_type->to_string() + "]";
    }

    virtual std::shared_ptr<Type> to_type() const override {
        return std::make_shared<ListType>(this->inner_type->to_type());
    }

    ~ListTypeAnnotation() {}
};
