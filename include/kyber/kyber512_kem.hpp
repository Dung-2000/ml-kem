#pragma once
#include "kyber/internals/kem.hpp"

// Kyber Key Encapsulation Mechanism (KEM) instantiated with Kyber512 parameters
namespace kyber512_kem {

// See row 1 of table 1 of specification @
// https://doi.org/10.6028/NIST.FIPS.203.ipd

constexpr size_t k = 2;
constexpr size_t η1 = 3;
constexpr size_t η2 = 2;
constexpr size_t du = 10;
constexpr size_t dv = 4;

// = 800 -bytes Kyber512 public key
constexpr size_t PKEY_BYTE_LEN = kyber_utils::get_kem_public_key_len(k);

// = 1632 -bytes Kyber512 secret key
constexpr size_t SKEY_BYTE_LEN = kyber_utils::get_kem_secret_key_len(k);

// = 768 -bytes Kyber512 cipher text length
constexpr size_t CIPHER_TEXT_BYTE_LEN = kyber_utils::get_kem_cipher_text_len(k, du, dv);

// = 32 -bytes Kyber512 fixed size shared secret byte length
constexpr size_t SHARED_SECRET_BYTE_LEN = 32;

// Computes a new Kyber512 KEM keypair s.t. public key is 800 -bytes and secret
// key is 1632 -bytes, given 32 -bytes seed d ( used in CPA-PKE ) and 32 -bytes
// seed z ( used in CCA-KEM ).
inline void
keygen(std::span<const uint8_t, 32> d, std::span<const uint8_t, 32> z, std::span<uint8_t, PKEY_BYTE_LEN> pubkey, std::span<uint8_t, SKEY_BYTE_LEN> seckey)
{
  kem::keygen<k, η1>(d, z, pubkey, seckey);
}

// Given 32 -bytes seed m ( which is used during encapsulation ) and a Kyber512
// KEM public key ( of 800 -bytes ), this routine computes a SHAKE256 XOF backed
// KDF (key derivation function) and 768 -bytes of cipher text, which can only
// be decrypted by corresponding Kyber512 KEM secret key, for arriving at same
// SHAKE256 XOF backed KDF.
//
// Returned KDF can be used for deriving shared key of arbitrary bytes length.
[[nodiscard("If public key is malformed, encapsulation fails")]] inline bool
encapsulate(std::span<const uint8_t, 32> m,
            std::span<const uint8_t, PKEY_BYTE_LEN> pubkey,
            std::span<uint8_t, CIPHER_TEXT_BYTE_LEN> cipher,
            std::span<uint8_t, SHARED_SECRET_BYTE_LEN> shared_secret)
{
  return kem::encapsulate<k, η1, η2, du, dv>(m, pubkey, cipher, shared_secret);
}

// Given a Kyber512 KEM secret key ( of 1632 -bytes ) and a cipher text of 768
// -bytes, which holds encrypted ( using corresponding Kyber512 KEM public key )
// 32 -bytes seed, this routine computes a SHAKE256 XOF backed KDF (key
// derivation function).
//
// Returned KDF can be used for deriving shared key of arbitrary bytes length.
inline void
decapsulate(std::span<const uint8_t, SKEY_BYTE_LEN> seckey, std::span<const uint8_t, CIPHER_TEXT_BYTE_LEN> cipher, std::span<uint8_t, SHARED_SECRET_BYTE_LEN> shared_secret)
{
  kem::decapsulate<k, η1, η2, du, dv>(seckey, cipher, shared_secret);
}

}
