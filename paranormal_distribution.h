// Copyright (c) 2025, Christoph Stiller. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef paranormal_distribution_h__
#define paranormal_distribution_h__

#include <stdint.h>
#include <stddef.h>
#include <bit>

namespace paranormal_distribution
{
  namespace _internal
  {
    inline uint8_t reverse_bits(const uint8_t n)
    {
      static uint8_t lut[16] = { 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };
      return (lut[n & 0b1111] << 4) | lut[n >> 4];
    }

    inline uint8_t decode_zigzag(const uint8_t val)
    {
      return (uint8_t)(-(int8_t)(val & 1) ^ (val >> 1));
    }
  }

  // retrieves a 1-bit smoothed paranormal distribution from 2 random bytes.
  inline int8_t to_paranormal(const uint8_t a, const uint8_t b)
  {
    uint8_t v = (uint8_t)(a) * (uint8_t)(b | 0b10000000);
    v = _internal::decode_zigzag(_internal::reverse_bits(v));
    v += (b >> 7); // without this, the distribution would be skewed towards -1.
    return (int8_t)v;
  }

  // retrieves a smoothed paranormal distribution from 3 random values assuming c is in the specified range.
  template <uint8_t smoothing_bits>
  inline int8_t to_smooth_paranormal(const uint8_t a, const uint8_t b /* 0 ~ 127 (0 ~ 255 is also fine) */, const uint8_t c /* 0 ~ ((1 << bits) - 1) */)
  {
    static_assert(smoothing_bits >= 2 && smoothing_bits <= 7, "smoothing bits is valid between 2 and 7");

    uint8_t v = (uint8_t)(a) * (uint8_t)(b | 0b10000000);
    v = _internal::decode_zigzag(_internal::reverse_bits(v));
    v += (c - (uint8_t)((1ULL << (smoothing_bits - 1)) - 1));
    return (int8_t)v;
  }

  // retrieves a smoothed paranormal distribution from 3 random bytes.
  template <uint8_t smoothing_bits>
  inline int8_t to_smooth_paranormal_safe(const uint8_t a, const uint8_t b, const uint8_t c)
  {
    static_assert(smoothing_bits >= 2 && smoothing_bits <= 7, "smoothing bits is valid between 2 and 7");
    return to_smooth_paranormal<smoothing_bits>(a, b, c & (uint8_t)((1ULL << smoothing_bits) - 1));
  }

  // retrieves a smoothed paranormal2 distribution from 2 random values assuming b is in the specified range.
  template <uint8_t smoothing_bits>
  inline int8_t to_paranormal2(const uint8_t a, const uint8_t b /* 0 ~ ((1 << bits) - 1) */)
  {
    static_assert(smoothing_bits >= 2 && smoothing_bits <= 7, "smoothing bits is valid between 2 and 7");

    uint8_t v = (uint8_t)(a >> std::popcount(a));
    v = _internal::decode_zigzag(v);

    if constexpr (smoothing_bits <= 3)
      v += 2;
    else if constexpr (smoothing_bits > 4)
      v++;

    v += ((b >> 1) - (uint8_t)((1ULL << (smoothing_bits - 2))));

    v ^= (b & 1) - 1;
    v += (b & 1);

    return (int8_t)v;
  }

  // retrieves a smoothed paranormal2 distribution from 2 random bytes.
  template <uint8_t smoothing_bits>
  inline int8_t to_smooth_paranormal2_safe(const uint8_t a, const uint8_t b)
  {
    static_assert(smoothing_bits >= 2 && smoothing_bits <= 7, "smoothing bits is valid between 2 and 7");
    return to_paranormal2<smoothing_bits>(a, b & (uint8_t)((1ULL << smoothing_bits) - 1));
  }

  // retrieves a tiny_paranormal distribution from 1 random value.
  inline int8_t to_tiny_paranormal(const uint8_t a)
  {
    uint8_t v = a >> 2;
    v = (uint8_t)(v >> std::popcount(v));
    v = _internal::decode_zigzag(v);
    v += ((a & 3) >> 1);
    v ^= (a & 1) - 1;
    v += (a & 1);
    return (int8_t)v;
  }
}

#endif // paranormal_distribution_h__
