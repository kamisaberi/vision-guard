// File: /home/kami/vision-guard/tests/test_rtsp_gige.cpp

#include "vision_guard/pipeline/guard_engine.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::cout << "[TEST] RTSP/ONVIF Stream Hijack & GigE Vision GVCP Write-Lock... ";
    vision_guard::pipeline::GuardEngine engine;

    const std::string rtsp_play = "PLAY rtsp://10.240.80.75:8554/feed RTSP/1.0\r\n";
    assert(engine.process_rtsp_onvif(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(rtsp_play.data()), rtsp_play.size()))
           == vision_guard::GuardStatus::PASS);

    const std::string rtsp_kill = "TEARDOWN rtsp://10.240.80.75:8554/feed RTSP/1.0\r\n";
    assert(engine.process_rtsp_onvif(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(rtsp_kill.data()), rtsp_kill.size()))
           == vision_guard::GuardStatus::REJECT_RTSP_STREAM_HIJACK);

    engine.reset_airgap();
    const std::vector<uint8_t> gvcp_read = {0x42, 0x01, 0x00, 0x80, 0x00, 0x04, 0x00, 0x01};
    assert(engine.process_gige_gvcp(gvcp_read) == vision_guard::GuardStatus::PASS);

    const std::vector<uint8_t> gvcp_write = {0x42, 0x01, 0x00, 0x82, 0x00, 0x08, 0x00, 0x01};
    assert(engine.process_gige_gvcp(gvcp_write) == vision_guard::GuardStatus::REJECT_GIGE_UNAUTHORIZED_WRITE);

    std::cout << "PASSED\n";
    return 0;
}
