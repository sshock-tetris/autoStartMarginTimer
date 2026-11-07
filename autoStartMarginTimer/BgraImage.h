#pragma once

#include <cstdint>
#include <vector>

// 色の平均値（各要素は 0.0〜255.0）
struct BgrMean {
  double b;
  double g;
  double r;
};

// キャプチャ画像。1 画素は 4 バイトで、メモリ上は B, G, R, A の順に並ぶ
// （DXGI_FORMAT_B8G8R8A8_UNORM と同じ）。行と行の間に余白はない。
// アルファチャンネルの値は計算に使わない。
// 範囲は RECT で指定し、right と bottom の位置は範囲に含めない。
// 画像からはみ出した部分は無視する。
class BgraImage {
 public:
  BgraImage(int width, int height);

  int width() const { return m_width; }
  int height() const { return m_height; }
  // y 行目の先頭の画素
  uint8_t* Row(int y);
  const uint8_t* Row(int y) const;

  // 範囲内の B, G, R それぞれの平均値を求める
  BgrMean Mean(const RECT& rect) const;
  // 画像全体の B, G, R の値の総和を求める
  // （OpenCV の cv::norm(image, cv::NORM_L1) と同じ値）
  uint64_t SumBgr() const;
  // other との B, G, R の値の差を二乗して範囲内で足し、その平方根を求める
  // （OpenCV の cv::norm(cv::abs(a - b), cv::NORM_L2) と同じ値）
  double DiffNormL2(const BgraImage& other, const RECT& rect) const;

 private:
  static constexpr int BYTES_PER_PIXEL = 4;

  // rect を画像の範囲内に切り詰める
  RECT Clip(const RECT& rect) const;

  int m_width;
  int m_height;
  std::vector<uint8_t> m_pixels;
};
