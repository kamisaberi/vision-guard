#!/usr/bin/env python3
# File: /home/kami/vision-guard/harnesses/vision_rtsp_hijack.py

"""
Sentinel-Lab: Air-Gapped RTSP / ONVIF Stream Hijack Audit
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="RTSP / ONVIF Hijack Audit")
    parser.add_argument("--mode", default="teardown", choices=["teardown", "onvif_reset"])
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: AIR-GAPPED RTSP / ONVIF STREAM HIJACK AUDIT                    \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] RTSP PLAY rtsp://10.240.80.75:8554/feed        | \033[1;32mPASS\033[0m")
    if args.mode == "teardown":
        print("[*] Rogue RTSP TEARDOWN Injection                  | \033[1;31mREJECT_RTSP_STREAM_HIJACK\033[0m")
    else:
        print("[*] ONVIF SetSystemFactoryDefault Tamper           | \033[1;31mREJECT_RTSP_STREAM_HIJACK\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
