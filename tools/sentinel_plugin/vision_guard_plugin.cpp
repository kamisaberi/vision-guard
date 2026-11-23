// File: /home/kami/vision-guard/tools/sentinel_plugin/vision_guard_plugin.cpp

#include "sentinel/sdk/abi.hpp"
#include "vision_guard/vision_guard.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <span>
#include <string>

static const SentinelHostInterface *g_host = nullptr;
static std::unique_ptr<vision_guard::pipeline::GuardEngine> g_guard_engine;

static int plugin_init(const SentinelHostInterface *host_iface)
{
    g_host = host_iface;
    g_guard_engine = std::make_unique<vision_guard::pipeline::GuardEngine>();
    vision_guard::core::TscClock::calibrate();

    if (g_host && g_host->log_message)
    {
        g_host->log_message(1, "VisionGuard",
                            "Air-Gapped ShmRing Vision, GigE Vision & RTSP/ONVIF Plugin Initialized (< 120 ns SLA).");
    }
    return 0;
}

static void plugin_shutdown()
{
    g_guard_engine.reset();
    g_host = nullptr;
}

static SentinelDissectorResult plugin_dissect(const SentinelRawPacket *packet)
{
    SentinelDissectorResult res{};
    res.verdict = SENTINEL_VERDICT_PASS;
    res.threat_score = 0;
    res.rule_id = 0;
    res.semantic_tags = 0;
    res.threat_name[0] = '\0';

    if (!packet || !packet->data || packet->length < 8 || !g_guard_engine)
    {
        return res;
    }

    vision_guard::GuardStatus st = vision_guard::GuardStatus::PASS;
    uint32_t matched_rule_id = 0;

    // 1. Bus D: Raw V4L2 / MIPI-CSI2 Camera Frame (reserved == 4)
    if (packet->reserved == 4)
    {
        st = g_guard_engine->process_v4l2_frame(std::span<const uint8_t>(packet->data, packet->length));
        matched_rule_id = 0x0056;
    }
    // 2. Bus C: POSIX /dev/shm Ring (reserved == 2) — "VIS1" or "DMS1"
    else if (packet->reserved == 2 && packet->length >= 4)
    {
        uint32_t magic = 0;
        std::memcpy(&magic, packet->data, 4);
        const std::span<const uint8_t> span(packet->data, packet->length);
        if (magic == vision_guard::protocols::ShmVisionValidator::MAGIC_VIS1)
        {
            st = g_guard_engine->process_shm_vision(span);
            matched_rule_id = 0x5631;
        }
        else if (magic == vision_guard::protocols::DmsGazeValidator::MAGIC_DMS1)
        {
            st = g_guard_engine->process_dms_gaze(span);
            matched_rule_id = 0x444D;
        }
    }
    else
    {
        // 2. Bus A: IPv4 TCP RTSP (:8554) or UDP GigE Vision GVCP (:3956)
        const uint8_t *payload_ptr = packet->data;
        size_t payload_len = packet->length;
        uint16_t dst_port = 0;
        uint8_t l4_proto = packet->transport_proto;

        if (packet->length >= 34 && packet->data[12] == 0x08 && packet->data[13] == 0x00)
        {
            const size_t ihl = static_cast<size_t>(packet->data[14] & 0x0F) * 4U;
            const size_t l4_off = 14U + ihl;
            l4_proto = packet->data[23];
            if (ihl >= 20 && l4_off + 8U <= packet->length)
            {
                dst_port = (static_cast<uint16_t>(packet->data[l4_off + 2]) << 8) | packet->data[l4_off + 3];
                if (l4_proto == 17)
                {
                    payload_ptr = packet->data + l4_off + 8U;
                    payload_len = packet->length - (l4_off + 8U);
                }
                else if (l4_proto == 6 && l4_off + 20U <= packet->length)
                {
                    const size_t tcp_hlen = static_cast<size_t>((packet->data[l4_off + 12] >> 4) & 0x0F) * 4U;
                    const size_t off = l4_off + tcp_hlen;
                    if (tcp_hlen < 20 || off >= packet->length)
                        return res;
                    payload_ptr = packet->data + off;
                    payload_len = packet->length - off;
                }
            }
        }

        std::span<const uint8_t> frame(payload_ptr, payload_len);
        const bool is_gige = (dst_port == 3956) || (dst_port == 0 && l4_proto == 17 && payload_len >= 8 && payload_ptr[0] == 0x42);
        const bool is_rtsp = (dst_port == 8554 || dst_port == 3702) || (dst_port == 0 && l4_proto == 6 && payload_len >= 8);

        if (is_gige)
        {
            st = g_guard_engine->process_gige_gvcp(frame);
            matched_rule_id = 0x3956;
        }
        else if (is_rtsp)
        {
            st = g_guard_engine->process_rtsp_onvif(frame);
            matched_rule_id = 0x8554;
        }
    }

    if (st != vision_guard::GuardStatus::PASS)
    {
        res.verdict = SENTINEL_VERDICT_KERNEL_DROP;
        res.threat_score = 990;
        res.rule_id = matched_rule_id;
        std::strncpy(res.threat_name, vision_guard::status_to_string(st).data(), sizeof(res.threat_name) - 1);

        if (g_host)
        {
            if (g_host->log_message)
            {
                std::string msg = "VISION PRIVACY TRIPWIRE: (" + std::string(res.threat_name) + ")";
                g_host->log_message(3, "VisionGuard", msg.c_str());
            }
            if (g_host->emit_metric_counter)
            {
                g_host->emit_metric_counter("vision_privacy_airgap_trips", 1);
            }
        }
        return res;
    }

    res.threat_score = static_cast<uint32_t>(std::min<size_t>(packet->length, 150U));
    return res;
}

static const SentinelPluginDescriptor g_descriptor = {
    .magic = SENTINEL_SDK_MAGIC,
    .sdk_version_major = SENTINEL_SDK_VERSION_MAJOR,
    .sdk_version_minor = SENTINEL_SDK_VERSION_MINOR,
    .tier = SENTINEL_TIER_NATIVE_CPP,
    .plugin_id = "org.aryorithm.plugin.vision_guard",
    .plugin_name = "Air-Gapped ShmRing Vision, GigE Vision & RTSP/ONVIF Privacy Guard",
    .plugin_version = "1.0.0",
    .target_protocol = "SHMRING_GIGE_RTSP_ONVIF",
    .default_port = 8554,
    .reserved = 0,
    .init_fn = plugin_init,
    .shutdown_fn = plugin_shutdown,
    .dissect_fn = plugin_dissect};

extern "C" ARY_SDK_API const SentinelPluginDescriptor *sentinel_plugin_get_descriptor(void)
{
    return &g_descriptor;
}
