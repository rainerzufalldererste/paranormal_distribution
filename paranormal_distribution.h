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

namespace paranormal_distribution
{
namespace _internal
{
inline uint8_t reverse(const uint8_t n)
{
  static uint8_t lut[16] = { 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };
  return (lut[n & 0b1111] << 4) | lut[n >> 4];
}

inline uint8_t zzdec(const uint8_t val)
{
  return (uint8_t)(-(int8_t)(val & 1) ^ (val >> 1));
}
}

inline uint8_t to_paranormal(const uint8_t a, const uint8_t b)
{
  uint8_t v = (uint8_t)(a) * (uint8_t)(b | 0b10000000);
  v = _internal::zzdec(_internal::reverse((uint8_t)v)) - 128;
  v += (b >> 7);
  return v;
}

template <size_t bits>
inline uint8_t to_smooth_paranormal(const uint8_t a, const uint8_t b /* 0 ~ 127 (0 ~ 255 is also fine) */, const uint8_t c /* 0 ~ ((1 << bits) - 1) */)
{
  uint8_t v = (uint8_t)(a) * (uint8_t)(b | 0b10000000);
  v = _internal::zzdec(_internal::reverse((uint8_t)v)) - 128;
  v += (c - ((1 << (bits - 1)) - 1));
  return v;
}
}

#endif // paranormal_distribution_h__
