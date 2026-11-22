// File: /home/kami/vision-guard/include/vision_guard/protocols/gige_vision.hpp

#pragma once

#include "vision_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace vision_guard::protocols {

class GigeVisionValidator {
public:
    static constexpr size_t MIN_GVCP_LEN = 8;

    [[nodiscard]] static GuardStatus validate_gvcp(
        std::span<const uint8_t> udp_payload,
        bool allow_reg_writes = false) noexcept;
};

} // namespace vision_guard::protocols
