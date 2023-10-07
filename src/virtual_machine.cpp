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
//    READW [is object]
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
    \
    INSTRUCTION_ENTRY(PUSH) \
    INSTRUCTION_ENTRY(DUP) \
    \
    INSTRUCTION_ENTRY(HALLOC) \
    INSTRUCTION_ENTRY(WRITEW) \
    INSTRUCTION_ENTRY(READW) \
    INSTRUCTION_ENTRY(WRITEB) \
    INSTRUCTION_ENTRY(READB) \
    INSTRUCTION_ENTRY(PADD) \
    INSTRUCTION_ENTRY(SPTR) \
    INSTRUCTION_ENTRY(PRINT) \
    INSTRUCTION_ENTRY(VLOAD) \
    INSTRUCTION_ENTRY(VWRITE) \
    \
    INSTRUCTION_ENTRY(IBNEG)  \
    INSTRUCTION_ENTRY(FNEG) \
    INSTRUCTION_ENTRY(INEG)  \
    INSTRUCTION_ENTRY(LNEG) \
    \
    INSTRUCTION_ENTRY(IADD) \
    INSTRUCTION_ENTRY(ISUB) \
    INSTRUCTION_ENTRY(IMUL) \
    INSTRUCTION_ENTRY(IDIV) \
    INSTRUCTION_ENTRY(IMOD) \
    \
    INSTRUCTION_ENTRY(ISHL) \
    INSTRUCTION_ENTRY(ISHR) \
    INSTRUCTION_ENTRY(IAND) \
    INSTRUCTION_ENTRY(IOR) \
    INSTRUCTION_ENTRY(IXOR) \
    \
    INSTRUCTION_ENTRY(FADD) \
    INSTRUCTION_ENTRY(FSUB) \
    INSTRUCTION_ENTRY(FMUL) \
    INSTRUCTION_ENTRY(FDIV) \
    \
    INSTRUCTION_ENTRY(LABEL) \
    INSTRUCTION_ENTRY(JUMP) \
    INSTRUCTION_ENTRY(JNEQ) \
    INSTRUCTION_ENTRY(JEQ) \
    INSTRUCTION_ENTRY(JEQZ) \
    \
    INSTRUCTION_ENTRY(JILT) \
    INSTRUCTION_ENTRY(JILE) \
    INSTRUCTION_ENTRY(JIGT) \
    INSTRUCTION_ENTRY(JIGE) \
    \
    INSTRUCTION_ENTRY(JFLT) \
    INSTRUCTION_ENTRY(JFLE) \
    INSTRUCTION_ENTRY(JFGT) \
    INSTRUCTION_ENTRY(JFGE)


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

    void set_operand(Word operand) {
        this->operand = operand;
    }
};

std::ostream& operator<<(std::ostream& output_stream, const Instruction& instruction) {
    return output_stream << instruction.get_type() << " " << instruction.get_operand().as_int;
}
    
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



class ObjectLayout {
private:
    size_t size;
    std::vector<size_t> object_offsets;
public:
    ObjectLayout(size_t size, std::vector<size_t> object_offsets)
        : size(size), object_offsets(std::move(object_offsets))
    {}

    static std::shared_ptr<ObjectLayout> predefined_layouts[];

    size_t get_size() const { return this->size; }
    const std::vector<size_t>& get_object_offsets() const { return this->object_offsets; }
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

class CallInfo {
private:
    size_t return_address;
    size_t local_var_offset;
public:
    CallInfo(size_t return_address, size_t local_var_offset)
        : return_address(return_address), local_var_offset(local_var_offset)
    {}

    size_t get_return_address() const { return this->return_address; }
    size_t get_local_var_offset() const { return this->local_var_offset; }
};

class VirtualMachine {
private:
    std::vector<AllocatedObject> allocated_objects;
    std::vector<CallInfo> call_stack;
    
    std::vector<StackElement> operand_stack;
    std::vector<StackElement> local_vars;

    std::vector<Instruction> program;
    std::vector<char> static_memory;
    size_t instruction_pointer;
public:
    VirtualMachine(std::vector<Instruction> program, std::vector<char> static_memory)
        : allocated_objects(), call_stack(), operand_stack(), local_vars(), program(std::move(program)), static_memory(std::move(static_memory)), instruction_pointer(0)
    {
        //for (const auto& instruction : this->program) {
        //    std::cout << instruction << std::endl;
        //}
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
            //std::cout << "==================" << std::endl;
            //for (size_t i = 0; i < this->program.size(); i++) {
            //    std::cout << (i == this->instruction_pointer ? "> " : "  ") << i << ": " << this->program[i] << std::endl;
            //}
            //
            //std::cout << "==================" << std::endl;

            //this->print_current_frame();
            execute_instruction();
            //std::string s;
            //std::cin >> s;
        }

        for (auto& object : this->allocated_objects) {
            std::free(object.get_data());
        }
    }

    void push_on_stack(StackElement value) {
        this->operand_stack.push_back(value);
    }

    StackElement pop_from_stack() {
        StackElement top = this->get_stack_top();
        this->operand_stack.pop_back();
        return top;
    }
    
    StackElement get_stack_top() {
        return this->operand_stack.back();
    }

    void print_current_frame() {
        std::cout << "Operand Stack: " << std::endl;
        for (const auto& element : this->operand_stack) {
            std::cout << element.get_content().as_int << std::endl;
        }
    }

    StackElement get_variable(size_t id) const {
        size_t offset;
        if (this->call_stack.size() > 0) {
            offset = call_stack.back().get_local_var_offset();
        } else {
            offset = 0;
        }
        return this->local_vars[offset + id];
    }
    
    void set_variable(size_t id, StackElement value){
        size_t offset;
        if (this->call_stack.size() > 0) {
            offset = call_stack.back().get_local_var_offset();
        } else {
            offset = 0;
        }

        size_t index = offset + id;
        if (index >= this->local_vars.size()) {
            this->local_vars.resize(index+1, StackElement(StackElementType::PRIMITIVE, Word { .as_int = 0 }));
        }

        this->local_vars[index] = value;
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
                    void *address = this->pop_from_stack().get_content().as_pointer;
                    *(Word*)address = value;
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::READW:
                {
                    void *address = this->pop_from_stack().get_content().as_pointer;
                    Word value = *((Word*)address);
                    if (current_instruction.get_operand().as_int != 0) { // value that was read is an object
                        this->push_on_stack(StackElement(StackElementType::OBJECT, value));
                    } else {
                        this->push_on_stack(StackElement(StackElementType::PRIMITIVE, value));
                    }
                    this->instruction_pointer += 1;
                }
                break;

            case InstructionType::WRITEB:
                {
                    int64_t value = this->pop_from_stack().get_content().as_int & 0xFF;
                    char as_byte = (char) value;
                    void *address = this->pop_from_stack().get_content().as_pointer;
                    *(char*)address = as_byte;
                    this->instruction_pointer += 1;
                }
                break;

            case InstructionType::READB:
                {
                    void *address = this->pop_from_stack().get_content().as_pointer;
                    char value = *((char*)address);
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_int = (int64_t) value }));
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
            case InstructionType::IBNEG:
                {
                    int64_t operand = this->pop_from_stack().get_content().as_int;
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_int = ~operand }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::INEG:
                {
                    int64_t operand = this->pop_from_stack().get_content().as_int;
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_int = -operand }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::FNEG:
                {
                    double operand = this->pop_from_stack().get_content().as_float;
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_float = -operand }));
                    this->instruction_pointer += 1;
                }
                break;
            case InstructionType::LNEG:
                {
                    int64_t operand = this->pop_from_stack().get_content().as_int;
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_int = operand == 0 ? 1 : 0 }));
                    this->instruction_pointer += 1;
                }
                break;
            
#define BINARY_INT_INSTRUCTION(INST,OP) \
            case InstructionType:: INST : \
                { \
                    int64_t second_operand = this->pop_from_stack().get_content().as_int; \
                    int64_t first_operand = this->pop_from_stack().get_content().as_int; \
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_int = first_operand OP second_operand })); \
                    this->instruction_pointer += 1; \
                } \
                break; 

            BINARY_INT_INSTRUCTION(IADD, +)
            BINARY_INT_INSTRUCTION(ISUB, -)
            BINARY_INT_INSTRUCTION(IMUL, *)
            BINARY_INT_INSTRUCTION(IDIV, /) // TODO: Check for divide by zero
            BINARY_INT_INSTRUCTION(IMOD, %)

            BINARY_INT_INSTRUCTION(ISHL, <<)
            BINARY_INT_INSTRUCTION(ISHR, >>)
            BINARY_INT_INSTRUCTION(IAND, &)
            BINARY_INT_INSTRUCTION(IOR, |)
            BINARY_INT_INSTRUCTION(IXOR, ^)

#define BINARY_FLOAT_INSTRUCTION(INST,OP) \
            case InstructionType:: INST : \
                { \
                    double second_operand = this->pop_from_stack().get_content().as_float; \
                    double first_operand = this->pop_from_stack().get_content().as_float; \
                    this->push_on_stack(StackElement(StackElementType::PRIMITIVE, Word { .as_float = first_operand OP second_operand })); \
                    this->instruction_pointer += 1; \
                } \
                break; 
            
            BINARY_FLOAT_INSTRUCTION(FADD, +)
            BINARY_FLOAT_INSTRUCTION(FSUB, -)
            BINARY_FLOAT_INSTRUCTION(FMUL, *)
            BINARY_FLOAT_INSTRUCTION(FDIV, /) // TODO: Check for divide by zero
            
            case InstructionType::JUMP:
                {
                    size_t location = (size_t) current_instruction.get_operand().as_int;
                    this->instruction_pointer = location;
                }
                break;
            
            case InstructionType::JNEQ:
                {
                    Word second_operand = this->pop_from_stack().get_content();
                    Word first_operand = this->pop_from_stack().get_content();
                    if (first_operand.as_int != second_operand.as_int) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JEQ:
                {
                    Word second_operand = this->pop_from_stack().get_content();
                    Word first_operand = this->pop_from_stack().get_content();
                    if (first_operand.as_int == second_operand.as_int) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JEQZ:
                {
                    int64_t first_operand = this->pop_from_stack().get_content().as_int;
                    if (first_operand == 0) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JILT:
                {
                    int64_t second_operand = this->pop_from_stack().get_content().as_int;
                    int64_t first_operand = this->pop_from_stack().get_content().as_int;
                    if (first_operand < second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JILE:
                {
                    int64_t second_operand = this->pop_from_stack().get_content().as_int;
                    int64_t first_operand = this->pop_from_stack().get_content().as_int;
                    if (first_operand <= second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JIGT:
                {
                    int64_t second_operand = this->pop_from_stack().get_content().as_int;
                    int64_t first_operand = this->pop_from_stack().get_content().as_int;
                    if (first_operand > second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JIGE:
                {
                    int64_t second_operand = this->pop_from_stack().get_content().as_int;
                    int64_t first_operand = this->pop_from_stack().get_content().as_int;
                    if (first_operand >= second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JFLT:
                {
                    double second_operand = this->pop_from_stack().get_content().as_float;
                    double first_operand = this->pop_from_stack().get_content().as_float;
                    if (first_operand < second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JFLE:
                {
                    double second_operand = this->pop_from_stack().get_content().as_float;
                    double first_operand = this->pop_from_stack().get_content().as_float;
                    if (first_operand <= second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JFGT:
                {
                    double second_operand = this->pop_from_stack().get_content().as_float;
                    double first_operand = this->pop_from_stack().get_content().as_float;
                    if (first_operand > second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::JFGE:
                {
                    double second_operand = this->pop_from_stack().get_content().as_float;
                    double first_operand = this->pop_from_stack().get_content().as_float;
                    if (first_operand >= second_operand) {
                        size_t location = (size_t) current_instruction.get_operand().as_int;
                        this->instruction_pointer = location;
                    } else {
                        this->instruction_pointer += 1;
                    }
                }
                break;
            
            case InstructionType::VLOAD:
                {
                    size_t id = (size_t)current_instruction.get_operand().as_int;
                    StackElement variable_value = this->get_variable(id);
                    this->push_on_stack(variable_value);
                    this->instruction_pointer += 1;
                }
                break;
            
            case InstructionType::VWRITE:
                {
                    size_t id = (size_t)current_instruction.get_operand().as_int;
                    StackElement new_value = this->pop_from_stack();
                    this->set_variable(id, new_value);
                    this->instruction_pointer += 1;
                }
                break;

            case InstructionType::HALT:
                break;
            case InstructionType::LABEL:
                this->instruction_pointer += 1;
                break;
            default:
                std::cerr << "Not implemented: " << current_instruction.get_type() << std::endl;
                assert(false && "TODO");
                break;
        }
    }
};
