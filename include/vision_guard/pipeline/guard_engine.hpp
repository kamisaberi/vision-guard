// File: /home/kami/vision-guard/include/vision_guard/pipeline/guard_engine.hpp

#pragma once

#include "vision_guard/core/types.hpp"
#include "vision_guard/protocols/shm_vision.hpp"
#include "vision_guard/protocols/rtsp_onvif.hpp"
#include "vision_guard/protocols/gige_vision.hpp"
#include "vision_guard/defense/privacy_airgap.hpp"
#include <span>

namespace vision_guard::pipeline {

class GuardEngine {
public:
    [[nodiscard]] GuardStatus process_shm_vision(
        std::span<const uint8_t> shm_payload,
        const protocols::VisionPolicy& policy = {}) noexcept;

    [[nodiscard]] GuardStatus process_rtsp_onvif(
        std::span<const uint8_t> payload,
        bool allow_control_mutations = false) noexcept;

    [[nodiscard]] GuardStatus process_gige_gvcp(
        std::span<const uint8_t> udp_payload,
        bool allow_reg_writes = false) noexcept;

    [[nodiscard]] bool is_airgap_tripped() const noexcept { return airgap_.is_tripped(); }
    void reset_airgap() noexcept {
        airgap_.reset();
        shm_validator_.reset();
    }

private:
    protocols::ShmVisionValidator shm_validator_;
    defense::PrivacyAirGap airgap_;
};

} // namespace vision_guard::pipeline
