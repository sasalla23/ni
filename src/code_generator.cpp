class CodeGenerator {
private:
    std::vector<Instruction> program;
    std::vector<char> static_data;
public:
    CodeGenerator() : program(), static_data() {}

    void push_instruction(Instruction instruction) {
        this->program.push_back(instruction);
    }

    std::vector<Instruction> get_program() {
        return std::move(this->program);
    }

    std::vector<char> get_static_data() {
        return std::move(this->static_data);
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
};


