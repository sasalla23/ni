
#define TYPE_ERROR(message) \
    do { \
        std::cerr << this->get_location() << ": TYPE_ERROR: " << message << std::endl; \
        std::exit(1); \
    } while(0)

class Type;

enum class FieldAccess {
    INTERNAL,
    READ,
    READ_WRITE,
};

class Field {
private:
    FieldAccess access;
    std::shared_ptr<Type> type;
    size_t alignment; // alignment in bytes
public:
    Field(FieldAccess access, std::shared_ptr<Type> type, size_t alignment)
        : access(access), type(type), alignment(alignment)
    {}

    FieldAccess get_access() const { return this->access; }
    std::shared_ptr<Type> get_type() const { return this->type; }
    size_t get_alignment() const { return this->alignment; }
};

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
    friend class GenericType;
protected:
    enum class TypeType {
        LIST,
        PRIMITIVE,
        GENERIC,
        INTERNAL,
        NO_TYPE
    };

    TypeType type_type;
    std::unordered_map<std::string, std::unique_ptr<Field>> fields;

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
    virtual bool is_object() const = 0;
    virtual size_t get_size() const = 0;
    virtual size_t get_layout_index() const = 0;

    bool has_field(const std::string& field_name) const {
        return this->fields.contains(field_name);
    }

    const std::unique_ptr<Field>& get_field(const std::string& field_name) const {
        return this->fields.at(field_name);
    }

    void add_field(const std::string& field_name, FieldAccess access, std::shared_ptr<Type> type, size_t current_alignment) {
        this->fields[field_name] = std::make_unique<Field>(access, type, current_alignment);
        //if (type->is_object()) {
        //    current_alignment += sizeof(Word);
        //} else {
        //    current_alignment += type->get_size();
        //}
    }

    void add_index_field(const std::string& index_field_name, FieldAccess access, std::shared_ptr<Type> type) {
        this->fields["@index"] = std::make_unique<Field>(access, type, this->fields[index_field_name]->get_alignment());
    }

    //std::shared_ptr<ObjectLayout> get_layout() const {
    //    std::vector<size_t> object_offsets;

    //    for (const auto& field : this->fields) {
    //        if (field.first[0] != '@' && field.second->get_type()->is_object()) {
    //            object_offsets.push_back(field.second->get_alignment());
    //        }
    //    }

    //    return std::make_shared<ObjectLayout>(this->get_size(), std::move(object_offsets));
    //}

    virtual ~Type() {}
};


// Predefined object layouts
// -> P = Primitive
// -> O = Object


class NoType : public Type {
public:
    NoType() : Type(Type::TypeType::NO_TYPE) {}
    
    virtual std::string to_string() const override {
        assert(false && "unreachable");
    }

    virtual bool fits(std::shared_ptr<Type>) const override {
        assert(false && "unreachable");
    }

    virtual bool is_generic() const override {
        assert(false && "unreachable");
    }

    virtual bool is_object() const override {
        assert(false && "unreachable");
    }

    virtual size_t get_size() const override {
        assert(false && "unreachable");
    }
    
    virtual size_t get_layout_index() const override {
        assert(false && "unreachable");
    }

    virtual ~NoType() {}
};

class InternalType : public Type {
private:
    size_t size;
    bool is_object_flag;
public:
    InternalType(size_t size, bool is_object_flag)
        : Type(Type::TypeType::INTERNAL), size(size), is_object_flag(is_object_flag)
    {}

    virtual std::string to_string() const override {
        return "INTERNAL";
    }

    virtual bool fits(std::shared_ptr<Type>) const override {
        assert(false && "unreachable");
    }

    virtual bool is_generic() const override {
        assert(false && "unreachable");
    }

    virtual bool is_object() const override {
        return this->is_object_flag;
    }

    virtual size_t get_size() const override {
        return this->size;
    }
    
    virtual size_t get_layout_index() const override {
        assert(false && "unreachable");
    }
};

class ListType : public Type {
private:
    std::shared_ptr<Type> inner_type;
    static std::shared_ptr<Type> internal_array_type;
public:
    ListType(std::shared_ptr<Type> inner_type)
        : Type(Type::TypeType::LIST), inner_type(inner_type)
    {
        // ######## -- length: int
        // ######## -- capacity: int
        // ######## -- data: pointer(array)
        this->add_field("length", FieldAccess::READ, Type::INT, LIST_LENGTH_OFFSET);
        this->add_field("capacity", FieldAccess::INTERNAL, Type::INT, LIST_CAPACITY_OFFSET);
        this->add_field("data", FieldAccess::INTERNAL, internal_array_type, LIST_DATA_OFFSET);
        this->add_index_field("data", FieldAccess::READ_WRITE, inner_type);
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

    virtual bool is_object() const override {
        return true;
    }

    virtual size_t get_size() const override {
        return LIST_SIZE;
    }
    
    virtual size_t get_layout_index() const override {
        return LIST_LAYOUT;
    }

    ~ListType() {}
};

std::shared_ptr<Type> ListType::internal_array_type = std::make_shared<InternalType>(0, true);


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
friend class GenericType;
private:
    Primitive primitive_type;
    static std::shared_ptr<Type> internal_array_type;
public:
    PrimitiveType(Primitive primitive_type)
        : Type(Type::TypeType::PRIMITIVE), primitive_type(primitive_type)
    {
        if (primitive_type == Primitive::STRING) {
            size_t alignment = 0;
            this->add_field("length", FieldAccess::READ, Type::INT, STRING_LENGTH_OFFSET);
            this->add_field("data", FieldAccess::INTERNAL, internal_array_type, STRING_DATA_OFFSET);
            this->add_index_field("data", FieldAccess::READ, Type::CHAR);
        }
    }

    virtual std::string to_string() const override {
        std::stringstream output;
        output << this->primitive_type;
        return output.str();
    }
    
    virtual bool fits(std::shared_ptr<Type> other) const override {
        if (other->type_type == Type::TypeType::GENERIC && this->primitive_type != Primitive::VOID) return true;
        if (other->type_type == Type::TypeType::PRIMITIVE) {
            return this->primitive_type == dynamic_cast<PrimitiveType*>(other.get())->primitive_type;
        } else {
            return false;
        }
    }

    virtual bool is_generic() const override {
        return false;
    }
    
    virtual bool is_object() const override {
        if (this->primitive_type == Primitive::STRING) {
            return true;
        } else {
            return false;
        }
    }

    virtual size_t get_size() const override {
        switch(this->primitive_type) {
            case Primitive::INT:
                return sizeof(int64_t);
            case Primitive::CHAR:
                return sizeof(char);
            case Primitive::STRING:
                return STRING_SIZE; 
            case Primitive::FLOAT:
                return sizeof(double);
            case Primitive::BOOL:
                return sizeof(bool);
            case Primitive::VOID:
                return 0;
            default:
                assert(false && "unreachable");
        }
    }


    virtual size_t get_layout_index() const override {
        if (this->primitive_type == Primitive::STRING) {
            return STRING_LAYOUT;
        } else {
            switch (this->get_size()) {
                case sizeof(Word):
                    return WORD_LAYOUT;
                case sizeof(char):
                    return BYTE_LAYOUT;
                default:
                    assert(false && "unreachable");
            }
        }
    }
    
    ~PrimitiveType() {}
};

std::shared_ptr<Type> PrimitiveType::internal_array_type = std::make_shared<InternalType>(0, true);

class GenericType : public Type {
public:
    GenericType() : Type(Type::TypeType::GENERIC) {}

    virtual std::string to_string() const override {
        return "GENERIC";
    }

    virtual bool fits(std::shared_ptr<Type> other_type) const override {
        if (other_type->type_type == Type::TypeType::PRIMITIVE) {
            auto other_primitive_type = dynamic_cast<PrimitiveType*>(other_type.get())->primitive_type;
            if (other_primitive_type == Primitive::VOID) {
                return false;
            }
        }
        return true;
    }

    virtual bool is_generic() const override {
        return true;
    }

    virtual bool is_object() const override {
        assert(false && "unreachable");
    }

    virtual size_t get_size() const override {
        assert(false && "unreachable");
    }
    
    virtual size_t get_layout_index() const override {
        assert(false && "unreachable");
    }

    ~GenericType() {}
};

#define PRIMITIVE_ENTRY(x) std::shared_ptr<Type> Type:: x = std::make_shared<PrimitiveType>(Primitive:: x);
PRIMITIVE_LIST
#undef PRIMITIVE_ENTRY
std::shared_ptr<Type> Type::NO = std::make_shared<NoType>();
std::shared_ptr<Type> Type::GENERIC = std::make_shared<GenericType>();

//#define PRIMITIVE_ENTRY(x) Type:: x ->get_layout(),
//std::shared_ptr<ObjectLayout> ObjectLayout::predefined_layouts[] = {
//    ListType(Type::GENERIC).get_layout(), // LIST_LAYOUT
//    std::make_shared<ObjectLayout>(sizeof(Word), std::vector<size_t> { 0 }),
//    PRIMITIVE_LIST
//};
//#undef PRIMITIVE_ENTRY
