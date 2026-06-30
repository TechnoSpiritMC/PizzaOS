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

import os

class FileData:
    def __init__(self, file):
        with open(file, "r+b") as f:
            
            self.name      = f.name
            self.fileName  = "".join([(self.name.split(":")[0]+ "       ")[i] for i in range(7)])
            self.extension = "".join([(self.name.split(":")[1]+ "   ")[i]     for i in range(3)])
            self.size      = os.path.getsize(f)

            
    
