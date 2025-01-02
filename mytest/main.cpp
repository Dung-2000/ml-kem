#include "ml_kem/internals/math/field.hpp"
#include "ml_kem/internals/poly/ntt.hpp"
#include "ml_kem/internals/poly/poly_vec.hpp"
#include <iostream>

using namespace ml_kem_ntt;

int main() {



  std::array<ml_kem_field::zq_t, N> f;
  std::array<ml_kem_field::zq_t, N> g;

  // Initialize the polynomials with some values.
  for (size_t i = 0; i < N; i++) {
    f[i] = ml_kem_field::zq_t(i);
    g[i] = ml_kem_field::zq_t(i + 1);
  }
  for(size_t i = 0; i < N; i++) {
    std::span<ml_kem_field::zq_t> vec=f;
    ml_kem_utils::poly_vec_ntt(vec);

  }

  std::cout << std::endl << "print original f value" << std::endl;
  for (size_t i = 1; i <= N; i++) {
    std::cout << f[i-1] << " ";
    if(i%8 == 0) std::cout << std::endl;
  }
  std::cout << std::endl << "print original g value" << std::endl;
 for (size_t i = 1; i <= N; i++) {
    std::cout << g[i-1] << " ";
    if(i%8 == 0) std::cout << std::endl;
  }
  // Convert the polynomials to NTT form.
  ntt(f);
  std::cout << "print NTT result f'" << std::endl;
  for (size_t i = 1; i <= N; i++) {
    std::cout << f[i-1] << " ";
    if(i%8 == 0) std::cout << std::endl;
  }
  ntt(g);

  // Multiply the polynomials in NTT form.
  std::array<ml_kem_field::zq_t, N> h;
  polymul(f, g, h);
  std::cout << "print NTT dot product result f'.g'=h'" << std::endl;
  for (size_t i = 1; i <= N; i++) {
    std::cout << h[i-1] << " ";
    if(i%8 == 0) std::cout << std::endl;
  }
  // Convert the result back to the original domain.
  intt(h);

  // Print the product polynomial.
  std::cout << "print INTT result h 123" << std::endl;
  for (size_t i = 1; i <= N; i++) {
    std::cout << h[i-1] << " ";
    if(i%8 == 0) std::cout << std::endl;
  }

  return 0;
}

/*

#include "ml_kem_ntt.hpp"

#include <gtest/gtest.h>

using namespace ml_kem_ntt;

TEST(NttTest, Basic) {
  // Define a degree-255 polynomial.
  std::array<ml_kem_field::zq_t, N> f;

  // Initialize the polynomial with some values.
  for (size_t i = 0; i < N; i++) {
    f[i] = ml_kem_field::zq_t(i);
  }

  // Convert the polynomial to NTT form.
  ntt(f);

  // Convert the polynomial back to the original domain.
  intt(f);

  // Check that the polynomial is unchanged.
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(f[i], ml_kem_field::zq_t(i));
  }
}

TEST(NttTest, Multiplication) {
  // Define two degree-255 polynomials.
  std::array<ml_kem_field::zq_t, N> f;
  std::array<ml_kem_field::zq_t, N> g;

  // Initialize the polynomials with some values.
  for (size_t i = 0; i < N; i++) {
    f[i] = ml_kem_field::zq_t(i);
    g[i] = ml_kem_field::zq_t(i + 1);
  }

  // Convert the polynomials to NTT form.
  ntt(f);
  ntt(g);

  // Multiply the polynomials in NTT form.
  std::array<ml_kem_field::zq_t, N> h;
  polymul(f, g, h);

  // Convert the result back to the original domain.
  intt(h);

  // Check that the product polynomial is correct.
  for (size_t i = 0; i < N; i++) {
    ml_kem_field::zq_t expected = 0;
    for (size_t j = 0; j < N; j++) {
      expected += f[j] * g[(i - j + N) % N];
    }
    EXPECT_EQ(h[i], expected);
  }
}
*/
