// File: /home/kami/vision-guard/tests/test_dms_gaze.cpp

#include "vision_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[TEST] Automotive DMS Head-Pose/PERCLOS & V4L2 MIPI-CSI2 YUYV... ";
    vision_guard::pipeline::GuardEngine engine;

    vision_guard::protocols::DmsGazeFrame ok{
        vision_guard::protocols::DmsGazeValidator::MAGIC_DMS1, 1, 8.0f, 0.29f, 0.28f, 5.0f, -2.0f, 0.2f, 3.8f};
    std::vector<uint8_t> buf(sizeof(ok));
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_dms_gaze(buf) == vision_guard::GuardStatus::PASS);

    // Drowsiness (PERCLOS 34% > 20%)
    ok.perclos_pct = 34.0f;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_dms_gaze(buf) == vision_guard::GuardStatus::REJECT_DMS_DRIVER_DROWSINESS);

    // Distraction (eyes off road 3.1s > 2.0s)
    engine.reset_airgap();
    ok.perclos_pct = 6.0f; ok.gaze_off_road_sec = 3.1f;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    assert(engine.process_dms_gaze(buf) == vision_guard::GuardStatus::REJECT_DMS_GAZE_DISTRACTION);

    // Bus D Raw V4L2 YUYV nominal vs laser blinded (Y=255)
    engine.reset_airgap();
    assert(engine.process_v4l2_frame(std::vector<uint8_t>(1024, 120)) == vision_guard::GuardStatus::PASS);
    assert(engine.process_v4l2_frame(std::vector<uint8_t>(1024, 255)) == vision_guard::GuardStatus::REJECT_VISION_OPTICAL_BLINDING);

    std::cout << "PASSED\n";
    return 0;
}