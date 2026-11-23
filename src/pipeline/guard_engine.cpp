// File: /home/kami/vision-guard/src/pipeline/guard_engine.cpp

#include "vision_guard/pipeline/guard_engine.hpp"

namespace vision_guard::pipeline {

GuardStatus GuardEngine::process_shm_vision(
    std::span<const uint8_t> shm_payload,
    const protocols::VisionPolicy& policy) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::PRIVACY_AIRGAP_TRIPPED;
    const GuardStatus st = shm_validator_.validate_frame(shm_payload, policy);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_rtsp_onvif(
    std::span<const uint8_t> payload,
    bool allow_control_mutations) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::PRIVACY_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::RtspOnvifValidator::validate_packet(payload, allow_control_mutations);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_gige_gvcp(
    std::span<const uint8_t> udp_payload,
    bool allow_reg_writes) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::PRIVACY_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::GigeVisionValidator::validate_gvcp(udp_payload, allow_reg_writes);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_dms_gaze(
    std::span<const uint8_t> payload,
    const protocols::DmsPolicy& policy) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::PRIVACY_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::DmsGazeValidator::validate_dms(payload, policy);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

GuardStatus GuardEngine::process_v4l2_frame(std::span<const uint8_t> yuyv_frame) noexcept
{
    if (airgap_.is_tripped()) return GuardStatus::PRIVACY_AIRGAP_TRIPPED;
    const GuardStatus st = protocols::DmsGazeValidator::validate_v4l2_yuyv(yuyv_frame);
    if (st != GuardStatus::PASS) airgap_.trip();
    return st;
}

} // namespace vision_guard::pipeline