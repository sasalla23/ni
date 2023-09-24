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
#include "expression.cpp"
#include "statement.cpp"
#include "parser.cpp"

int main(void) {
    Tokenizer tokenizer("test.ni");
    auto tokens = tokenizer.collect_tokens();
    Parser parser(tokens);
    auto statement = parser.parse_statement();
    std::cout << *statement;

    return 0;
}
