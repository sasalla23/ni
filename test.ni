fun fibonnaci(n: int): int {
    var a = 0;
    var b = 1;

    var counter = 0;

    while (counter < n) {
        var temp = a + b;
        a = b;
        b = temp;
        counter = counter + 1;
    }

    return a;
}

fun compare_strings(a: string, b: string): bool {
    if (a.length != b.length) {
        return false;
    }

    var index = 0;
    while (index < a.length) {
        if (a[index] != b[index]) {
            return false;
        }
        index = index + 1;
    }

    return true;
}


fun main(args: [string]): void {
    if (args.length < 1) {
        print_line("That's not enough arguments!");
    }

    if (compare_strings(args[0], "23")) {
        print("fib(23) = ");
        print_line(#string fibonnaci(23));
    } else {
        print_line("Supply a cool number.");
    }
}
