// File: /home/kami/vision-guard/tools/vision_guard_ctl/main.cpp

#include "vision_guard/vision_guard.hpp"
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    vision_guard::core::TscClock::calibrate();
    vision_guard::pipeline::GuardEngine engine;

    vision_guard::protocols::ShmVisionHeader hdr{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 1, 0, 4.5f, 12.0f, 0.05f};
    std::vector<uint8_t> buf(sizeof(hdr));
    std::memcpy(buf.data(), &hdr, sizeof(hdr));
    const auto st = engine.process_shm_vision(buf);

    std::cout << "[vision-guard-ctl] TSC Clock : " << vision_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "[vision-guard-ctl] Self-Test : " << vision_guard::status_to_string(st) << "\n";
    return (st == vision_guard::GuardStatus::PASS) ? 0 : 1;
}
