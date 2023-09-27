#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <unordered_map>

void indent_layer(std::ostream& output_stream, size_t layer) {
    for (size_t i = 0; i < layer; i++) {
        output_stream << "\t";
    }
}

#include "tokenizer.cpp"
#include "type.cpp"
#include "type_annotation.cpp"
#include "type_checker.cpp"
#include "expression.cpp"
#include "statement.cpp"
#include "global_definition.cpp"
#include "parser.cpp"

int main(void) {
    Tokenizer tokenizer("test.ni");
    auto tokens = tokenizer.collect_tokens();
    Parser parser(std::move(tokens));

    TypeChecker type_checker;

    auto expression = parser.parse_expression();
    expression->type_check(type_checker);
    std::cout << *expression;
    std::cout << expression->get_type()->to_string() << std::endl;
    //auto global_definitions = parser.parse_file();
    //for (auto& global_definition : global_definitions) {
    //    std::cout << *global_definition;
    //}

    return 0;
}
