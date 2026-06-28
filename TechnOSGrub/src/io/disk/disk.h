#pragma once

#include <stdbool.h>

#include "../include/stdint.h"
#include "../../stdlib/string.h"
#include "../../include/util.h"

struct __attribute__((packed)) DirectoryEntry {
    char name[11];
    uint8_t attr;
    uint8_t reserved[8];      // was 12 — shrunk by 4 to make room below
    uint32_t file_size;       // NEW — keeps total struct size at 32 bytes
    uint16_t cluster_high;
    uint16_t time;
    uint16_t date;
    uint16_t cluster_low;
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
bool init_ata(void);
void ata_io_wait(void);

uint32_t find_file_start_cluster(const char* filename);
void read_fat_file(uint32_t start_cluster, uint16_t *buffer);
void read_fat_file_limited(uint32_t start_cluster, uint8_t *buffer, uint32_t max_bytes);
uint32_t get_next_cluster(uint32_t current_cluster);
void write_cluster_data(uint32_t cluster, const uint16_t *buffer);

uint32_t create_file(const char* filename);
int32_t write_file_data(const char *filename, const void *user_data, uint32_t total_bytes);
int32_t read_file_by_name(const char* filename, void* output_buffer, uint32_t max_bytes);

void testDisk(bool ataGood);