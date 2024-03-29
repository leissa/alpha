#pragma once

#include <cassert>
#include <cstdint>

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

using namespace std::string_literals;

/// @name helpers
///@{
template<class T, class... Args> auto mku(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
template<class T, class... Args> auto mks(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

template<class T> using UPtr = std::unique_ptr<T>;
template<class T> using SPtr = std::shared_ptr<T>;

template<class P>
struct Dump {
    void dump() const { std::cout << static_cast<const P*>(this)->str() << std::endl; }
};
///@}

/// @name hash
///@{
constexpr uint64_t operator""_u64(unsigned long long int u) { return uint64_t(u); }
using hash_t = uint64_t;

/// @name FNV-1/FNV-1a hash
/// See [Wikipedia](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash).
/// [Magic numbers](http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-var) for FNV-1 hash.
struct FNV1 {
    static const hash_t offset = 14695981039346656037_u64;
    static const hash_t prime  = 1099511628211_u64;
};

/// Returns a new hash by combining the hash @p seed with @p val.
template<class T>
hash_t hash_combine(hash_t seed, T v) {
    static_assert(std::is_signed<T>::value || std::is_unsigned<T>::value, "please provide your own hash function");

    hash_t val = v;
    for (hash_t i = 0; i < sizeof(T); ++i) {
        hash_t octet = val & 0xff_u64; // extract lower 8 bits
        seed ^= octet;
        seed *= FNV1::prime;
        val >>= 8_u64;
    }
    return seed;
}

template<class T, class... Args>
hash_t hash_combine(hash_t seed, T val, Args&&... args) {
    return hash_combine(hash_combine(seed, val), std::forward<Args>(args)...);
}

template<class T>
hash_t hash_begin(T val) {
    return hash_combine(FNV1::offset, val);
}
inline hash_t hash_begin() { return FNV1::offset; }
///@}

