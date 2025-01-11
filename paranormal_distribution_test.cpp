#include "paranormal_distribution.h"

#include <stdio.h>
#include <inttypes.h>

#include <algorithm>
#include <bit>

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

void set_console_color(const console_color fg, const console_color bg = console_color::Black)
{
  const size_t fgColour = ((size_t)fg & 0xF);
  const size_t bgColour = ((size_t)bg & 0xF);

  printf("\x1b[%" PRIu64 ";%" PRIu64 "m", fgColour < 0x8 ? (30 + fgColour) : (90 - 8 + fgColour), bgColour < 0x8 ? (40 + bgColour) : (100 - 8 + bgColour));
}

void reset_console_color()
{
  fputs("\x1b[0m", stdout);
}

void print_histogram(const size_t(&hist)[256])
{
  size_t maxHeight = 1;

  for (size_t i = 0; i < 256; i++)
    maxHeight = std::max(maxHeight, hist[i]);

  constexpr size_t barHeight = 32;
  constexpr size_t displayFactor = 4;
  const size_t heightDiv = maxHeight / barHeight;
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

void test_raw_distribution()
{
  size_t hist[256] = {};

  for (size_t a = 0; a < 256; a++)
    for (size_t b = 0; b < 256; b++)
      hist[(uint8_t)paranormal_distribution::to_paranormal(a, b)]++;

  puts("Raw Distribution (No Smoothing)");
  print_histogram(hist);
  puts("");
}

template <uint8_t smoothing_bits>
void test_smooth_distribution()
{
  size_t hist[256] = {};

  for (size_t a = 0; a < 256; a++)
    for (size_t b = 0; b < 256; b++)
      for (size_t c = 0; c < (1ULL << smoothing_bits); c++)
        hist[(uint8_t)paranormal_distribution::to_smooth_paranormal<smoothing_bits>(a, b, c)]++;

  printf("Smooth Distribution (%" PRIu8 " Bit Smoothing)\n", (uint8_t)smoothing_bits);
  print_histogram(hist);

}

int32_t main(void)
{
  test_raw_distribution();
  test_smooth_distribution<1>();
  test_smooth_distribution<2>();
  test_smooth_distribution<3>();
  test_smooth_distribution<4>();
  test_smooth_distribution<5>();
  test_smooth_distribution<6>();
  test_smooth_distribution<7>();

  return 0;
}
