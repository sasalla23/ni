class CodeGenerator {
private:
    std::vector<Instruction> program;
    std::vector<char> static_data;
    size_t label_count;

    size_t break_label;
    size_t continue_label;
    size_t main_label;
    bool main_label_found;
public:
    CodeGenerator(size_t initial_label_count) :
        program(), static_data(), label_count(initial_label_count), break_label(0), continue_label(0), main_label(0), main_label_found(false)
    {}

    void push_instruction(Instruction instruction) {
        this->program.push_back(instruction);
    }

    std::vector<Instruction> get_program() {
        return std::move(this->program);
    }

    std::vector<char> get_static_data() {
        return std::move(this->static_data);
    }
    
    void set_break_label(size_t break_label) {
        this->break_label = break_label;
    }
    
    void set_continue_label(size_t continue_label) {
        this->continue_label = continue_label;
    }

    size_t get_break_label() const {
        return this->break_label;
    }
    
    size_t get_continue_label() const {
        return this->continue_label;
    }

    void set_main_label(size_t label) {
        this->main_label = label;
        this->main_label_found = true;
    }

    size_t allocate_static_objects(std::shared_ptr<ObjectLayout> layout, size_t count) {
        size_t allocated_bytes = layout->get_size() * count;
        size_t offset = this->static_data.size();
        this->static_data.resize(offset + allocated_bytes, 0);
        return offset;
    }

    void *get_static_data_pointer(size_t offset) {
        return this->static_data.data() + offset;
    }

    size_t generate_label() {
        size_t new_label = this->label_count;
        this->label_count += 1;
        return new_label;
    }

    bool is_jump_instruction(InstructionType type) {
        switch(type)  {
            case InstructionType::JUMP:
            case InstructionType::JNEQ:
            case InstructionType::JEQ:
            
            case InstructionType::JILT:
            case InstructionType::JILE:
            case InstructionType::JIGT:
            case InstructionType::JIGE:
            
            case InstructionType::JFLT:
            case InstructionType::JFLE:
            case InstructionType::JFGT:
            case InstructionType::JFGE:

            case InstructionType::CALL:
                return true;
            default:
                return false;
        }
    }

    void finalize() {
        // TODO: Check for this in type checker
        if (!this->main_label_found) {
            std::cerr << "GENERATION_ERROR: No main function defined..." << std::endl;
            std::exit(1);
        }

        this->program.insert(this->program.begin(), Instruction(InstructionType::JUMP, Word { .as_int = (int64_t) this->main_label }));
        //for (const auto& instruction : this->program) {
        //    std::cout << instruction << std::endl;
        //}

        std::vector<size_t> label_locations;
        label_locations.resize(this->label_count);
        for (size_t i = 0; i < this->program.size(); i++) {
            const Instruction& instruction = program[i];
            if (instruction.get_type() == InstructionType::LABEL) {
                label_locations[(size_t)instruction.get_operand().as_int] = i;
            }
        }

        for (auto& instruction : this->program) {
            if (this->is_jump_instruction(instruction.get_type())) {
                size_t label_index = (size_t)instruction.get_operand().as_int;
                instruction.set_operand(Word { .as_int = (int64_t) label_locations[label_index] });
            }
        }
    }
};


