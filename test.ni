{
    var text = "Foo, Bar";
    var index = 0;

    while (index < text.length) {
        print(#string index);
        print(": ");
        (#string text[index]).print_line();

        index = index + 1;
    }
}
