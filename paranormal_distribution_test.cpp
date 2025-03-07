// Copyright (c) 2025, Christoph Stiller. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "paranormal_distribution.h"

#include <stdio.h>
#include <inttypes.h>

#include <algorithm>
#include <bit>
#include <chrono>

#if defined(_MSC_VER) && !defined(__llvm__)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

//////////////////////////////////////////////////////////////////////////

enum class console_color : uint8_t
{
  Black,
  DarkRed,
  DarkGreen,
  DarkYellow,
  DarkBlue,
  DarkMagenta,
  DarkCyan,
  BrightGray,
  DarkGray,
  BrightRed,
  BrightGreen,
  BrightYellow,
  BrightBlue,
  BrightMagenta,
  BrightCyan,
  White
};

static void set_console_color(const console_color fg, const console_color bg = console_color::Black)
{
  const size_t fgColour = ((size_t)fg & 0xF);
  const size_t bgColour = ((size_t)bg & 0xF);

  printf("\x1b[%" PRIu64 ";%" PRIu64 "m", fgColour < 0x8 ? (30 + fgColour) : (90 - 8 + fgColour), bgColour < 0x8 ? (40 + bgColour) : (100 - 8 + bgColour));
}

static void reset_console_color()
{
  fputs("\x1b[0m", stdout);
}

static void print_histogram(const size_t(&hist)[256])
{
  size_t totalSamples = 0;
  int64_t valueSum = 0;
  int64_t valueSqSum = 0;
  size_t maxHeight = 1;

  for (size_t i = 0; i < std::size(hist); i++)
  {
    const size_t samples = hist[i];
    maxHeight = std::max(maxHeight, samples);
    totalSamples += samples;
    const int64_t val = (int64_t)(int8_t)i;
    valueSum += samples * val;
    valueSqSum += samples * val * val;
  }

  const double mean = valueSum / (double)totalSamples;
  const double stdDev = std::sqrt(valueSqSum / (double)totalSamples - mean * mean);

  printf("Histogram: (mean: %f, std dev: %f)\n", mean, stdDev);

  constexpr bool printValues = false;

  if constexpr (printValues)
  {
    for (int64_t i = std::numeric_limits<int8_t>::min(); i <= std::numeric_limits<int8_t>::max(); i++)
      printf("%" PRIu64 ", ", hist[(uint8_t)(int8_t)i]);

    puts("");
  }

  constexpr size_t barHeight = 32;
  constexpr size_t displayFactor = 4;
  const size_t heightDiv = std::max<size_t>(1, std::round(maxHeight / (double)barHeight));
  const console_color rangeColors[2] = { console_color::BrightYellow, console_color::BrightCyan };

  for (int64_t i = barHeight; i >= 0; i--)
  {
    const size_t displayScale = (size_t)i * displayFactor;

    for (int64_t j = std::numeric_limits<int8_t>::min(); j <= std::numeric_limits<int8_t>::max(); j++)
    {
      set_console_color(rangeColors[std::countl_zero((uint8_t)std::abs(j)) & 1]);

      const size_t div = (hist[(uint8_t)(int8_t)j] * displayFactor) / heightDiv;
      size_t rem = 0;

      if (div > displayScale)
        rem = div - displayScale;

      switch (rem)
      {
      case 0: fputc(' ', stdout); break;
      case 1: fputc('_', stdout); break;
      case 2: fputc('.', stdout); break;
      case 3: fputc('o', stdout); break;
      default:fputc('#', stdout); break;
      }
    }

    puts("");
  }

  reset_console_color();
}

inline int64_t get_current_time_ns()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

//////////////////////////////////////////////////////////////////////////

void test_raw_distribution()
{
  size_t hist[256] = {};
  const int64_t startNs = get_current_time_ns();
  const int64_t startTicks = __rdtsc();

  for (size_t a = 0; a < 256; a++)
    for (size_t b = 0; b < 256; b++)
      hist[(uint8_t)paranormal_distribution::to_paranormal((uint8_t)a, (uint8_t)b)]++;

  const int64_t endTicks = __rdtsc();
  const int64_t endNs = get_current_time_ns();
  constexpr size_t iterationCount = 256 * 256;
  const size_t totalNs = endNs - startNs;
  const size_t totalTicks = endTicks - startTicks;

  puts("Plain Paranormal Distribution (w/o Additional Smoothing)");
  print_histogram(hist);
  printf("\nThrouhgput (including loop): %3.1f its/ms = %3.1f ns/call, %4.2f ticks/it\n\n", iterationCount / (totalNs * 1e-6), (double)totalNs / iterationCount, (double)totalTicks / iterationCount);
}

template <uint8_t smoothing_bits>
void test_smooth_distribution()
{
  size_t hist[256] = {};
  const int64_t startNs = get_current_time_ns();
  const int64_t startTicks = __rdtsc();

  for (size_t a = 0; a < 256; a++)
    for (size_t b = 0; b < 256; b++)
      for (size_t c = 0; c < (1ULL << smoothing_bits); c++)
        hist[(uint8_t)paranormal_distribution::to_smooth_paranormal<smoothing_bits>((uint8_t)a, (uint8_t)b, (uint8_t)c)]++;

  const int64_t endTicks = __rdtsc();
  const int64_t endNs = get_current_time_ns();
  constexpr size_t iterationCount = 256 * 256 * (1ULL << smoothing_bits);
  const size_t totalNs = endNs - startNs;
  const size_t totalTicks = endTicks - startTicks;

  printf("Smooth Paranormal Distribution (%" PRIu8 " Bit Smoothing)\n", (uint8_t)smoothing_bits);
  print_histogram(hist);
  printf("\nThrouhgput (including loop): %3.1f its/ms = %3.1f ns/call, %4.2f ticks/it\n\n", iterationCount / (totalNs * 1e-6), (double)totalNs / iterationCount, (double)totalTicks / iterationCount);
}

template <uint8_t smoothing_bits>
void test_distribution2()
{
  size_t hist[256] = {};
  const int64_t startNs = get_current_time_ns();
  const int64_t startTicks = __rdtsc();

  for (size_t a = 0; a < 256; a++)
    for (size_t b = 0; b < (1ULL << smoothing_bits); b++)
      hist[(uint8_t)paranormal_distribution::to_paranormal2<smoothing_bits>((uint8_t)a, (uint8_t)b)]++;

  const int64_t endTicks = __rdtsc();
  const int64_t endNs = get_current_time_ns();
  constexpr size_t iterationCount = 256 * (1ULL << smoothing_bits);
  const size_t totalNs = endNs - startNs;
  const size_t totalTicks = endTicks - startTicks;

  printf("Paranormal2 Distribution (%" PRIu8 " Bit Smoothing)\n", (uint8_t)smoothing_bits);
  print_histogram(hist);
  printf("\nThrouhgput (including loop): %3.1f its/ms = %3.1f ns/call, %4.2f ticks/it\n\n", iterationCount / (totalNs * 1e-6), (double)totalNs / iterationCount, (double)totalTicks / iterationCount);
}

void test_tiny_distribution()
{
  size_t hist[256] = {};
  const int64_t startNs = get_current_time_ns();
  const int64_t startTicks = __rdtsc();

  for (size_t a = 0; a < 256; a++)
    hist[(uint8_t)paranormal_distribution::to_tiny_paranormal((uint8_t)a)]++;

  const int64_t endTicks = __rdtsc();
  const int64_t endNs = get_current_time_ns();
  constexpr size_t iterationCount = 256;
  const size_t totalNs = endNs - startNs;
  const size_t totalTicks = endTicks - startTicks;

  printf("Tiny Paranormal Distribution\n");
  print_histogram(hist);
  printf("\nThrouhgput (including loop): %3.1f its/ms = %3.1f ns/call, %4.2f ticks/it\n\n", iterationCount / (totalNs * 1e-6), (double)totalNs / iterationCount, (double)totalTicks / iterationCount);
}

//////////////////////////////////////////////////////////////////////////

int32_t main(void)
{
  test_raw_distribution();
  test_smooth_distribution<2>();
  test_smooth_distribution<3>();
  test_smooth_distribution<4>();
  test_smooth_distribution<5>();
  test_smooth_distribution<6>();
  test_smooth_distribution<7>();

  test_distribution2<2>();
  test_distribution2<3>();
  test_distribution2<4>();
  test_distribution2<5>();
  test_distribution2<6>();
  test_distribution2<7>();

  test_tiny_distribution();

  return 0;
}
