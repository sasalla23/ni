union Word {
    int64_t as_int;
    double as_float;
    void *as_pointer;
};

//
//    // ==== Operators
//
//    // Integer arithmetic
//    IADD,
//    IMUL,
//    ISUB,
//    IDIV,
//
//    // Bitwise instructions
//    ISHL,
//    ISHR,
//    IAND,
//    IXOR,
//    IOR,
//    
//    // Modulo
//    IMOD,
//
//    // Binary / numerical negation
//    IBNEG, // ~
//    INEG,
//
//    // Float arithmetics
//    FADD,
//    FMUL,
//    FSUB,
//    FDIV,
//
//    // float negation
//    FNEG,
//
//    // Logical negation
//    NOT,
//
//    // ==== Jump instructions
//    
//    JUMP,
//
//    // Compare ints
//    JILE,
//    JILT,
//    JIGE,
//    JIGT,
//
//    // Compare floats
//    JFLE,
//    JFLT,
//    JFGE,
//    JFGT,
//
//    // Compare words
//    JEQ,
//    JNEQ,
//    JEQZ, // Jump if equal to zero
//
//    // ==== Stack manipulation
//    PUSH [value],
//    POP,
//    DUB, // dublicate item on top of the stack
//    NOP, // No instruction to see here
//
//    // Local var read/write
//    VREAD,
//    VWRITE,
//    SETVAR, // Set local var count
//
//    // Heap read/write
//    HREAD,
//    WRITEW, stack: ... [pointer] [value]
//    HALLOC [object layout], stack: ... [count]
//    HREALLOC,
//
//    // static memory
//    SPTR [offset], // puts absolute pointer to static memory location on the stack (offset is relative to the beginning of the static memory)
//    
//    // Pointer arithmetic
//    PADD,
//
//    // Call function
//    CALL,
//    RET, 
//
//    // Type Conversions
//    I2C,
//    I2S,
//    I2F,
//    C2I,
//    F2S,
//    F2I,
//
//    // Call garbage collector
//    CLEAN,
//
//    // Halt
//    HALT,

#define INSTRUCTION_TYPE_LIST \
    INSTRUCTION_ENTRY(HALT) \
    INSTRUCTION_ENTRY(PUSH) \
    INSTRUCTION_ENTRY(HALLOC) \
    INSTRUCTION_ENTRY(DUP) \
    INSTRUCTION_ENTRY(WRITEW) \
    INSTRUCTION_ENTRY(PADD) \
    INSTRUCTION_ENTRY(SPTR) \
    INSTRUCTION_ENTRY(PRINT)

#define INSTRUCTION_ENTRY(x) x,
enum class InstructionType {
    INSTRUCTION_TYPE_LIST
};
#undef INSTRUCTION_ENTRY

#define INSTRUCTION_ENTRY(x) case InstructionType:: x: return output_stream << #x;

std::ostream& operator<<(std::ostream& output_stream, const InstructionType& instruction_type) {
    switch (instruction_type) {
        INSTRUCTION_TYPE_LIST
        default: assert(false && "unreachable");
    }
}

#undef INSTRUCTION_ENTRY

class Instruction {
private:
    InstructionType type;
    Word operand;
public:
    Instruction(InstructionType type)
        : type(type), operand(0)
    {}

    Instruction(InstructionType type, Word operand)
        : type(type), operand(operand)
    {}

    InstructionType get_type() const {
        return this->type;
    }

    Word get_operand() const {
        return this->operand;
    }
};
    
enum class StackElementType {
    PRIMITIVE,
    OBJECT,
} type;

class StackElement {
private:
    StackElementType type;
    Word content;
public:
    StackElement(StackElementType type, Word content)
        : type(type), content(content)
    {}

    StackElementType get_type() const { return this->type; }
    Word get_content() const { return this->content; }
};

class Frame {
private:
    std::vector<StackElement> operand_stack;
    std::vector<StackElement> local_variables;
    size_t return_address;
public:
    Frame(size_t return_address)
        : operand_stack(), local_variables(), return_address(return_address)
    {}

    void push_operand(StackElement operand) {
        this->operand_stack.push_back(operand);
    }
    
    StackElement get_top_operand() {
        return operand_stack.back();
    }

    StackElement pop_operand() {
        StackElement top = get_top_operand();
        operand_stack.pop_back();
        return top;
    }


    void print() {
        std::cout << "Operand Stack:" << std::endl;
        for (const auto& stack_element : this->operand_stack) {
            std::cout << stack_element.get_content().as_int << std::endl;
        }
    }
};

enum PredefinedLayouts {
    CHAR_LAYOUT = 0,
    STRING_LAYOUT,
    PREDEFINED_LAYOUT_COUNT
};

class ObjectLayout {
private:
    size_t size;
    std::vector<size_t> object_offsets;
public:
    ObjectLayout(size_t size, std::vector<size_t> object_offsets)
        : size(size), object_offsets(std::move(object_offsets))
    {}

    static std::shared_ptr<ObjectLayout> predefined_layouts[PREDEFINED_LAYOUT_COUNT];

    size_t get_size() const { return this->size; }
    const std::vector<size_t>& get_object_offsets() const { return this->object_offsets; }
};

// Predefined object layouts
// -> P = Primitive
// -> O = Object

std::shared_ptr<ObjectLayout> ObjectLayout::predefined_layouts[] = {
    // Char
    // P # -- 1 byte char
    /*[CHAR_LAYOUT] = */ std::make_shared<ObjectLayout>(sizeof(char), std::vector<size_t> {}),

    // String
    // P ######## -- 8 byte size
    // O ######## -- 8 byte pointer to string data
    /*[STRING_LAYOUT] = */ std::make_shared<ObjectLayout>(sizeof(Word) * 2, std::vector<size_t> { 1 * sizeof(Word) }),
};

class AllocatedObject {
private:
    size_t count;
    void *data;
    std::shared_ptr<ObjectLayout> object_layout;
public:
    AllocatedObject(size_t count, void *data, std::shared_ptr<ObjectLayout> object_layout)
        : count(count), data(data), object_layout(object_layout)
    {}

    void *get_data() const {
        return this->data;
    }
};

class VirtualMachine {
private:
    std::vector<AllocatedObject> allocated_objects;
    std::vector<Frame> call_stack;
    std::vector<Instruction> program;
    std::vector<char> static_memory;
    size_t instruction_pointer;
public:
    VirtualMachine(std::vector<Instruction> program, std::vector<char> static_memory)
        : allocated_objects(), call_stack(), program(std::move(program)), static_memory(std::move(static_memory)), instruction_pointer(0)
    {
        this->call_stack.push_back(Frame(0));
    }

    Instruction get_current_instruction() {
        if (this->instruction_pointer < this->program.size()) {
            return this->program[instruction_pointer];
        } else {
            return Instruction(InstructionType::HALT);
        }
    }

    void execute() {
        while (this->get_current_instruction().get_type() != InstructionType::HALT) {
            execute_instruction();
        }

        for (auto& object : this->allocated_objects) {
            std::free(object.get_data());
        }
    }

    void push_on_stack(StackElement value) {
        assert(this->call_stack.size() > 0);
        this->call_stack.back().push_operand(value);
    }

    StackElement pop_from_stack() {
        assert(this->call_stack.size() > 0);
        return this->call_stack.back().pop_operand();
    }
    
    StackElement get_stack_top() {
        assert(this->call_stack.size() > 0);
        return this->call_stack.back().get_top_operand();
    }

    void print_current_frame() {
        assert(this->call_stack.size() > 0);
        this->call_stack.back().print();
    }

    void execute_instruction() {
        const Instruction& current_instruction = this->get_current_instruction();
        switch (current_instruction.get_type()) {
            case InstructionType::PUSH:
                push_on_stack(StackElement(StackElementType::PRIMITIVE, current_instruction.get_operand()));
                this->instruction_pointer += 1;
                break;

            case InstructionType::HALLOC:
                {
                    // TODO: Check for valid layout index
                    size_t layout_index = (size_t) current_instruction.get_operand().as_int;
                    auto object_layout = ObjectLayout::predefined_layouts[layout_index];
                    Word count = this->pop_from_stack().get_content();
                    void *data = std::malloc(count.as_int * object_layout->get_size());
                    this->allocated_objects.push_back(AllocatedObject((size_t)count.as_int, data, object_layout));
                    push_on_stack(StackElement(StackElementType::OBJECT, Word { .as_pointer = data }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::DUP:
                this->push_on_stack(this->get_stack_top());
                this->instruction_pointer += 1;
                break;
            case InstructionType::WRITEW:
                {
                    Word value = this->pop_from_stack().get_content();
                    void* address = this->pop_from_stack().get_content().as_pointer;
                    *(Word*)address = value;
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::PADD:
                {
                    size_t offset = (size_t)this->pop_from_stack().get_content().as_int;
                    void *address = this->pop_from_stack().get_content().as_pointer;
                    void *new_address = (char*)address + offset;
                    this->push_on_stack(StackElement(StackElementType::OBJECT, Word { .as_pointer = new_address }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::SPTR:
                {
                    size_t offset = (size_t)current_instruction.get_operand().as_int;
                    void *address = this->static_memory.data() + offset;
                    this->push_on_stack(StackElement(StackElementType::OBJECT, Word { .as_pointer = address }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::PRINT:
                {
                    void *string_object = this->pop_from_stack().get_content().as_pointer;
                    size_t size = (size_t)(*(Word*)string_object).as_int;
                    char *data = (char*)(*(Word*)((char*)string_object + sizeof(Word))).as_pointer;
                    std::string printed_string(data, size);
                    std::cout << printed_string;
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::HALT:
                break;
            default:
                std::cerr << "Not implemented: " << current_instruction.get_type() << std::endl;
                assert(false && "TODO");
                break;
        }
    }
};
