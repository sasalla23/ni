fun main(): void {
    var i = 0;
    while (i < 10) {
        var fib_i = fib(i);
        var fac_i = fac(i);
        
        "==============".print_line();

        print("Iteration: ");
        print_line(#string i);

        print("Fibonacci: ");
        print_line(#string fib_i);

        print("Factorial: ");
        print_line(#string fac_i);

        i = i + 1;
    }
}

fun fib(n: int): int {
    var iteration = 0;
    var a = 0;
    var b = 1;

    while (iteration < n) {
        var temp = b;
        b = a + b;
        a = temp;

        iteration = iteration + 1;
    }

    return a;
}

fun fac(n: int): int {
    if (n <= 1) {
        return 1;
    } else {
        return n * fac(n-1);
    }
}

