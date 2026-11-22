// File: /home/kami/vision-guard/src/protocols/rtsp_onvif.cpp

#include "vision_guard/protocols/rtsp_onvif.hpp"
#include <string_view>

namespace vision_guard::protocols {

GuardStatus RtspOnvifValidator::validate_packet(
    std::span<const uint8_t> payload,
    bool allow_control_mutations) noexcept
{
    if (payload.size() < 8) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    const std::string_view sv(reinterpret_cast<const char*>(payload.data()), payload.size());
    if (!allow_control_mutations) {
        if (sv.find("TEARDOWN ") != std::string_view::npos ||
            sv.find("SET_PARAMETER ") != std::string_view::npos ||
            sv.find("RECORD ") != std::string_view::npos ||
            sv.find("SystemReboot") != std::string_view::npos ||
            sv.find("SetSystemFactoryDefault") != std::string_view::npos) {
            return GuardStatus::REJECT_RTSP_STREAM_HIJACK;
        }
    }

    return GuardStatus::PASS;
}

} // namespace vision_guard::protocols
