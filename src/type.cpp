
#define PRIMITIVE_LIST \
    PRIMITIVE_ENTRY(INT) \
    PRIMITIVE_ENTRY(CHAR) \
    PRIMITIVE_ENTRY(VOID) \
    PRIMITIVE_ENTRY(STRING) \
    PRIMITIVE_ENTRY(FLOAT) \
    PRIMITIVE_ENTRY(GENERIC) \

class Type {
protected:
    enum class TypeType {
        LIST,
        PRIMITIVE,
        NO_TYPE
    };

    TypeType type_type;
public:
    Type(TypeType type_type) : type_type(type_type) {}

#define PRIMITIVE_ENTRY(x) static std::shared_ptr<Type> x;
    PRIMITIVE_LIST
#undef PRIMITIVE_ENTRY    
    static std::shared_ptr<Type> NO;

    virtual std::string to_string() const = 0;

    virtual bool fits(std::shared_ptr<Type> other) const = 0;

    virtual ~Type() {}
};

class NoType : public Type {
public:
    NoType() : Type(Type::TypeType::NO_TYPE) {}
    
    virtual std::string to_string() const override {
        assert(false && "unreachable");
    }

    virtual bool fits(std::shared_ptr<Type> other) const {
        return false;
    }

    virtual ~NoType() {}
};

class ListType : public Type {
private:
    std::shared_ptr<Type> inner_type;
public:
    ListType(std::shared_ptr<Type> inner_type)
        : Type(Type::TypeType::LIST), inner_type(inner_type)
    {}

    virtual std::string to_string() const override {
        return "[" + inner_type->to_string() + "]";
    }
    
    virtual bool fits(std::shared_ptr<Type> other) const {
        if (other->type_type == Type::TypeType::LIST) {
            return this->inner_type->fits(dynamic_cast<std::shared_ptr<ListType>>(other)->inner_type);
        } else {
            return false;
        }
    }

    ~ListType() {}
};


#define PRIMITIVE_ENTRY(x) x,

enum class Primitive {
    PRIMITIVE_LIST
};

#undef PRIMITIVE_ENTRY

#define PRIMITIVE_ENTRY(x) case Primitive:: x : return output_stream << #x;

std::ostream& operator<<(std::ostream& output_stream, const Primitive& primitive) {
    switch (primitive) {
        PRIMITIVE_LIST
        default: assert(false && "unreachable");
    }
}

#undef PRIMITIVE_ENTRY

class PrimitiveType : public Type {
private:
    Primitive primitive_type;
public:
    PrimitiveType(Primitive primitive_type)
        : Type(Type::TypeType::PRIMITIVE), primitive_type(primitive_type)
    {}

    virtual std::string to_string() const override {
        std::stringstream output;
        output << this->primitive_type;
        return output.str();
    }
    
    ~PrimitiveType() {}
};

#define PRIMITIVE_ENTRY(x) std::shared_ptr<Type> Type:: x = std::make_shared<PrimitiveType>(Primitive:: x);
PRIMITIVE_LIST
#undef PRIMITIVE_ENTRY
std::shared_ptr<Type> Type::NO = std::make_shared<NoType>();
