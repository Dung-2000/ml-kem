#pragma once
#include "ml_kem/internals/math/field.hpp"
#include "ml_kem/internals/poly/poly_vec.hpp"
#include "ml_kem/internals/poly/sampling.hpp"
#include "ml_kem/internals/utility/params.hpp"
#include "ml_kem/internals/utility/utils.hpp"
#include "sha3_512.hpp"

// Public Key Encryption Scheme
namespace k_pke {

// K-PKE key generation algorithm, generating byte serialized public key and secret keym given a 32 -bytes input seed `d`.
// See algorithm 12 of K-PKE specification https://doi.org/10.6028/NIST.FIPS.203.ipd.
template<size_t k, size_t eta1>
static inline void
keygen(std::span<const uint8_t, 32> d, std::span<uint8_t, k * 12 * 32 + 32> pubkey, std::span<uint8_t, k * 12 * 32> seckey)
  requires(ml_kem_params::check_keygen_params(k, eta1))
{
  std::array<uint8_t, 64> g_out{};
  auto _g_out = std::span(g_out);

  sha3_512::sha3_512_t h512;
  h512.absorb(d);
  h512.finalize();
  h512.digest(_g_out);

  const auto rho = _g_out.template subspan<0, 32>();
  const auto sigma = _g_out.template subspan<rho.size(), 32>();

  std::array<ml_kem_field::zq_t, k * k * ml_kem_ntt::N> A_prime{};
  ml_kem_utils::generate_matrix<k, false>(A_prime, rho);

  uint8_t N = 0;

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> s{};
  ml_kem_utils::generate_vector<k, eta1>(s, sigma, N);
  N += k;

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> e{};
  ml_kem_utils::generate_vector<k, eta1>(e, sigma, N);
  N += k;

  ml_kem_utils::poly_vec_ntt<k>(s);
  ml_kem_utils::poly_vec_ntt<k>(e);

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> t_prime{};

  ml_kem_utils::matrix_multiply<k, k, k, 1>(A_prime, s, t_prime);
  ml_kem_utils::poly_vec_add_to<k>(e, t_prime);

  constexpr size_t pkoff = k * 12 * 32;
  auto _pubkey0 = pubkey.template subspan<0, pkoff>();
  auto _pubkey1 = pubkey.template subspan<pkoff, 32>();

  ml_kem_utils::poly_vec_encode<k, 12>(t_prime, _pubkey0);
  std::copy(rho.begin(), rho.end(), _pubkey1.begin());
  ml_kem_utils::poly_vec_encode<k, 12>(s, seckey);
}

// Given a *valid* K-PKE public key, 32 -bytes message ( to be encrypted ) and 32 -bytes random coin
// ( from where all randomness is deterministically sampled ), this routine encrypts message using
// K-PKE encryption algorithm, computing compressed cipher text.
//
// If modulus check, as described in point (2) of section 6.2 of ML-KEM draft standard, fails, it returns false.
//
// See algorithm 13 of K-PKE specification https://doi.org/10.6028/NIST.FIPS.203.ipd.
template<size_t k, size_t eta1, size_t eta2, size_t du, size_t dv>
[[nodiscard("Use result of modulus check on public key")]] static inline bool
encrypt(std::span<const uint8_t, k * 12 * 32 + 32> pubkey,
        std::span<const uint8_t, 32> msg,
        std::span<const uint8_t, 32> rcoin,
        std::span<uint8_t, k * du * 32 + dv * 32> enc)
  requires(ml_kem_params::check_encrypt_params(k, eta1, eta2, du, dv))
{
  constexpr size_t pkoff = k * 12 * 32;
  auto _pubkey0 = pubkey.template subspan<0, pkoff>();
  auto rho = pubkey.template subspan<pkoff, 32>();

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> t_prime{};
  std::array<uint8_t, _pubkey0.size()> encoded_tprime{};

  ml_kem_utils::poly_vec_decode<k, 12>(_pubkey0, t_prime);
  ml_kem_utils::poly_vec_encode<k, 12>(t_prime, encoded_tprime);

  using encoded_pkey_t = std::span<const uint8_t, _pubkey0.size()>;
  const auto are_equal = ml_kem_utils::ct_memcmp(encoded_pkey_t(_pubkey0), encoded_pkey_t(encoded_tprime));
  if (are_equal == 0u) {
    // Got an invalid public key
    return false;
  }

  std::array<ml_kem_field::zq_t, k * k * ml_kem_ntt::N> A_prime{};
  ml_kem_utils::generate_matrix<k, true>(A_prime, rho);

  uint8_t N = 0;

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> r{};
  ml_kem_utils::generate_vector<k, eta1>(r, rcoin, N);
  N += k;

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> e1{};
  ml_kem_utils::generate_vector<k, eta2>(e1, rcoin, N);
  N += k;

  std::array<ml_kem_field::zq_t, ml_kem_ntt::N> e2{};
  ml_kem_utils::generate_vector<1, eta2>(e2, rcoin, N);

  ml_kem_utils::poly_vec_ntt<k>(r);

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> u{};

  ml_kem_utils::matrix_multiply<k, k, k, 1>(A_prime, r, u);
  ml_kem_utils::poly_vec_intt<k>(u);
  ml_kem_utils::poly_vec_add_to<k>(e1, u);

  std::array<ml_kem_field::zq_t, ml_kem_ntt::N> v{};

  ml_kem_utils::matrix_multiply<1, k, k, 1>(t_prime, r, v);
  ml_kem_utils::poly_vec_intt<1>(v);
  ml_kem_utils::poly_vec_add_to<1>(e2, v);

  std::array<ml_kem_field::zq_t, ml_kem_ntt::N> m{};
  ml_kem_utils::decode<1>(msg, m);
  ml_kem_utils::poly_decompress<1>(m);
  ml_kem_utils::poly_vec_add_to<1>(m, v);

  constexpr size_t encoff = k * du * 32;
  auto _enc0 = enc.template subspan<0, encoff>();
  auto _enc1 = enc.template subspan<encoff, dv * 32>();

  ml_kem_utils::poly_vec_compress<k, du>(u);
  ml_kem_utils::poly_vec_encode<k, du>(u, _enc0);

  ml_kem_utils::poly_compress<dv>(v);
  ml_kem_utils::encode<dv>(v, _enc1);

  return true;
}

// Given K-PKE secret key and cipher text, this routine recovers 32 -bytes plain text which
// was encrypted using K-PKE public key i.e. associated with this secret key.
//
// See algorithm 14 defined in K-PKE specification https://doi.org/10.6028/NIST.FIPS.203.ipd.
template<size_t k, size_t du, size_t dv>
static inline void
decrypt(std::span<const uint8_t, k * 12 * 32> seckey, std::span<const uint8_t, k * du * 32 + dv * 32> enc, std::span<uint8_t, 32> dec)
  requires(ml_kem_params::check_decrypt_params(k, du, dv))
{
  constexpr size_t encoff = k * du * 32;
  auto _enc0 = enc.template subspan<0, encoff>();
  auto _enc1 = enc.template subspan<encoff, dv * 32>();

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> u{};

  ml_kem_utils::poly_vec_decode<k, du>(_enc0, u);
  ml_kem_utils::poly_vec_decompress<k, du>(u);

  std::array<ml_kem_field::zq_t, ml_kem_ntt::N> v{};

  ml_kem_utils::decode<dv>(_enc1, v);
  ml_kem_utils::poly_decompress<dv>(v);

  std::array<ml_kem_field::zq_t, k * ml_kem_ntt::N> s_prime{};
  ml_kem_utils::poly_vec_decode<k, 12>(seckey, s_prime);

  ml_kem_utils::poly_vec_ntt<k>(u);

  std::array<ml_kem_field::zq_t, ml_kem_ntt::N> t{};

  ml_kem_utils::matrix_multiply<1, k, k, 1>(s_prime, u, t);
  ml_kem_utils::poly_vec_intt<1>(t);
  ml_kem_utils::poly_vec_sub_from<1>(t, v);

  ml_kem_utils::poly_compress<1>(v);
  ml_kem_utils::encode<1>(v, dec);
}

}
