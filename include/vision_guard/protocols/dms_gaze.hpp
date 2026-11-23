// File: /home/kami/vision-guard/include/vision_guard/protocols/dms_gaze.hpp

#pragma once

#include "vision_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace vision_guard::protocols {

struct DmsGazeFrame {
    uint32_t magic;            // 0x444D5331 ("DMS1")
    uint32_t frame_seq;
    float perclos_pct;         // Percentage of eyelid closure over pupil (0..100%)
    float ear_left;            // Eye Aspect Ratio left (nominal ~0.28)
    float ear_right;           // Eye Aspect Ratio right (nominal ~0.28)
    float head_yaw_deg;        // Left/right head rotation
    float head_pitch_deg;      // Up/down head nod
    float gaze_off_road_sec;   // Continuous eyes-off-road duration
    float power_watts;         // Edge SoC power envelope (target < 5W)
};

struct DmsPolicy {
    float max_perclos_pct{20.0f};     // EU GSR / NHTSA drowsiness threshold
    float min_ear{0.18f};
    float max_yaw_deg{30.0f};
    float max_pitch_deg{25.0f};
    float max_gaze_off_road_sec{2.0f};
};

class DmsGazeValidator {
public:
    static constexpr uint32_t MAGIC_DMS1 = 0x444D5331U;

    [[nodiscard]] static GuardStatus validate_dms(
        std::span<const uint8_t> payload,
        const DmsPolicy& policy = {}) noexcept;

    /// Constant-time O(64) strided luma check on raw Bus D V4L2/MIPI-CSI2 YUYV frames
    [[nodiscard]] static GuardStatus validate_v4l2_yuyv(
        std::span<const uint8_t> yuyv_frame) noexcept;
};

} // namespace vision_guard::protocols