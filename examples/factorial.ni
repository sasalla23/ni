fun fac(n: int): int {
    if (n <= 1) {
        return 1;
    } else {
        return n*fac(n-1);
    }
}

fun main(): void {
    print_line(#string fac(10));
}
