// File: /home/kami/vision-guard/include/vision_guard/protocols/rtsp_onvif.hpp

#pragma once

#include "vision_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace vision_guard::protocols {

class RtspOnvifValidator {
public:
    [[nodiscard]] static GuardStatus validate_packet(
        std::span<const uint8_t> payload,
        bool allow_control_mutations = false) noexcept;
};

} // namespace vision_guard::protocols
