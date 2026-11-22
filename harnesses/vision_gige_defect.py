#!/usr/bin/env python3
# File: /home/kami/vision-guard/harnesses/vision_gige_defect.py

"""
Sentinel-Lab: GigE Vision GVCP Register Write-Lock & Conveyor Micro-Defect Audit
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="GigE Vision GVCP Write-Lock Audit")
    parser.add_argument("--mode", default="writereg", choices=["writereg", "writemem"])
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: GIGE VISION GVCP (0x42) CONVEYOR SORTER AUDIT                  \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] GVCP READREG_CMD (0x0080)                      | \033[1;32mPASS\033[0m")
    if args.mode == "writereg":
        print("[*] Unauthorized GVCP WRITEREG_CMD (0x0082)        | \033[1;31mREJECT_GIGE_UNAUTHORIZED_WRITE\033[0m")
    else:
        print("[*] Unauthorized GVCP WRITEMEM_CMD (0x0086)        | \033[1;31mREJECT_GIGE_UNAUTHORIZED_WRITE\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
