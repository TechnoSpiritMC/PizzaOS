# Disk structure:
#
# SECTION NAME   START LBA   START BYTE OFFSET   DESCRIPTION
# Boot/Kernel    0x000       (0x00000000)        Reserved for your bootloader and kernel binary.
# FAT            0x100       (0x00020000)        Stores cluster chains. Spans 32 sectors up to 0x11F.
# Root dir       0x400       (0x00080000)        Stores file metadata entries. Spans 32 sectors up to 0x41F.*
# Data Clusters  0x500       (0x000A0000)        Stores raw file contents.

# FAT:
#
# Each entry is 4 bytes long.
# 512 bytes per sector -> 128 entries per sector.
#
#
# VALUE         MEANING
# 0x00000000    Free Cluster
# 0x0FFFFFFF    End of File (EOF)

# Directory entries:
#
# struct __attribute__((packed)) DirectoryEntry {
#     char name[11];          // Offset 0:  8 bytes name, 3 bytes extension
#     uint8_t attr;           // Offset 11: File attributes
#     uint8_t reserved[8];    // Offset 12: Reserved padding
#     uint32_t file_size;     // Offset 20: Total size in bytes
#     uint16_t cluster_high;  // Offset 24: High 16-bits of starting cluster
#     uint16_t time;          // Offset 26: Modification time
#     uint16_t date;          // Offset 28: Modification date
#     uint16_t cluster_low;   // Offset 30: Low 16-bits of starting cluster
# };
#
# File names are saved in the following way:
# HELP.TXT -> "HELP    TXT"
# MAIN.C   -> "MAIN    C  "
# Deleted files start with 0xE5
# Empty slots start with a null byte.

# Data clusters follow standard FAT numbering rules, meaning Cluster 2 is the very first available data sector.
# To find the exact LBA sector or byte offset for a cluster number, use these formulas:
#
# LBA Sector = 0x500 + (Cluster - 2)
# Byte Offset = LBA Sector * 512
#
# entries = [struct.unpack(dir_entry_format, sector_data[i*32:(i+1)*32]) for i in range(16)]

import os
import struct
import math

dir_entry_format = "<11sB8sI4H"

class FileData:
    def __init__(self, file):
        with open(file, "r+b") as f:
            self.name      = f.name

            self.fileName = self.name.split(":")[0][:8].ljust(8)
            self.extension = self.name.split(":")[1][:3].ljust(3)
            self.fatName = self.fileName + self.extension

            self.size      = os.path.getsize(file)
            self.data      = f.read()

        print(f"Opened a file: {self.name} (Saved format: \"{''.join([self.fileName, self.extension])}\"). Size: {self.size}")

    def getName(self):
        return self.name

    def getFatName(self):
        return "".join([self.fileName, self.extension])

    def getSize(self):
        return self.size

    def getData(self):
        return self.data

class FAT:
    def __init__(self, path, fileData: FileData):
        self.fileData = fileData
        self.path = path

        with open(path, "r+b") as f:
            self.content = list(f.read())
            self.entries = [struct.unpack(dir_entry_format, self.content[(0x80000 + i*32):(0x80000 + (i+1)*32)]) for i in range(16)]

        self.emptySlots = []

        for i in range(16):
            if self.entries[i][0] == 0xE5 or self.entries[i][0] == 0x00:
                print("Found empty slot: ", self.entries)
                self.emptySlots.append(i)

        self.emptyClusters = []

        for i in range(256):
            if self.content[(0x20000 + 4*i):(0x20000 + 4*(i+1))] == b'\x00\x00\x00\x00':
                print("Found free cluster: ", i)
                self.emptyClusters.append(i)

        self.requiredClusters = math.ceil(self.fileData.getSize() / 512)

        if len(self.emptyClusters) >= self.requiredClusters:
            print("Enough free clusters found!")
        else:
            print("Not enough free clusters found!")
            raise Exception("Not enough free clusters found!")

        for i, cluster in enumerate(self.emptyClusters[0:self.requiredClusters]):
            for j in range(512):
                self.content[(0x500 + (cluster-2)) * 512 + j] = self.fileData.getData()[512*i + j]


        for index in range(self.requiredClusters):
            self.content[(0x20000 + 4*index):(0x20000 + 4*(index+1))] = struct.pack("<I", (self.emptyClusters[index+1] if index < len(self.emptyClusters)-1 else 0x0fffffff))

        cluster = self.emptyClusters[0]
        cluster_high = (cluster >> 16) & 0xFFFF
        cluster_low = cluster & 0xFFFF

        self.content[(0x80000 + self.emptySlots[0] * 32):(0x80000 + (self.emptySlots[0] + 1) * 32)] = struct.pack(
                dir_entry_format,
                self.fileData.getFatName().encode(),
                0x00,
                b'\x00\x00\x00\x00\x00\x00\x00\x00',
                self.fileData.getSize(),
                cluster_high,
                0,
                0,
                cluster_low
        )

        self.binary = bytes(self.content)

        with open(path, "wb") as f:
            f.write(self.binary)