
#define PRIMITIVE_LIST \
   PRIMITIVE_ENTRY(INT) \
    PRIMITIVE_ENTRY(CHAR) \
    PRIMITIVE_ENTRY(VOID) \
    PRIMITIVE_ENTRY(STRING) \
    PRIMITIVE_ENTRY(FLOAT) \
    PRIMITIVE_ENTRY(BOOL) \

class Type {
    friend class PrimitiveType;
    friend class ListType;
protected:
    enum class TypeType {
        LIST,
        PRIMITIVE,
        GENERIC,
        NO_TYPE
    };

    TypeType type_type;
    std::unordered_map<std::string, std::shared_ptr<Type>> fields;

public:
    Type(TypeType type_type) : type_type(type_type) {}

#define PRIMITIVE_ENTRY(x) static std::shared_ptr<Type> x;
    PRIMITIVE_LIST
#undef PRIMITIVE_ENTRY    

    static std::shared_ptr<Type> NO;
    static std::shared_ptr<Type> GENERIC;

    virtual std::string to_string() const = 0;
    virtual bool fits(std::shared_ptr<Type> other) const = 0;
    virtual bool is_generic() const = 0;

    bool has_field(const std::string& field_name) const {
        return this->fields.contains(field_name);
    }

    std::shared_ptr<Type> get_field_type(const std::string& field_name) const {
        return this->fields.at(field_name);
    }

    virtual ~Type() {}
};

class NoType : public Type {
public:
    NoType() : Type(Type::TypeType::NO_TYPE) {}
    
    virtual std::string to_string() const override {
        assert(false && "unreachable");
    }

    virtual bool fits(std::shared_ptr<Type>) const override {
        return false;
    }

    virtual bool is_generic() const override {
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
    {
        this->fields["length"] = Type::INT;
    }

    virtual std::string to_string() const override {
        return "[" + inner_type->to_string() + "]";
    }

    std::shared_ptr<Type> get_inner_type() const {
        return this->inner_type;
    }
    
    virtual bool fits(std::shared_ptr<Type> other) const override {
        if (other->type_type == Type::TypeType::GENERIC) return true;
        if (other->type_type == Type::TypeType::LIST) {
            return this->inner_type->fits(dynamic_cast<ListType*>(other.get())->inner_type);
        } else {
            return false;
        }
    }

    virtual bool is_generic() const override {
        return this->inner_type->is_generic();
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
    {
        if (primitive_type == Primitive::STRING) {
            this->fields["length"] = Type::INT;
        }
    }

    virtual std::string to_string() const override {
        std::stringstream output;
        output << this->primitive_type;
        return output.str();
    }
    
    virtual bool fits(std::shared_ptr<Type> other) const override {
        if (other->type_type == Type::TypeType::GENERIC) return true;
        if (other->type_type == Type::TypeType::PRIMITIVE) {
            return this->primitive_type == dynamic_cast<PrimitiveType*>(other.get())->primitive_type;
        } else {
            return false;
        }
    }

    virtual bool is_generic() const override {
        return false;
    }
    
    ~PrimitiveType() {}
};

class GenericType : public Type {
public:
    GenericType() : Type(Type::TypeType::GENERIC) {}

    virtual std::string to_string() const override {
        return "GENERIC";
    }

    virtual bool fits(std::shared_ptr<Type>) const override {
        return true;
    }

    virtual bool is_generic() const override {
        return true;
    }

    ~GenericType() {}
};

#define PRIMITIVE_ENTRY(x) std::shared_ptr<Type> Type:: x = std::make_shared<PrimitiveType>(Primitive:: x);
PRIMITIVE_LIST
#undef PRIMITIVE_ENTRY
std::shared_ptr<Type> Type::NO = std::make_shared<NoType>();
std::shared_ptr<Type> Type::GENERIC = std::make_shared<GenericType>();
