class CodeGenerator {
private:
    std::vector<Instruction> program;
public:
    CodeGenerator() : program() {}

    void push_instruction(Instruction instruction) {
        this->program.push_back(instruction);
    }

    std::vector<Instruction> get_program() {
        return std::move(this->program);
    }
};
