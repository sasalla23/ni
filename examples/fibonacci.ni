fun fib(n: int): int {
    var a = 0;
    var b = 1;
    var iteration = 0;

    while (iteration < n) {
        var temp = b;
        b = a + b;
        a = temp;

        iteration = iteration + 1;
    }

    return a;
}

fun main(): void {
    print_line(#string fib(10));
}
