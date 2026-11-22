// File: /home/kami/vision-guard/benchmarks/bench_latency.cpp

#include "vision_guard/vision_guard.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <numeric>
#include <vector>

int main()
{
    vision_guard::core::TscClock::calibrate();
    vision_guard::pipeline::GuardEngine engine;

    constexpr size_t ITERATIONS = 100'000;
    std::vector<double> samples_ns;
    samples_ns.reserve(ITERATIONS);

    vision_guard::protocols::ShmVisionHeader hdr{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 1, 0, 4.5f, 12.0f, 0.05f};
    std::vector<uint8_t> buf(sizeof(hdr));

    for (size_t i = 1; i <= ITERATIONS; ++i)
    {
        hdr.frame_seq = static_cast<uint32_t>(i);
        std::memcpy(buf.data(), &hdr, sizeof(hdr));
        const uint64_t t0 = vision_guard::core::TscClock::rdtsc();
        const auto st = engine.process_shm_vision(buf);
        const uint64_t t1 = vision_guard::core::TscClock::rdtsc();
        if (st == vision_guard::GuardStatus::PASS)
            samples_ns.push_back(vision_guard::core::TscClock::cycles_to_nanoseconds(t1 - t0));
    }

    std::sort(samples_ns.begin(), samples_ns.end());
    const double sum = std::accumulate(samples_ns.begin(), samples_ns.end(), 0.0);
    const double mean = sum / static_cast<double>(samples_ns.size());
    const double p99 = samples_ns[(samples_ns.size() * 99) / 100];

    std::cout << "================================================================================\n";
    std::cout << "  VISION-GUARD HARDWARE LATENCY BENCHMARK (100,000 SHM-VISION FRAMES)\n";
    std::cout << "================================================================================\n";
    std::cout << "  TSC Frequency : " << vision_guard::core::TscClock::ghz() << " GHz\n";
    std::cout << "  Mean Latency  : " << mean << " ns\n";
    std::cout << "  P99 Latency   : " << p99 << " ns\n";
    std::cout << "  SLA Target    : < 120.0 ns -> " << (mean < 120.0 ? "PASS" : "FAIL") << "\n";
    std::cout << "================================================================================\n";
    return (mean < 120.0) ? 0 : 1;
}
