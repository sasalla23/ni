#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <cstring>

void indent_layer(std::ostream& output_stream, size_t layer) {
    for (size_t i = 0; i < layer; i++) {
        output_stream << "\t";
    }
}


#include "virtual_machine.cpp"
#include "tokenizer.cpp"
#include "type.cpp"
#include "type_annotation.cpp"
#include "type_checker.cpp"
#include "code_generator.cpp"
#include "expression.cpp"
#include "statement.cpp"
#include "global_definition.cpp"
#include "parser.cpp"

int main(int argc, const char **argv) {
    if (argc < 2) {
        std::cerr << "ERROR: Not enough arguments" << std::endl;
        std::cerr << "USAGE: " << argv[0] << " [input.ni]" << std::endl;
        std::exit(1);
    }

    Tokenizer tokenizer(argv[1]);
    auto tokens = tokenizer.collect_tokens();
    Parser parser(std::move(tokens));

    auto global_definitions = parser.parse_file();
    
    for (auto& global_definition : global_definitions) {
        global_definition->first_pass();
        //std::cout << *global_definition;
    }

    for (auto& global_definition : global_definitions) {
        global_definition->type_check();
        //std::cout << *global_definition;
    }

    CodeGenerator code_generator(TypeChecker::get().get_function_count());
    for (auto& global_definition : global_definitions) {
        global_definition->emit(code_generator);
        //std::cout << *global_definition;
    }
    code_generator.finalize();

    VirtualMachine virtual_machine(std::move(code_generator.get_program()), std::move(code_generator.get_static_data()));
    virtual_machine.execute();

    return 0;
}
