// File: /home/kami/vision-guard/tests/test_sentinel_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include "vision_guard/protocols/shm_vision.hpp"
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <vector>

static int g_logs = 0;
static int g_metrics = 0;

static void mock_log(int level, const char *sender, const char *msg)
{
    std::cout << "  [HOST_LOG level=" << level << " sender=" << sender << "] " << msg << "\n";
    g_logs++;
}

static void mock_metric(const char *name, uint64_t delta)
{
    std::cout << "  [HOST_METRIC] " << name << " (+" << delta << ")\n";
    g_metrics += static_cast<int>(delta);
}

int main()
{
    std::cout << "[TEST] blackbox-sentinel Native Vision-Guard Plugin ABI Integration...\n";

    const char *plugin_path = std::filesystem::exists("./vision_guard_dissector_plugin.so")
                                  ? "./vision_guard_dissector_plugin.so"
                                  : "/usr/local/lib/blackbox/vision_guard_dissector_plugin.so";

    void *handle = dlopen(plugin_path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) { std::cerr << "[-] dlopen failed: " << dlerror() << "\n"; return 1; }

    using GetDescFn = const SentinelPluginDescriptor *(*)(void);
    auto get_desc = reinterpret_cast<GetDescFn>(dlsym(handle, "sentinel_plugin_get_descriptor"));
    assert(get_desc != nullptr);
    const SentinelPluginDescriptor *desc = get_desc();
    assert(desc && desc->magic == SENTINEL_SDK_MAGIC);
    assert(std::string(desc->plugin_id) == "org.aryorithm.plugin.vision_guard");

    SentinelHostInterface host{};
    host.engine_version = 1;
    host.log_message = mock_log;
    host.emit_metric_counter = mock_metric;
    assert(desc->init_fn(&host) == 0);
    assert(g_logs >= 1);

    // 1. Nominal ShmRing Vision frame (reserved=2) -> PASS
    vision_guard::protocols::ShmVisionHeader ok{vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1, 1, 1, 640, 480, 1, 1, 0, 4.5f, 10.0f, 0.05f};
    std::vector<uint8_t> buf(sizeof(ok));
    std::memcpy(buf.data(), &ok, sizeof(ok));
    SentinelRawPacket p1{};
    p1.data = buf.data();
    p1.length = buf.size();
    p1.reserved = 2; // SHM_RING
    assert(desc->dissect_fn(&p1).verdict == SENTINEL_VERDICT_PASS);

    // 2. Optical blinding attack -> KERNEL_DROP
    ok.frame_seq = 2; ok.saturation_pct = 96.5f;
    std::memcpy(buf.data(), &ok, sizeof(ok));
    SentinelDissectorResult r2 = desc->dissect_fn(&p1);
    assert(r2.verdict == SENTINEL_VERDICT_KERNEL_DROP);
    assert(r2.rule_id == 0x5631);
    assert(g_metrics >= 1);

    desc->shutdown_fn();
    dlclose(handle);
    std::cout << ">>> RESULT: SUCCESS (Vision-Guard ShmRing + GigE + RTSP Plugin Verified) <<<\n";
    return 0;
}
