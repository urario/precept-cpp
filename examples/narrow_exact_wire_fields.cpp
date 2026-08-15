// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Narrowing in-memory values into the field widths a wire format defines.
//
// The record header stores a payload length in 16 bits and a sequence number in 32 bits, while the
// program works with `std::size_t` and a signed counter. Each conversion is the point where a value
// either fits the format or the frame cannot be built, and `narrow_exact` makes that the shape of
// the code: there is no cast that could silently wrap a length or turn a negative sequence number
// into a very large one.
//
// The result of a successful conversion is an ordinary `std::uint16_t` or `std::uint32_t`. Nothing
// downstream has to know it was validated, because the value itself is what was verified.

#include <precept/narrow_exact.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace {

struct record_header {
  std::uint16_t payload_length;
  std::uint32_t sequence_number;
};

// The only place where the in-memory types stop being the program's own types.
std::optional<record_header> make_header(std::span<const std::byte> payload,
                                         std::int64_t sequence_number) noexcept {
  const auto length = precept::narrow_exact<std::uint16_t>(payload.size());
  if (!length) {
    return std::nullopt;
  }

  const auto sequence = precept::narrow_exact<std::uint32_t>(sequence_number);
  if (!sequence) {
    return std::nullopt;
  }

  return record_header{*length, *sequence};
}

// Downstream code takes the standard types. It cannot be reached with a value that does not fit,
// and it does not repeat the check.
std::array<std::byte, 6> encode(record_header header) noexcept {
  return {std::byte{static_cast<unsigned char>(header.payload_length >> 8U)},
          std::byte{static_cast<unsigned char>(header.payload_length & 0xFFU)},
          std::byte{static_cast<unsigned char>(header.sequence_number >> 24U)},
          std::byte{static_cast<unsigned char>((header.sequence_number >> 16U) & 0xFFU)},
          std::byte{static_cast<unsigned char>((header.sequence_number >> 8U) & 0xFFU)},
          std::byte{static_cast<unsigned char>(header.sequence_number & 0xFFU)}};
}

} // namespace

int main() {
  const std::array<std::byte, 258> payload{};

  const auto header = make_header(payload, 70000);
  if (!header) {
    return 1;
  }

  const std::array<std::byte, 6> encoded = encode(*header);
  const std::array<std::byte, 6> expected{std::byte{0x01}, std::byte{0x02}, std::byte{0x00},
                                          std::byte{0x01}, std::byte{0x11}, std::byte{0x70}};
  if (encoded != expected) {
    return 2;
  }

  // A negative sequence number is refused rather than reinterpreted as a large unsigned one.
  if (make_header(payload, -1)) {
    return 3;
  }

  // A payload longer than the 16-bit length field is refused rather than truncated.
  static const std::array<std::byte, 70000> oversized{};
  if (make_header(oversized, 1)) {
    return 4;
  }

  return 0;
}
