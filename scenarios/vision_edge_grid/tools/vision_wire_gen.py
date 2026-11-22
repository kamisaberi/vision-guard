#!/usr/bin/env python3
# File: /home/kami/vision-guard/scenarios/vision_edge_grid/tools/vision_wire_gen.py

import mmap
import os
import socket
import struct
import time

SHM_PATH = "/dev/shm/sentinel_vision_ring"
SHM_MAGIC = 0x53484D52


def try_write_shm_vision(seq: int):
    if not os.path.exists(SHM_PATH):
        return False
    try:
        fd = os.open(SHM_PATH, os.O_RDWR)
        size = os.fstat(fd).st_size
        if size < 64 + 64 * 4096:
            os.close(fd)
            return False
        mm = mmap.mmap(fd, size)
        magic, _ver, slot_cnt, slot_sz = struct.unpack("<IIII", mm[0:16])
        if magic != SHM_MAGIC or slot_cnt == 0:
            mm.close()
            os.close(fd)
            return False

        wseq = struct.unpack("<Q", mm[16:24])[0]
        slot_off = 64 + (wseq % slot_cnt) * slot_sz
        vis_payload = struct.pack("<IIIHHBBHfff", 0x56495331, 1, seq, 640, 480, 1, 1, 0, 4.5, 12.0, 0.05)
        slot_hdr = struct.pack("<QIHH", wseq, len(vis_payload), 1, 2)
        mm[slot_off:slot_off + 16] = slot_hdr
        mm[slot_off + 16:slot_off + 16 + len(vis_payload)] = vis_payload
        mm[16:24] = struct.pack("<Q", wseq + 1)
        mm.close()
        os.close(fd)
        return True
    except Exception:
        return False


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    seq = 1
    while True:
        # 1. GigE Vision GVCP READREG (:3956) -> Edge-Vision-Conveyor-02
        gvcp = struct.pack(">BBHHH", 0x42, 0x01, 0x0080, 4, seq & 0xFFFF)
        try:
            sock.sendto(gvcp, ("10.240.80.102", 3956))
        except Exception:
            pass

        # 2. POSIX /dev/shm Lock-Free Vision Frame ("VIS1") -> Edge-Vision-HIPAA-03
        try_write_shm_vision(seq)

        seq += 1
        time.sleep(0.4)


if __name__ == "__main__":
    main()
