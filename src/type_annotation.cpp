class TypeAnnotation {
private:
    Location location;
public:
    TypeAnnotation(const Location& location) : location(location)
    {}

    virtual std::string to_string() const = 0;

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

    ~ListTypeAnnotation() {}
};
