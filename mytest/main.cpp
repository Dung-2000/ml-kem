#include <iostream>
#include <array>
#include "ml_kem/internals/math/field.hpp"
#include "ml_kem/internals/poly/ntt.hpp"

int main() {
    constexpr size_t N = ml_kem_ntt::N;
    using zq_t = ml_kem_field::zq_t;

    // Represent two big numbers as polynomials
    std::array<zq_t, N> num1{}, num2{}, result{};

    // Initialize num1 and num2 as coefficients of big numbers
    // Example: num1 = 1234 -> [4, 3, 2, 1], num2 = 5678 -> [8, 7, 6, 5]
    num1[0] = 4; num1[1] = 3; num1[2] = 2; num1[3] = 1;
    num2[0] = 8; num2[1] = 7; num2[2] = 6; num2[3] = 5;

    // Perform forward NTT on both num1 and num2
    std::cout << "test NTT INTT \n"; 
    ml_kem_ntt::ntt(num1);
    ml_kem_ntt::intt(num1);
    std::cout << "start test Multiply with NTT\n"; 
    ml_kem_ntt::ntt(num1);
    ml_kem_ntt::ntt(num2);

    // std::cout << "print out the intermedia result ntt(num1): ";
    // for (size_t i = 0; i < N; ++i) {
    //     if (num1[i] != zq_t::zero()) { // Print only non-zero coefficients
    //         std::cout << num1[i] << " ";
    //     }
    // }
    // std::cout << std::endl;

    // Multiply in the NTT domain
    ml_kem_ntt::polymul(num1, num2, result);

    // Perform inverse NTT on the result
    ml_kem_ntt::intt(result);

    // Output the resulting coefficients
    // std::cout << "Resulting coefficients (polynomial form): ";
    // for (size_t i = 0; i < N; ++i) {
    //     if (result[i] != zq_t::zero()) { // Print only non-zero coefficients
    //         std::cout << '(' << result[i]  <<", " << i << ')';
    //     }
    // }
    // std::cout << std::endl;
  for(size_t i = 0; i < N; i++) {
    std::cout << "(i, " << result[i] << "), ";  
  }
  std::cout << '\n'; 

    return 0;
}
