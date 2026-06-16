#include "disk.h"

#include "../include/util.h"

#define DATA_START_LBA 0x500

#define ATA_CMD_IDENTIFY 0xEC

bool init_ata(void) {
    // 1. Soft reset the controller by setting the SRST bit in the Control Register
    outPortB(0x3F6, 0x04);
    ata_io_wait();
    outPortB(0x3F6, 0x00); // Clear the reset bit
    ata_io_wait();

    // 2. Select the master drive
    outPortB(0x1F6, 0xA0);
    ata_io_wait();

    // 3. Set sector count and LBA registers to 0
    outPortB(0x1F2, 0);
    outPortB(0x1F3, 0);
    outPortB(0x1F4, 0);
    outPortB(0x1F5, 0);

    // 4. Send the IDENTIFY command
    outPortB(0x1F7, ATA_CMD_IDENTIFY);
    ata_io_wait();

    // 5. Check if the drive exists. If status is 0, there is no drive.
    uint8_t status = inPortB(0x1F7);
    if (status == 0) return false;

    // 6. Wait until the drive clears the busy bit (BSY) and sets the data ready bit (DRQ)
    while ((inPortB(0x1F7) & 0x80) != 0); // Wait for BSY to clear

    if ((inPortB(0x1F7) & 0x08) == 0) {
        return false; // Error: Drive didn't set Data Request
    }

    // 7. Read the 512 bytes of identification data to clear the buffer
    uint16_t id_buffer[256];
    for (int i = 0; i < 256; i++) {
        id_buffer[i] = inW(0x1F0);
    }

    return true; // Drive is active and ready for action!
}

void ata_read_sector(uint32_t lba, uint16_t *buffer) {
    // 1. Select the drive and pass the highest bits of the LBA
    outPortB(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));

    // 2. Set the sector count to 1
    outPortB(0x1F2, 1);

    // 3. Send the rest of the LBA bits
    outPortB(0x1F3, (uint8_t)lba);          // Low 8 bits
    outPortB(0x1F4, (uint8_t)(lba >> 8));   // Mid 8 bits
    outPortB(0x1F5, (uint8_t)(lba >> 16));  // High 8 bits

    // 4. Send the READ command (0x20)
    outPortB(0x1F7, 0x20);

    // 5. Wait for the drive to finish fetching data
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        // Polling...
    }

    // 6. Read the 256 words into our buffer
    for (int i = 0; i < 256; i++) {
        buffer[i] = inW(0x1F0);
    }
}

void ata_read_sectors_burst(uint32_t lba, uint16_t *buffer, uint8_t sector_count) {
    // 1. Select the drive and pass the highest bits of the LBA
    outPortB(0x1F6, 0xF0 | ((lba >> 24) & 0x0F));

    // 2. Set the total sector count up front!
    outPortB(0x1F2, sector_count);

    // 3. Send the LBA bits
    outPortB(0x1F3, (uint8_t)lba);
    outPortB(0x1F4, (uint8_t)(lba >> 8));
    outPortB(0x1F5, (uint8_t)(lba >> 16));

    // 4. Send the READ command (0x20)
    outPortB(0x1F7, 0x20);

    // 5. Wait for the drive to finish fetching data
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        // Polling...
    }

    // 6. Read ALL the words sequentially
    uint32_t total_words = sector_count * 256;
    for (uint32_t i = 0; i < total_words; i++) {
        buffer[i] = inW(0x1F0);
    }
}

void ata_write_sector(uint32_t lba, const uint16_t *buffer, bool slave) {
    // 1. Select the drive (Master/Slave) and pass the highest bits of the LBA
    outPortB(0x1F6, 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F));

    // 2. Set the sector count to 1
    outPortB(0x1F2, 1);

    // 3. Send the rest of the LBA bits
    outPortB(0x1F3, (uint8_t)lba);
    outPortB(0x1F4, (uint8_t)(lba >> 8));
    outPortB(0x1F5, (uint8_t)(lba >> 16));

    // 4. Send the WRITE command (0x30)
    outPortB(0x1F7, 0x30);

    // 5. Wait for the drive to be ready to accept data
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        // Polling...
    }

    // 6. Write the 256 words from our buffer to the disk
    for (int i = 0; i < 256; i++) {
        outW(0x1F0, buffer[i]);
    }
}

void read_fat_file(uint32_t start_cluster, uint16_t *buffer) {
    uint32_t current_cluster = start_cluster;

    while (current_cluster != 0x0FFFFFFF) {
        // 1. Calculate the LBA address for the current cluster
        uint32_t lba = DATA_START_LBA + (current_cluster - 2);

        // 2. Read the sector into the current buffer location
        ata_read_sectors_burst(lba, buffer, 1);

        // 3. Move the buffer pointer forward by one sector (256 words)
        buffer += 256;

        // 4. Get the next cluster from the File Allocation Table
        current_cluster = get_next_cluster(current_cluster);
    }
}

void load_file_by_name(const char* filename, uint16_t *output_buffer) {
    // 1. Find where the file starts
    int32_t start_cluster = find_file_start_cluster(filename);

    // 2. Check if the file actually exists
    if (start_cluster == -1) {
        // Handle error: File not found!
        return;
    }

    // 3. Read the file into memory using our chain-linker loop
    read_fat_file((uint32_t)start_cluster, output_buffer);
}

#define FAT_START_LBA 0x100

uint32_t get_next_cluster(uint32_t current_cluster) {
    // Each FAT32 entry is 4 bytes (32 bits).
    // A 512-byte sector can hold (512 / 4) = 128 entries.
    uint32_t fat_sector = FAT_START_LBA + (current_cluster / 128);
    uint32_t fat_offset = current_cluster % 128;

    // Temporary buffer to hold the FAT sector (256 words = 512 bytes)
    uint16_t fat_buffer[256];

    // Read the specific FAT sector from disk
    ata_read_sectors_burst(fat_sector, fat_buffer, 1);

    // Cast our buffer to a 32-bit pointer so we can read it as an array of uint32_t
    uint32_t *fat_entries = (uint32_t *)fat_buffer;

    // FAT32 ignores the top 4 bits, so we mask them out with 0x0FFFFFFF
    return fat_entries[fat_offset] & 0x0FFFFFFF;
}

#define ROOT_DIR_START_LBA 0x400

uint32_t find_file_start_cluster(const char* filename) {
    uint16_t dir_buffer[256]; // 512 bytes

    // Read the root directory sector
    ata_read_sectors_burst(ROOT_DIR_START_LBA, dir_buffer, 1);

    // Cast to our DirectoryEntry structure array (16 entries per sector)
    struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;

    for (int i = 0; i < 16; i++) {
        // If the first byte of the name is 0x00, the rest of the directory is empty
        if (entries[i].name[0] == 0x00) {
            break;
        }

        // If the first byte is 0xE5, the file was deleted, skip it
        if ((uint8_t)entries[i].name[0] == 0xE5) {
            continue;
        }

        // Use your string comparison function (cast const char* to char*)
        if (fileNameStrcmp((char*)filename, entries[i].name)) {
            return entries[i].cluster_low; // Found it!
        }
    }

    return -1; // File not found
}


uint32_t create_file(const char* filename) {
    // Step A: Check if the file already exists
    if (find_file_start_cluster(filename) != -1) {
        return -1; // Error: File already exists
    }

    // Step B: Find a free cluster in the FAT table
    uint32_t free_cluster = 0;
    bool cluster_found = false;
    uint16_t fat_buffer[256];
    uint32_t current_fat_sector = FAT_START_LBA;

    // Search the FAT table sectors
    for (uint32_t sector_offset = 0; sector_offset < 32; sector_offset++) {
        ata_read_sectors_burst(current_fat_sector, fat_buffer, 1);
        uint32_t *fat_entries = (uint32_t *)fat_buffer;

        for (int i = 0; i < 128; i++) {
            if (fat_entries[i] == 0x00000000) {
                free_cluster = sector_offset * 128 + i;
                fat_entries[i] = 0x0FFFFFFF; // Mark as end of file
                ata_write_sector(current_fat_sector, fat_buffer, false);
                cluster_found = true;
                break;
            }
        }
        if (cluster_found) break;
        current_fat_sector++;
    }

    if (!cluster_found) return -1; // Error: Disk is full

    // Step C: Find a free slot in the Root Directory
    uint32_t current_dir_sector = ROOT_DIR_START_LBA;
    uint16_t dir_buffer[256];
    bool slot_found = false;
    uint32_t free_entry_index = 0;

    while (!slot_found) {
        ata_read_sectors_burst(current_dir_sector, dir_buffer, 1);
        struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                free_entry_index = i;
                slot_found = true;
                break;
            }
        }

        current_dir_sector++;
        if (current_dir_sector >= ROOT_DIR_START_LBA + 32) {
            return -1; // Error: Root directory full
        }
    }

    // Step D: Format the name and populate the entry
    uint8_t name_len = 0;
    while (filename[name_len] != '\0' && filename[name_len] != '.') {
        name_len++;
    }

    char padded[11] = "           ";
    memcpy(padded, filename, name_len);
    if (filename[name_len] == '.') {
        memcpy(&padded[8], &filename[name_len + 1], 3);
    }

    struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;
    memcpy(entries[free_entry_index].name, padded, 11);
    entries[free_entry_index].attr = 0x00;
    entries[free_entry_index].cluster_low = (uint16_t)(free_cluster & 0xFFFF);
    // You could also set size, time, and date fields to 0 here

    // Step E: Commit changes to disk
    ata_write_sector(current_dir_sector, dir_buffer, false);

    return free_cluster;
}

void ata_io_wait(void) {
    inPortB(0x80);
    inPortB(0x80);
}