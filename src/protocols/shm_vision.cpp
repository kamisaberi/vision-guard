// File: /home/kami/vision-guard/src/protocols/shm_vision.cpp

#include "vision_guard/protocols/shm_vision.hpp"
#include <cmath>
#include <cstring>

namespace vision_guard::protocols {

GuardStatus ShmVisionValidator::validate_frame(
    std::span<const uint8_t> shm_payload,
    const VisionPolicy& policy) noexcept
{
    if (shm_payload.size() < sizeof(ShmVisionHeader)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    ShmVisionHeader hdr{};
    std::memcpy(&hdr, shm_payload.data(), sizeof(ShmVisionHeader));

    if (hdr.magic != MAGIC_VIS1 || hdr.width == 0 || hdr.height == 0 ||
        !std::isfinite(hdr.entropy_score) ||
        !std::isfinite(hdr.saturation_pct) ||
        !std::isfinite(hdr.anomaly_score)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    if (policy.require_ram_only && hdr.ram_only_ephemeral != 1) {
        return GuardStatus::REJECT_VISION_PRIVACY_RETENTION_VIOLATION;
    }

    if (initialized_ && hdr.frame_seq <= last_seq_) {
        return GuardStatus::REJECT_VISION_FRAME_FREEZE;
    }

    if (hdr.saturation_pct > policy.max_saturation_pct) {
        return GuardStatus::REJECT_VISION_OPTICAL_BLINDING;
    }

    if (hdr.entropy_score < policy.min_entropy) {
        return GuardStatus::REJECT_VISION_FRAME_FREEZE;
    }

    if (hdr.anomaly_score >= policy.alert_threshold) {
        return GuardStatus::REJECT_VISION_DEFECT_OR_BREACH;
    }

    last_seq_ = hdr.frame_seq;
    initialized_ = true;
    return GuardStatus::PASS;
}

} // namespace vision_guard::protocols
