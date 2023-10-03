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
//    IPUSH,
//    FPUSH,
//    //PPUSH,
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
//    HWRITE,
//    HALLOC,
//    HREALLOC,
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

    void print() {
        std::cout << "Operand Stack:" << std::endl;
        for (const auto& stack_element : this->operand_stack) {
            std::cout << stack_element.get_content().as_int << std::endl;
        }
    }
};

class ObjectLayout {
private:
    size_t size;
    std::vector<size_t> object_offsets;
public:
    ObjectLayout(size_t size, std::vector<size_t> object_offsets)
        : size(size), object_offsets(std::move(object_offsets))
    {}
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
};

class VirtualMachine {
private:
    std::vector<AllocatedObject> allocated_objects;
    std::vector<Frame> call_stack;
    std::vector<Instruction> program;
    size_t instruction_pointer;
public:
    VirtualMachine(std::vector<Instruction> program)
        : allocated_objects(), call_stack(), program(std::move(program)), instruction_pointer(0)
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
    }

    void push_on_stack(StackElement value) {
        assert(this->call_stack.size() > 0);
        this->call_stack[this->call_stack.size() - 1].push_operand(value);
    }

    void print_current_frame() {
        assert(this->call_stack.size() > 0);
        this->call_stack[this->call_stack.size() - 1].print();
    }

    void execute_instruction() {
        const Instruction& current_instruction = this->get_current_instruction();
        switch (current_instruction.get_type()) {
            case InstructionType::PUSH:
                push_on_stack(StackElement(StackElementType::PRIMITIVE, current_instruction.get_operand()));
                this->instruction_pointer += 1;
                break;
            default:
                break;
        }
    }
};
