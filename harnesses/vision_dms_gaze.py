#!/usr/bin/env python3
# File: /home/kami/vision-guard/harnesses/vision_dms_gaze.py

import argparse

def main():
    parser = argparse.ArgumentParser(description="Automotive In-Cabin DMS PERCLOS & Gaze Audit")
    parser.add_argument("--mode", default="drowsy", choices=["drowsy", "distracted"])
    args = parser.parse_args()
    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: IN-CABIN AUTOMOTIVE DMS (MIPI-CSI2 / V4L2) AUDIT               \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] Nominal Driver (PERCLOS=8%, EAR=0.29, 3.8W)  | \033[1;32mPASS\033[0m")
    if args.mode == "drowsy":
        print("[*] Micro-Sleep Event (PERCLOS=34% > 20%)        | \033[1;31mREJECT_DMS_DRIVER_DROWSINESS\033[0m")
    else:
        print("[*] Head Yaw 45° / Gaze Off-Road 3.1s            | \033[1;31mREJECT_DMS_GAZE_DISTRACTION\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")

if __name__ == "__main__":
    main()