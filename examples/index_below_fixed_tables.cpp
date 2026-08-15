// Copyright 2026 Yuta Urano
// SPDX-License-Identifier: Apache-2.0

// Carrying a fixed protocol bound from an input boundary into multiple consumers.
//
// A decoded field number is checked once and then used by both layout and classification layers.
// The fixed-extent span and `index_below<16>` state the same numeric bound; subscript remains an
// explicit `.value()` operation because the standard container consumes an ordinary size value.
//
// A one-shot opcode lookup below keeps its local bounds check. Its fact is consumed immediately,
// so materializing a carrier would add ceremony without carrying anything across an API boundary.

#include <precept/index_below.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace {

using field_index = precept::index_below<16>;

struct field_description {
  std::string_view name;
  std::size_t width;
};

constexpr std::array<field_description, 16> fields{{
    {"version", 1},
    {"flags", 1},
    {"payload-size", 2},
    {"sequence", 4},
    {"source", 2},
    {"destination", 2},
    {"kind", 1},
    {"priority", 1},
    {"timestamp", 8},
    {"checksum", 4},
    {"extension-0", 1},
    {"extension-1", 1},
    {"extension-2", 1},
    {"extension-3", 1},
    {"extension-4", 1},
    {"extension-5", 1},
}};

std::string_view field_name(std::span<const field_description, 16> layout,
                            field_index index) noexcept {
  return layout[index.value()].name;
}

std::size_t field_offset(std::span<const field_description, 16> layout,
                         field_index index) noexcept {
  std::size_t offset = 0;
  // This ordinary loop already establishes its bound locally. Wrapping its induction variable
  // would not preserve a fact for another layer.
  for (std::size_t current = 0; current < index.value(); ++current) {
    offset += layout[current].width;
  }
  return offset;
}

struct decoded_field {
  std::string_view name;
  std::size_t offset;
};

std::optional<decoded_field> decode_field(std::size_t raw_index) noexcept {
  const auto index = field_index::try_from(raw_index);
  if (!index) {
    return std::nullopt;
  }

  // The same validated fact crosses two call boundaries and neither consumer repeats the check.
  return decoded_field{field_name(fields, *index), field_offset(fields, *index)};
}

constexpr std::array<std::string_view, 8> opcode_names{"nop",      "load", "store",   "add",
                                                       "subtract", "jump", "compare", "halt"};

std::optional<std::string_view> lookup_opcode(std::size_t raw_index) noexcept {
  if (raw_index >= opcode_names.size()) {
    return std::nullopt;
  }

  return opcode_names[raw_index];
}

} // namespace

int main() {
  const auto sequence = decode_field(3);
  if (!sequence || sequence->name != "sequence" || sequence->offset != 4) {
    return 1;
  }

  if (decode_field(16)) {
    return 2;
  }

  if (lookup_opcode(7) != "halt" || lookup_opcode(8)) {
    return 3;
  }

  return 0;
}
