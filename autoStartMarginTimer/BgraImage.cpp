#include "stdafx.h"

#include "BgraImage.h"

#include <algorithm>
#include <cmath>

BgraImage::BgraImage(int width, int height)
    : m_width(std::max(width, 0)),
      m_height(std::max(height, 0)),
      m_pixels(static_cast<size_t>(m_width) * m_height * BYTES_PER_PIXEL) {}

uint8_t* BgraImage::Row(int y) {
  return m_pixels.data() + static_cast<size_t>(y) * m_width * BYTES_PER_PIXEL;
}

const uint8_t* BgraImage::Row(int y) const {
  return m_pixels.data() + static_cast<size_t>(y) * m_width * BYTES_PER_PIXEL;
}

RECT BgraImage::Clip(const RECT& rect) const {
  RECT clipped;
  clipped.left = std::clamp<LONG>(rect.left, 0, m_width);
  clipped.top = std::clamp<LONG>(rect.top, 0, m_height);
  clipped.right = std::clamp<LONG>(rect.right, clipped.left, m_width);
  clipped.bottom = std::clamp<LONG>(rect.bottom, clipped.top, m_height);
  return clipped;
}

BgrMean BgraImage::Mean(const RECT& rect) const {
  const RECT r = Clip(rect);
  uint64_t sum_b = 0;
  uint64_t sum_g = 0;
  uint64_t sum_r = 0;
  for (LONG y = r.top; y < r.bottom; y++) {
    const uint8_t* pixel =
        Row(y) + static_cast<size_t>(r.left) * BYTES_PER_PIXEL;
    for (LONG x = r.left; x < r.right; x++) {
      sum_b += pixel[0];
      sum_g += pixel[1];
      sum_r += pixel[2];
      pixel += BYTES_PER_PIXEL;
    }
  }
  const uint64_t count = static_cast<uint64_t>(r.right - r.left) *
                         static_cast<uint64_t>(r.bottom - r.top);
  if (count == 0) {
    return {0.0, 0.0, 0.0};
  }
  const double n = static_cast<double>(count);
  return {static_cast<double>(sum_b) / n, static_cast<double>(sum_g) / n,
          static_cast<double>(sum_r) / n};
}

uint64_t BgraImage::SumBgr() const {
  uint64_t sum = 0;
  for (size_t i = 0; i < m_pixels.size(); i += BYTES_PER_PIXEL) {
    sum += static_cast<uint64_t>(m_pixels[i]) + m_pixels[i + 1] +
           m_pixels[i + 2];
  }
  return sum;
}

double BgraImage::DiffNormL2(const BgraImage& other, const RECT& rect) const {
  // 大きさの違う画像と比べる場合は、両方に収まる範囲だけを使う
  const RECT r = other.Clip(Clip(rect));
  uint64_t sum = 0;
  for (LONG y = r.top; y < r.bottom; y++) {
    const uint8_t* p1 = Row(y) + static_cast<size_t>(r.left) * BYTES_PER_PIXEL;
    const uint8_t* p2 =
        other.Row(y) + static_cast<size_t>(r.left) * BYTES_PER_PIXEL;
    for (LONG x = r.left; x < r.right; x++) {
      for (int c = 0; c < 3; c++) {
        const int diff = p1[c] - p2[c];
        sum += static_cast<uint64_t>(diff * diff);
      }
      p1 += BYTES_PER_PIXEL;
      p2 += BYTES_PER_PIXEL;
    }
  }
  return std::sqrt(static_cast<double>(sum));
}
