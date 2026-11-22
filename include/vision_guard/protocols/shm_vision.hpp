// File: /home/kami/vision-guard/include/vision_guard/protocols/shm_vision.hpp

#pragma once

#include "vision_guard/core/types.hpp"
#include <cstdint>
#include <span>

namespace vision_guard::protocols {

struct ShmVisionHeader {
    uint32_t magic;              // 0x56495331 ("VIS1")
    uint32_t camera_id;
    uint32_t frame_seq;
    uint16_t width;
    uint16_t height;
    uint8_t  pixel_format;       // 1=RGB8, 2=GRAY8, 3=LWIR16
    uint8_t  ram_only_ephemeral; // 1 = instant RAM purge enforced (HIPAA/GDPR)
    uint16_t reserved;
    float    entropy_score;      // 0.0 = frozen/blinding, 1.0..8.0 = healthy scene
    float    saturation_pct;     // > 85% = laser blinding / overexposure
    float    anomaly_score;      // Edge weapon / defect / fall score (0..1)
};

struct VisionPolicy {
    bool require_ram_only{true};
    float min_entropy{0.35f};
    float max_saturation_pct{85.0f};
    float alert_threshold{0.85f};
};

class ShmVisionValidator {
public:
    static constexpr uint32_t MAGIC_VIS1 = 0x56495331U;

    [[nodiscard]] GuardStatus validate_frame(
        std::span<const uint8_t> shm_payload,
        const VisionPolicy& policy = {}) noexcept;

    void reset() noexcept { last_seq_ = 0; initialized_ = false; }

private:
    uint32_t last_seq_{0};
    bool initialized_{false};
};

} // namespace vision_guard::protocols
