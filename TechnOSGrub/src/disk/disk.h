#pragma once

#include <stdbool.h>

#include "../include/stdint.h"
#include "../stdlib/string.h"
#include "../include/util.h"

struct DirectoryEntry {
    char name[11];         // 8 bytes name, 3 bytes extension
    uint8_t attr;          // Attributes
    uint8_t reserved[12];  // Reserved bits
    uint16_t time;         // Creation time
    uint16_t date;         // Creation date
    uint16_t cluster_low;  // Starting cluster number
};

static inline bool fileNameStrcmp(char* requestedFile, char* paddedFile) {
    uint8_t name_len = 0;
    while (requestedFile[name_len] != '\0' && requestedFile[name_len] != '.') {
        name_len++;
    }

    char padded[11] = "           ";

    memcpy(padded, requestedFile, name_len);
    memcpy(&padded[8], &requestedFile[name_len + 1], 3);

    return strcmpFixed(padded, paddedFile, 11);
}

void ata_read_sector(uint32_t lba, uint16_t *buffer);
void ata_write_sector(uint32_t lba, const uint16_t *buffer, bool slave);
void ata_read_sectors_burst(uint32_t lba, uint16_t *buffer, uint8_t sector_count);
void read_fat_file(uint32_t start_cluster, uint16_t *buffer);
void ata_io_wait(void);
uint32_t get_next_cluster(uint32_t current_cluster);
bool init_ata(void);

uint32_t find_file_start_cluster(const char* filename);