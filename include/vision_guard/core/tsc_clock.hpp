// File: /home/kami/vision-guard/include/vision_guard/core/tsc_clock.hpp

#pragma once

#include <cstdint>

namespace vision_guard::core {

class TscClock {
public:
    static void calibrate() noexcept;

    [[nodiscard]] static inline uint64_t rdtsc() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
        uint32_t lo = 0, hi = 0;
        __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
        return (static_cast<uint64_t>(hi) << 32) | lo;
#elif defined(__aarch64__)
        uint64_t val = 0;
        __asm__ volatile("mrs %0, cntvct_el0" : "=r"(val));
        return val;
#else
        return 0;
#endif
    }

    [[nodiscard]] static double cycles_to_nanoseconds(uint64_t cycles) noexcept;
    [[nodiscard]] static double ghz() noexcept;
};

} // namespace vision_guard::core
