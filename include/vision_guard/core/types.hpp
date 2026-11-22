// File: /home/kami/vision-guard/include/vision_guard/core/types.hpp

#pragma once

#include <cstdint>
#include <string_view>

namespace vision_guard {

enum class GuardStatus : uint8_t {
    PASS = 0,
    REJECT_MALFORMED_FRAME,
    REJECT_VISION_FRAME_FREEZE,
    REJECT_VISION_OPTICAL_BLINDING,
    REJECT_VISION_PRIVACY_RETENTION_VIOLATION,
    REJECT_VISION_DEFECT_OR_BREACH,
    REJECT_RTSP_STREAM_HIJACK,
    REJECT_GIGE_UNAUTHORIZED_WRITE,
    PRIVACY_AIRGAP_TRIPPED
};

[[nodiscard]] constexpr std::string_view status_to_string(GuardStatus st) noexcept {
    switch (st) {
        case GuardStatus::PASS:                                      return "PASS";
        case GuardStatus::REJECT_MALFORMED_FRAME:                    return "REJECT_MALFORMED_FRAME";
        case GuardStatus::REJECT_VISION_FRAME_FREEZE:                return "REJECT_VISION_FRAME_FREEZE";
        case GuardStatus::REJECT_VISION_OPTICAL_BLINDING:            return "REJECT_VISION_OPTICAL_BLINDING";
        case GuardStatus::REJECT_VISION_PRIVACY_RETENTION_VIOLATION: return "REJECT_VISION_PRIVACY_RETENTION_VIOLATION";
        case GuardStatus::REJECT_VISION_DEFECT_OR_BREACH:            return "REJECT_VISION_DEFECT_OR_BREACH";
        case GuardStatus::REJECT_RTSP_STREAM_HIJACK:                 return "REJECT_RTSP_STREAM_HIJACK";
        case GuardStatus::REJECT_GIGE_UNAUTHORIZED_WRITE:            return "REJECT_GIGE_UNAUTHORIZED_WRITE";
        case GuardStatus::PRIVACY_AIRGAP_TRIPPED:                    return "PRIVACY_AIRGAP_TRIPPED";
    }
    return "UNKNOWN";
}

} // namespace vision_guard
