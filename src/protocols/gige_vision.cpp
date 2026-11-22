// File: /home/kami/vision-guard/src/protocols/gige_vision.cpp

#include "vision_guard/protocols/gige_vision.hpp"

namespace vision_guard::protocols {

GuardStatus GigeVisionValidator::validate_gvcp(
    std::span<const uint8_t> udp_payload,
    bool allow_reg_writes) noexcept
{
    if (udp_payload.size() < MIN_GVCP_LEN || udp_payload[0] != 0x42) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    const uint16_t cmd = (static_cast<uint16_t>(udp_payload[2]) << 8) | udp_payload[3];
    // GVCP WRITEREG_CMD = 0x0082, WRITEMEM_CMD = 0x0086
    if (!allow_reg_writes && (cmd == 0x0082 || cmd == 0x0086)) {
        return GuardStatus::REJECT_GIGE_UNAUTHORIZED_WRITE;
    }

    return GuardStatus::PASS;
}

} // namespace vision_guard::protocols
