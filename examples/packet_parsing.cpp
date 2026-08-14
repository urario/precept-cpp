// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Packet and header parsing.
//
// A wire format usually says "the first 16 bytes are the header". Written as
// `std::span<const std::byte>`, that condition is invisible in the signature, and every function
// down the call chain has to re-check it. `at_least_span<const std::byte, 16>` states it once, so
// `parse_packet()` below splits header from payload without a size check of its own.

#include <precept/span/at_least_span.hpp>
#include <precept/span/checked_span.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

namespace {

constexpr std::size_t header_size = 16;

// The header is exactly 16 bytes, so it travels as a fixed-extent standard span. No Precept type is
// needed here: `std::span<const std::byte, 16>` already carries the whole fact.
std::uint16_t read_message_type(std::span<const std::byte, header_size> header) {
  return static_cast<std::uint16_t>(static_cast<unsigned>(header[0]) << 8 |
                                    static_cast<unsigned>(header[1]));
}

// The minimum size is part of the parameter type, so this function neither re-checks it nor
// documents it in a comment that could go stale.
void parse_packet(precept::at_least_span<const std::byte, header_size> packet) {
  const std::span<const std::byte, header_size> header = packet.prefix();
  const std::span<const std::byte> payload = packet.rest();

  std::cout << "message type: " << read_message_type(header) << '\n'
            << "payload bytes: " << payload.size() << '\n';
}

std::vector<std::byte> receive(std::size_t size) {
  std::vector<std::byte> bytes(size, std::byte{0});
  if (size >= 2) {
    bytes[0] = std::byte{0x00};
    bytes[1] = std::byte{0x2a};
  }
  return bytes;
}

} // namespace

int main() {
  // Validation happens once, at the boundary where the bytes arrive.
  const std::vector<std::byte> datagram = receive(24);
  if (auto packet = precept::at_least_span<const std::byte, header_size>::try_from(
          std::span<const std::byte>{datagram})) {
    parse_packet(*packet);
  }

  // A short datagram is rejected without throwing, and without a truncated view being handed on.
  const std::vector<std::byte> truncated = receive(8);
  if (!precept::at_least_span<const std::byte, header_size>::try_from(
          std::span<const std::byte>{truncated})) {
    std::cout << "rejected a " << truncated.size() << "-byte datagram\n";
  }

  // When a buffer proves the size at compile time, no runtime validation is involved at all.
  const std::array<std::byte, header_size> fixed{};
  parse_packet(std::span<const std::byte, header_size>{fixed});

  // `checked_span` is the exact-size counterpart. It accepts only a 16-byte view and returns the
  // standard fixed-extent span, so a header that must not carry a payload can say so.
  const std::vector<std::byte> header_only = receive(header_size);
  if (auto header = precept::checked_span<header_size>(std::span<const std::byte>{header_only})) {
    std::cout << "standalone header type: " << read_message_type(*header) << '\n';
  }

  return 0;
}
