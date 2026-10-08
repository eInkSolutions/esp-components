#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Epd {

/// Non-owning view of an e-paper framebuffer.
///
/// Framebuffer does not allocate, copy, or own the underlying pixel data.
/// It provides information about the buffer dimensions and helpers for
/// translating pixel coordinates into byte offsets.
///
/// -------------------------------------------------------------------------
/// Pixel storage
/// -------------------------------------------------------------------------
///
/// Each pixel is represented by 4 bits (one nibble). Two pixels are packed
/// into each byte:
///
///     Byte:
///
///     ┌───────────────┬───────────────┐
///     │   Pixel 1     │    Pixel 2    │
///     │   bits 7..4   │    bits 3..0  │
///     └───────────────┴───────────────┘
///
/// In other words:
///
///     byte = (pixel1 << 4) | pixel2
///
/// For example, four pixels:
///
///     Pixel:     1    2    3    4
///     Data:      0x2  0x5  0x3  0x6
///
/// are stored as:
///
///     Byte 0 = 0x25
///     Byte 1 = 0x36
///
/// Binary representation:
///
///     Pixel:       1         2         3         4
///     Bits:        7 6 5 4   3 2 1 0   7 6 5 4   3 2 1 0
///     Data:        0 0 1 0   0 1 0 1   0 0 1 1   0 1 1 0
///                  └─0x2──┘  └─0x5──┘  └─0x3──┘  └─0x6──┘
///
/// Therefore:
///
///     Pixel 1 = high nibble of byte 0
///     Pixel 2 = low  nibble of byte 0
///     Pixel 3 = high nibble of byte 1
///     Pixel 4 = low  nibble of byte 1
///
/// -------------------------------------------------------------------------
/// Row storage
/// -------------------------------------------------------------------------
///
/// Since two pixels are stored in each byte, the number of bytes required
/// for one row is:
///
///     bytesPerRow = (width + 1) / 2
///
/// For example, an 800-pixel-wide display requires:
///
///     (800 + 1) / 2 = 400 bytes per row
///
/// The framebuffer is stored in row-major order:
///
///     Row 0:  [byte 0] [byte 1] [byte 2] ...
///     Row 1:  [byte 0] [byte 1] [byte 2] ...
///     Row 2:  [byte 0] [byte 1] [byte 2] ...
///
/// The byte index is:
///
///     index(x, y) = y * bytesPerRow() + x
///
/// Here, x represents a BYTE coordinate, not a pixel coordinate.
///
/// Use byteX() to convert a pixel X coordinate to its corresponding byte:
///
///     byteX(pixelX) = pixelX / 2
///
/// For example:
///
///     pixel X = 0 -> byte X = 0
///     pixel X = 1 -> byte X = 0
///     pixel X = 2 -> byte X = 1
///     pixel X = 3 -> byte X = 1
///
/// -------------------------------------------------------------------------
/// Memory ownership
/// -------------------------------------------------------------------------
///
/// Framebuffer is a non-owning view. It does not allocate or free the
/// memory passed to it. The caller is responsible for keeping the buffer
/// alive for as long as the Framebuffer object is used.
///
/// Example:
///
///     uint8_t buffer[400 * 480];
///     Epd::Framebuffer framebuffer(buffer, 800, 480);
///
///     framebuffer.byte(10, 20) = 0x25;
///
/// The Framebuffer object itself only stores a pointer and the two
/// dimensions; it does not contain a copy of the framebuffer.
class Framebuffer {
public:
  constexpr Framebuffer(uint8_t *data, uint16_t width, uint16_t height)
      : data_(data), width_(width), height_(height) {}

  constexpr uint16_t width() const { return width_; }

  constexpr uint16_t height() const { return height_; }

  constexpr uint16_t bytesPerRow() const { return (width_ + 1) / 2; }

  constexpr size_t size() const {
    return static_cast<size_t>(bytesPerRow()) * height_;
  }

  constexpr uint16_t byteX(uint16_t pixelX) const { return pixelX / 2; }

  constexpr size_t index(uint16_t pixelX, uint16_t y) const {
    return static_cast<size_t>(y) * bytesPerRow() + byteX(pixelX);
  }

  uint8_t &byte(uint16_t pixelX, uint16_t y) { return data_[index(pixelX, y)]; }

  const uint8_t &byte(uint16_t pixelX, uint16_t y) const {
    return data_[index(pixelX, y)];
  }

  uint8_t getPixel(uint16_t x, uint16_t y) const {
    const uint8_t value = byte(x, y);

    if ((x & 1) == 0) {
      return value >> 4;
    }

    return value & 0x0F;
  }

  void setPixel(uint16_t x, uint16_t y, uint8_t color) {
    uint8_t &value = byte(x, y);

    color &= 0x0F;

    if ((x & 1) == 0) {
      value = (value & 0x0F) | (color << 4);
    } else {
      value = (value & 0xF0) | color;
    }
  }

  uint8_t *data() { return data_; }

  const uint8_t *data() const { return data_; }

private:
  uint8_t *data_;
  uint16_t width_;
  uint16_t height_;
};

} // namespace Epd
