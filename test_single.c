#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int add_numbers(int a, int b) {
    return a + b;
}

EMSCRIPTEN_KEEPALIVE
int multiply_numbers(int a, int b) {
    return a * b;
}

EMSCRIPTEN_KEEPALIVE
void print_hello() {
    // Simple function without printf
    return;
}