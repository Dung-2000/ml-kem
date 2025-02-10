#include <stdio.h>

// Function to compute the bit-reversed value of a 7-bit number
unsigned int bitreverse7(unsigned int num) {
    unsigned int result = 0;
    for (int i = 0; i < 7; i++) {
        result <<= 1;          // Shift result left by 1 bit
        result |= (num & 1);   // Add the least significant bit of num to result
        num >>= 1;             // Shift num right by 1 bit
    }
    return result;
}

int main() {
    // Compute and display the bit-reversed values for i = 0 to 127
    for (unsigned int i = 1; i < 128; i++) {
        printf("NTT->i = %3u, bitreverse7(i) = %3u\n", i, bitreverse7(i));
    }

    for (unsigned int i = 127; i > 0; i--) {
        printf("i = %3u, bitreverse7(i) = %3u\n", i, bitreverse7(i));
    }
    return 0;
}
