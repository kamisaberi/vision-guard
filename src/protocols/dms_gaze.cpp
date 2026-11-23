// File: /home/kami/vision-guard/src/protocols/dms_gaze.cpp

#include "vision_guard/protocols/dms_gaze.hpp"
#include <cmath>
#include <cstring>

namespace vision_guard::protocols {

GuardStatus DmsGazeValidator::validate_dms(
    std::span<const uint8_t> payload,
    const DmsPolicy& policy) noexcept
{
    if (payload.size() < sizeof(DmsGazeFrame)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    DmsGazeFrame f{};
    std::memcpy(&f, payload.data(), sizeof(DmsGazeFrame));

    if (f.magic != MAGIC_DMS1 ||
        !std::isfinite(f.perclos_pct) || !std::isfinite(f.ear_left) || !std::isfinite(f.ear_right) ||
        !std::isfinite(f.head_yaw_deg) || !std::isfinite(f.head_pitch_deg) ||
        !std::isfinite(f.gaze_off_road_sec)) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    if (f.perclos_pct > policy.max_perclos_pct ||
        (f.ear_left < policy.min_ear && f.ear_right < policy.min_ear)) {
        return GuardStatus::REJECT_DMS_DRIVER_DROWSINESS;
    }

    if (std::abs(f.head_yaw_deg) > policy.max_yaw_deg ||
        std::abs(f.head_pitch_deg) > policy.max_pitch_deg ||
        f.gaze_off_road_sec > policy.max_gaze_off_road_sec) {
        return GuardStatus::REJECT_DMS_GAZE_DISTRACTION;
    }

    return GuardStatus::PASS;
}

GuardStatus DmsGazeValidator::validate_v4l2_yuyv(std::span<const uint8_t> yuyv_frame) noexcept {
    if (yuyv_frame.size() < 128) {
        return GuardStatus::REJECT_MALFORMED_FRAME;
    }

    // Sample 64 strided Y (luma) bytes from YUYV (even offsets) in < 50 ns
    const size_t stride = (yuyv_frame.size() / 64U) & ~size_t{1};
    const size_t step = (stride >= 2U) ? stride : 2U;
    uint32_t sum_y = 0;
    size_t samples = 0;

    for (size_t off = 0; off < yuyv_frame.size() && samples < 64; off += step) {
        sum_y += yuyv_frame[off];
        samples++;
    }

    const uint32_t mean_y = (samples > 0) ? (sum_y / static_cast<uint32_t>(samples)) : 0;
    if (mean_y > 248U) {
        return GuardStatus::REJECT_VISION_OPTICAL_BLINDING;
    }
    if (mean_y < 8U) {
        return GuardStatus::REJECT_VISION_FRAME_FREEZE; // Covered / occluded NIR lens
    }

    return GuardStatus::PASS;
}

} // namespace vision_guard::protocols