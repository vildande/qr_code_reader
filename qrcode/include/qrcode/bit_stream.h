#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace qr {

/// Reads unsigned values from a non-owning, MSB-first bit sequence.
class BitStream {
public:
    /// Creates a reader over bits that must outlive the stream.
    explicit BitStream(std::span<const std::uint8_t> bits) noexcept;

    /// Consumes and returns up to 32 bits.
    /// @throws std::invalid_argument if 'count' exceeds 32.
    /// @throws std::out_of_range if too few bits remain.
    [[nodiscard]] std::uint32_t read_bits(std::size_t count);

    /// Returns the number of unread bits.
    [[nodiscard]] std::size_t remaining() const noexcept;

private:
    std::span<const std::uint8_t> bits_;
    std::size_t position_{};
};

} // namespace qr
