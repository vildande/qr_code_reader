#include <qrcode/bit_stream.h>

#include <limits>
#include <stdexcept>

namespace qr {

BitStream::BitStream(std::span<const std::uint8_t> bits) noexcept
    : bits_{bits} {}

std::uint32_t BitStream::read_bits(std::size_t count) {
    constexpr auto max_bits = std::numeric_limits<std::uint32_t>::digits;

    if (count > max_bits) {
        throw std::invalid_argument{"BitStream cannot read more than 32 bits at once"};
    }
    if (count > bits_.size() - position_) {
        throw std::out_of_range{"not enough bits in BitStream"};
    }

    std::uint32_t value{};
    for (std::size_t i = 0; i < count; ++i) {
        value = (value << 1U) | static_cast<std::uint32_t>(bits_[position_] != 0);
        ++position_;
    }

    return value;
}

std::size_t BitStream::remaining() const noexcept {
    return bits_.size() - position_;
}

} // namespace qr
