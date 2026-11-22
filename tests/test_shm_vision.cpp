// File: /home/kami/vision-guard/tests/test_shm_vision.cpp

#include "vision_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] ShmVision HIPAA RAM-Only, Freeze & Blinding Validation... ";
    vision_guard::pipeline::GuardEngine engine;

    vision_guard::protocols::ShmVisionHeader ok{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 10, 640, 480, 1, 1, 0, 4.5f, 15.0f, 0.08f};
    std::vector<uint8_t> buf(sizeof(ok));
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::PASS);

    // Freeze / replay frame_seq=10
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::REJECT_VISION_FRAME_FREEZE);

    // Disk retention violation (ram_only_ephemeral=0)
    engine.reset_airgap();
    ok.frame_seq = 11; ok.ram_only_ephemeral = 0;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::REJECT_VISION_PRIVACY_RETENTION_VIOLATION);

    // Optical blinding (> 85% saturation)
    engine.reset_airgap();
    ok.frame_seq = 12; ok.ram_only_ephemeral = 1; ok.saturation_pct = 96.0f;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::REJECT_VISION_OPTICAL_BLINDING);

    // Weapon/Breach/Defect detection (anomaly_score >= 0.85)
    engine.reset_airgap();
    ok.frame_seq = 13; ok.saturation_pct = 15.0f; ok.anomaly_score = 0.94f;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_shm_vision(buf) == vision_guard::GuardStatus::REJECT_VISION_DEFECT_OR_BREACH);

    std::cout << "PASSED\n";
    return 0;
}
