# ni-lang
A compiler for a simple turing complete programming language written in c++. The language at hand is called 'ni' and
is a statically typed interpreted language.

## Quick Start (Linux)
``` console
$ ./build.sh
$ ./main [input.ni]
```

## Syntax
Ni follows a simple c-like syntax with a couple of adjustments. 
### Hello World
``` kotlin
fun main(): void {
    print_line("Hello, World");
}
```
### Variables
``` kotlin
var x = 23;
x = x * x;
print_line(#string x * x);
```
### Strings
``` kotlin
var text = "Hello";
var firt_char = text[0];
var text_length = text.length;
```
### Lists
``` kotlin
var xs = [1,2,3];
var first_element = xs[0];
xs[1] = 5;
var list_length = xs.length;
```
### Types
``` kotlin
var x: int = 23;
var y: float = 23.5;
var z: char = 'H';
var a: string = "Hello, World";
var b: [int] = [1,2,3];
```
### Functions
``` kotlin
fun add(x: int, y: int): int {
    return x + y;
}

fun main(): void {
    var a = 23;
    var b = 2 * a;
    var c = add(a,b);

    // Method calls are just regular function calls
    var d = a.add(b);
}
```
