#include "disk.h"

#include "../../include/util.h"
#include "../../stdlib/serial.h"

#define DATA_START_LBA 0x500
#define FAT_START_LBA 0x100
#define ROOT_DIR_START_LBA 0x400

#define ATA_CMD_IDENTIFY 0xEC

// Iteration cap for busy-wait loops. Not a real time unit, just a large
// bound so a stuck drive/controller logs an error instead of hanging the
// kernel forever. Tune this up if you see false-positive timeouts on real
// hardware (slower than QEMU).
#define ATA_WAIT_TIMEOUT 2000000

bool init_ata(void) {

    //serial_printf("Initializing ATA...\n\r");

    // 1. Soft reset the controller by setting the SRST bit in the Control Register
    outPortB(0x3F6, 0x04);
    ata_io_wait();
    outPortB(0x3F6, 0x00); // Clear the reset bit
    ata_io_wait();

    //serial_printf("Reset the controller.\n\r");

    // 2. Select the master drive
    outPortB(0x1F6, 0xA0);
    ata_io_wait();

    //serial_printf("Selected master drive.\n\r");

    // 3. Set sector count and LBA registers to 0
    outPortB(0x1F2, 0);
    outPortB(0x1F3, 0);
    outPortB(0x1F4, 0);
    outPortB(0x1F5, 0);

    //serial_printf("Set sector count and lba registers to 0\n\r");

    // 4. Send the IDENTIFY command
    outPortB(0x1F7, ATA_CMD_IDENTIFY);
    ata_io_wait();

    //serial_printf("Required drive to identify itself\n\r");

    // 5. Check if the drive exists. If status is 0, there is no drive.
    uint8_t status = inPortB(0x1F7);

    //serial_printf("Gor answer: %x\n\r", status);

    if (status == 0) {
        //serial_printf("Drive not found\n\r");
        return false;
    }

    uint64_t i = 0;
    // 6. Wait until the drive clears the busy bit (BSY) and sets the data ready bit (DRQ)
    while ((inPortB(0x1F7) & 0x80) != 0) {
        if (++i > ATA_WAIT_TIMEOUT) {
            //serial_printf("init_ata: TIMEOUT waiting for BSY to clear. status=%x error=%x\n\r", inPortB(0x1F7), inPortB(0x1F1));
            return false;
        }
    }

    if ((inPortB(0x1F7) & 0x08) == 0) {
        //serial_printf("Drive didn't set Data Request\n\r");
        return false; // Error: Drive didn't set Data Request
    }

    // 7. Read the 512 bytes of identification data to clear the buffer
    uint16_t id_buffer[256];
    for (int i = 0; i < 256; i++) {
        id_buffer[i] = inW(0x1F0);
    }

    //serial_printf("Oke stoopid drive is ready\n\r");

    return true; // Drive is active and ready for action!
}

void ata_read_sector(uint32_t lba, uint16_t *buffer) {
    // 1. Select the drive and pass the highest bits of the LBA
    // NOTE: 0xE0 = master, not 0xF0 (which selects the slave). This function
    // is currently unused (ata_read_sectors_burst replaced it) but fixed for
    // consistency in case it gets revived later.
    outPortB(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_wait(); // Wait 400ns for the drive select to stabilize

    // 2. Set the sector count to 1
    outPortB(0x1F2, 1);

    // 3. Send the rest of the LBA bits
    outPortB(0x1F3, (uint8_t)lba);
    outPortB(0x1F4, (uint8_t)(lba >> 8));
    outPortB(0x1F5, (uint8_t)(lba >> 16));

    // 4. Send the READ command (0x20)
    outPortB(0x1F7, 0x20);
    ata_io_wait(); // Wait 400ns for the status register to update

    // 5. Wait for the drive to finish fetching data
    uint64_t counter = 0;
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        if (++counter > ATA_WAIT_TIMEOUT) {
            //serial_printf("ata_read_sector: TIMEOUT lba=%x status=%x error=%x\n\r", lba, inPortB(0x1F7), inPortB(0x1F1));
            return;
        }
    }

    // 6. Read the 256 words into our buffer
    for (int i = 0; i < 256; i++) {
        buffer[i] = inW(0x1F0);
    }
}

void ata_read_sectors_burst(uint32_t lba, uint16_t *buffer, uint8_t sector_count) {

    //serial_printf("Burst reading %x to %x (%x sectors)\n\r", lba, buffer, sector_count);

    // 1. Select the drive and pass the highest bits of the LBA
    outPortB(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_wait(); // Safeguard 1: Wait for drive selection to stabilize

    //serial_printf("First wait\n\r");

    // 2. Set the total sector count up front!
    outPortB(0x1F2, sector_count);

    // 3. Send the LBA bits
    outPortB(0x1F3, (uint8_t)lba);
    outPortB(0x1F4, (uint8_t)(lba >> 8));
    outPortB(0x1F5, (uint8_t)(lba >> 16));

    // 4. Send the READ command (0x20)
    outPortB(0x1F7, 0x20);
    ata_io_wait(); // Safeguard 2: Wait for status register to refresh

    //serial_printf("Second wait\n\r");

    uint64_t counter = 0;

    // 5. Wait for the drive to finish fetching data
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        if (++counter > ATA_WAIT_TIMEOUT) {
            //serial_printf("ata_read_sectors_burst: TIMEOUT lba=%x status=%x error=%x\n\r", lba, inPortB(0x1F7), inPortB(0x1F1));
            return; // bail out — buffer is left untouched/stale, caller should treat this as a failed read
        }
    }

    //serial_printf("Reading sectors: ");

    // 6. Read ALL the words sequentially
    uint32_t total_words = sector_count * 256;
    for (uint32_t i = 0; i < total_words; i++) {
        buffer[i] = inW(0x1F0);
        //serial_printf("%x ", buffer[i]);
    }

    //serial_printf("Done reading %x sectors\n\r", sector_count);
}

void ata_write_sector(uint32_t lba, const uint16_t *buffer, bool slave) {

    //serial_printf("Writing new sector to lba: %x, buffer %x, slave? %x\n\r", lba, buffer, slave);

    outPortB(0x1F6, 0xE0 | (slave << 4) | ((lba >> 24) & 0x0F));
    ata_io_wait();

    outPortB(0x1F2, 1);
    outPortB(0x1F3, (uint8_t)lba);
    outPortB(0x1F4, (uint8_t)(lba >> 8));
    outPortB(0x1F5, (uint8_t)(lba >> 16));

    outPortB(0x1F7, 0x30);
    ata_io_wait();

    uint64_t counter = 0;
    while ((inPortB(0x1F7) & 0x80) || !(inPortB(0x1F7) & 0x08)) {
        if (++counter > ATA_WAIT_TIMEOUT) {
            //serial_printf("ata_write_sector: TIMEOUT waiting for DRQ before write. lba=%x status=%x error=%x\n\r", lba, inPortB(0x1F7), inPortB(0x1F1));
            return;
        }
    }

    for (int i = 0; i < 256; i++) {
        outW(0x1F0, buffer[i]);
    }

    // Wait for the drive to finish committing the write before returning,
    // so the next command issued sees a clean, settled controller state.
    ata_io_wait();
    counter = 0;
    while (inPortB(0x1F7) & 0x80) {
        if (++counter > ATA_WAIT_TIMEOUT) {
            //serial_printf("ata_write_sector: TIMEOUT waiting for BSY to clear after write. lba=%x status=%x error=%x\n\r", lba, inPortB(0x1F7), inPortB(0x1F1));
            return;
        }
    }

    //serial_printf("Write to lba %x completed.\n\r", lba);
}

void read_fat_file(uint32_t start_cluster, uint16_t *buffer) {
    uint32_t current_cluster = start_cluster;

    //serial_printf("Started reading file (unbounded).\n\r");

    while (current_cluster != 0x0FFFFFFF) {
        // 1. Calculate the LBA address for the current cluster
        uint32_t lba = DATA_START_LBA + (current_cluster - 2);

        //serial_printf("Calculated lba: %x. Burst reading sectors:\n\r", lba);

        // 2. Read the sector into the current buffer location
        ata_read_sectors_burst(lba, buffer, 1);

        //serial_printf("Read everything into the buffer.\n\r");

        // 3. Move the buffer pointer forward by one sector (256 words)
        buffer += 256;

        //serial_printf("Moved buffer pointer.\n\r");

        // 4. Get the next cluster from the File Allocation Table
        current_cluster = get_next_cluster(current_cluster);

        //serial_printf("Next cluster: %x\n\r", current_cluster);
    }
}

void read_fat_file_limited(uint32_t start_cluster, uint8_t *buffer, uint32_t max_bytes) {
    uint32_t current_cluster = start_cluster;
    uint32_t bytes_copied = 0;
    uint16_t sector_buffer[256];

    //serial_printf("Started bounded read. start_cluster=%x max_bytes=%x\n\r", start_cluster, max_bytes);

    while (current_cluster != 0x0FFFFFFF && bytes_copied < max_bytes) {
        uint32_t lba = DATA_START_LBA + (current_cluster - 2);

        //serial_printf("Bounded read: cluster=%x lba=%x bytes_copied_so_far=%x\n\r", current_cluster, lba, bytes_copied);

        ata_read_sectors_burst(lba, sector_buffer, 1);

        uint32_t remaining = max_bytes - bytes_copied;
        uint32_t to_copy = remaining < 512 ? remaining : 512;
        memcpy(buffer + bytes_copied, sector_buffer, to_copy);
        bytes_copied += to_copy;

        current_cluster = get_next_cluster(current_cluster);
    }

    //serial_printf("Bounded read finished. Total bytes copied: %x\n\r", bytes_copied);
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
    // NOTE: still unbounded — caller must ensure output_buffer is large
    // enough for the file's full cluster chain. Prefer read_file_by_name
    // (which uses read_fat_file_limited) when you have a known buffer size.
    read_fat_file((uint32_t)start_cluster, output_buffer);
}

uint32_t get_next_cluster(uint32_t current_cluster) {
    uint32_t sector_offset = current_cluster / 128;
    uint32_t entry_index = current_cluster % 128;

    uint32_t sector_to_read = FAT_START_LBA + sector_offset;
    uint16_t fat_buffer[256]; // 512 bytes total

    // 1. Read the sector into our buffer
    ata_read_sectors_burst(sector_to_read, fat_buffer, 1);

    // 2. Cast the buffer to 32-bit integers so we can read FAT32 entries
    uint32_t *fat_entries = (uint32_t *)fat_buffer;

    // 3. Return the next cluster number
    return fat_entries[entry_index];
}

uint32_t find_file_start_cluster(const char* filename) {
    uint16_t dir_buffer[256]; // 512 bytes

    //serial_printf("find_file_start_cluster: looking for %s\n\r", filename);

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
            uint32_t cluster = entries[i].cluster_low | ((uint32_t)entries[i].cluster_high << 16);
            //serial_printf("find_file_start_cluster: found %s at cluster %x\n\r", filename, cluster);
            return cluster; // Found it!
        }
    }

    //serial_printf("find_file_start_cluster: %s not found\n\r", filename);
    return 0xFFFFFFFF; // File not found (NOTE: as unsigned, this is -1)
}

// Scans the root directory (across all its sectors, matching create_file's
// search range) for filename and, if found, overwrites its file_size field
// on disk. Returns true on success, false if the entry wasn't found.
static bool update_file_size_entry(const char* filename, uint32_t new_size) {
    uint16_t dir_buffer[256];
    uint32_t current_dir_sector = ROOT_DIR_START_LBA;

    while (current_dir_sector < ROOT_DIR_START_LBA + 32) {
        ata_read_sectors_burst(current_dir_sector, dir_buffer, 1);
        struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) break;
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;

            if (fileNameStrcmp((char*)filename, entries[i].name)) {
                entries[i].file_size = new_size;
                ata_write_sector(current_dir_sector, dir_buffer, false);
                //serial_printf("update_file_size_entry: set %s size to %x (sector %x)\n\r", filename, new_size, current_dir_sector);
                return true;
            }
        }

        current_dir_sector++;
    }

    //serial_printf("update_file_size_entry: couldn't find %s to update size\n\r", filename);
    return false;
}

uint32_t create_file(const char* filename) {
    // Step A: Check if the file already exists
    if (find_file_start_cluster(filename) != 0xFFFFFFFF) {
        //serial_printf("create_file: %s already exists\n\r", filename);
        return 0xFFFFFFFF; // Error: File already exists
    }

    //serial_printf("create_file: %s doesn't exist yet, proceeding.\n\r", filename);

    // Step B: Find a free cluster in the FAT table
    uint32_t free_cluster = 0;
    bool cluster_found = false;
    uint16_t fat_buffer[256];
    uint32_t current_fat_sector = FAT_START_LBA;

    // Search the FAT table sectors
    for (uint32_t sector_offset = 0; sector_offset < 32; sector_offset++) {
        ata_read_sectors_burst(current_fat_sector, fat_buffer, 1);
        uint32_t *fat_entries = (uint32_t *)fat_buffer;

        //serial_printf("create_file: scanning FAT sector %x, first entry=%x\n\r", current_fat_sector, *fat_entries);

        for (int i = 0; i < 128; i++) {
            uint32_t candidate = sector_offset * 128 + i;

            // Clusters 0 and 1 are reserved (0 collides with the "FAT entry
            // is free" sentinel, 1 is reserved by convention in real FAT) —
            // never hand them out as real file clusters.
            if (candidate < 2) continue;

            if (fat_entries[i] == 0x00000000) {
                free_cluster = candidate;
                fat_entries[i] = 0x0FFFFFFF; // Mark as end of file
                ata_write_sector(current_fat_sector, fat_buffer, false);
                cluster_found = true;
                //serial_printf("create_file: allocated cluster %x\n\r", free_cluster);
                break;
            }
        }
        if (cluster_found) break;
        current_fat_sector++;
    }

    if (!cluster_found) {
        //serial_printf("create_file: disk full, no free cluster\n\r");
        return 0xFFFFFFFF; // Error: Disk is full
    }

    // Step C: Find a free slot in the Root Directory
    uint32_t current_dir_sector = ROOT_DIR_START_LBA;
    uint16_t dir_buffer[256];
    bool slot_found = false;
    uint32_t free_entry_index = 0;

    while (true) {
        ata_read_sectors_burst(current_dir_sector, dir_buffer, 1);
        struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                free_entry_index = i;
                slot_found = true;
                goto end;
            }
        }

        current_dir_sector++;
        if (current_dir_sector >= ROOT_DIR_START_LBA + 32) {
            //serial_printf("create_file: root directory full\n\r");
            return 0xFFFFFFFF; // Error: Root directory full
        }
    }

    end:

    //serial_printf("create_file: writing new entry at dir sector %x, index %x\n\r", current_dir_sector, free_entry_index);

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
    entries[free_entry_index].cluster_high = (uint16_t)(free_cluster >> 16);
    entries[free_entry_index].file_size = 0; // new file starts empty
    // You could also set time/date fields here

    ata_write_sector(current_dir_sector, dir_buffer, false);

    return free_cluster;
}

void write_cluster_data(uint32_t cluster, const uint16_t *buffer) {
    ata_write_sector(DATA_START_LBA + (cluster - 2), buffer, false);
}

int32_t write_file_data(const char *filename, const void *user_data, uint32_t total_bytes) {

    //serial_printf("write_file_data: looking up %s\n\r", filename);

    // 1. Find where the file starts on the disk
    uint32_t current_cluster = find_file_start_cluster(filename);

    //serial_printf("write_file_data: start cluster = %x\n\r", current_cluster);

    // find_file_start_cluster returns 0xFFFFFFFF (i.e. -1 as unsigned) when
    // the file doesn't exist. 0 is a real, if reserved, value — don't treat
    // it as "not found", or a write to a legitimately-zero cluster will be
    // silently skipped.
    if (current_cluster == 0xFFFFFFFF) {
        //serial_printf("write_file_data: file not found\n\r");
        return -1;
    }

    uint32_t bytes_written = 0;

    //serial_printf("write_file_data: starting write loop for %x bytes\n\r", total_bytes);

    // 2. Loop until all bytes are written to the disk
    while (bytes_written < total_bytes) {
        uint32_t bytes_to_write = total_bytes - bytes_written;

        // If the remaining data is larger than a sector, chunk it to 512 bytes
        if (bytes_to_write > 512) {
            bytes_to_write = 512;
        }

        // Prepare a temporary 512-byte sector buffer
        uint16_t sector_buffer[256] = {0};

        // Copy the current chunk of user data into our temporary sector buffer
        memcpy(sector_buffer, (const uint8_t*)user_data + bytes_written, bytes_to_write);

        //serial_printf("write_file_data: writing %x bytes to cluster %x\n\r", bytes_to_write, current_cluster);

        // Write the sector buffer to the current cluster on the disk
        write_cluster_data(current_cluster, sector_buffer);

        bytes_written += bytes_to_write;

        // 3. If we still have more data to write, move to the next cluster
        if (bytes_written < total_bytes) {
            uint32_t next = get_next_cluster(current_cluster);
            //serial_printf("write_file_data: need another cluster, next=%x\n\r", next);

            // If we hit the End of Cluster chain but still have data left, we ran out of space
            if (next == 0x0FFFFFFF) {
                //serial_printf("write_file_data: ran out of allocated clusters\n\r");
                return -2; // Error: No more allocated space left in this file
            }

            current_cluster = next;
        }
    }

    //serial_printf("write_file_data: wrote %x bytes total, updating directory entry size\n\r", total_bytes);

    // Persist the real size so subsequent reads know how much data to pull
    // back out of the cluster chain instead of always seeing 0.
    if (!update_file_size_entry(filename, total_bytes)) {
        //serial_printf("write_file_data: WARNING data was written but size update failed\n\r");
    }

    return 0; // Success!
}

int32_t read_file_by_name(const char* filename, void* output_buffer, uint32_t max_bytes) {
    uint16_t dir_buffer[256];

    //serial_printf("read_file_by_name: looking for %s\n\r", filename);

    // 1. Read the root directory sector
    ata_read_sectors_burst(ROOT_DIR_START_LBA, dir_buffer, 1);
    struct DirectoryEntry *entries = (struct DirectoryEntry *)dir_buffer;

    // 2. Scan for the file to get both its cluster chain and its exact size
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) break;
        if ((uint8_t)entries[i].name[0] == 0xE5) continue;

        if (fileNameStrcmp((char*)filename, entries[i].name)) {

            uint32_t start_cluster = entries[i].cluster_low | ((uint32_t)entries[i].cluster_high << 16);
            uint32_t size_to_read = entries[i].file_size;

            //serial_printf("read_file_by_name: found %s, start_cluster=%x, size=%x\n\r", filename, start_cluster, size_to_read);

            // Prevent buffer overflows! Don't read more than the buffer can hold
            if (size_to_read > max_bytes) {
                //serial_printf("read_file_by_name: clamping size %x down to max_bytes %x\n\r", size_to_read, max_bytes);
                size_to_read = max_bytes;
            }

            if (size_to_read == 0) {
                //serial_printf("read_file_by_name: file is empty, nothing to read\n\r");
                return 0;
            }

            // 3. Read only as much of the cluster chain as the file (and the
            // caller's buffer) actually allows — never the whole 512-byte
            // sector regardless of size, which previously overran small
            // destination buffers.
            read_fat_file_limited(start_cluster, (uint8_t*)output_buffer, size_to_read);

            return size_to_read; // Return exact size read so your OS knows where the data ends!
        }
    }

    //serial_printf("read_file_by_name: %s not found\n\r", filename);
    return -1; // Error: File not found
}

void ata_io_wait(void) {
    inPortB(0x80);
    inPortB(0x80);
}

void testDisk(bool ataGood) {
    if (ataGood) {
        serial_printf("Every service initialized successfully.\r\n");

        //serial_printf("==================Trying to create a file:=====================\r\n");
        uint32_t lba = create_file("HELP.TXT");
        if (lba == -1) {
            //serial_printf(">>>>>>>>>>>>>>>>>>>> Failed to create file! <<<<<<<<<<<<<<<<<<<<<<<<\r\n");

            goto afterFile;
        }
        //serial_printf("LBA: %x\r\n", lba);
        //serial_printf(">>>>>>>>>>> File created successfully.\r\n");

        //serial_printf("================Trying to write the file:================\r\n");
        const char* data = "This is a test file.";
        write_file_data("HELP.TXT", data, strlen(data));

        char* newData[strlen(data)] = {};

        //serial_printf("===============Trying to read the file:=================\r\n");
        read_file_by_name("HELP.TXT", newData, strlen(data));

        serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
        serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", newData);



        //serial_printf("==================Trying to create a file:=====================\r\n");
        uint32_t _lba = create_file("MARTIN.TXT");
        if (_lba == -1) {
            //serial_printf(">>>>>>>>>>>>>>>>>>>> Failed to create file! <<<<<<<<<<<<<<<<<<<<<<<<\r\n");

            goto afterFile;
        }
        //serial_printf("LBA: %x\r\n", lba);
        //serial_printf(">>>>>>>>>>> File created successfully.\r\n");

        //serial_printf("================Trying to write the file:================\r\n");
        const char* _data = "Martin, sache que si tu lis ce texte, c'est que tu es une personne très aimable qui regarde ce que ce tdc de denis t'envoie en masse, et c'est ce sur quoi il passe ses nuits au lieu de dormir. Enft, je fais durer ce texte pour vérifier que mon merdier peut écrire sur plusieurs secteurs en même telmps. C'est un test très important car sans ç comment suis-je sencé savoir si mon truc marche ou pas, ru vois?";
        write_file_data("MARTIN.TXT", _data, strlen(_data));

        char* _newData[strlen(_data)] = {};

        //serial_printf("===============Trying to read the file:=================\r\n");
        read_file_by_name("MARTIN.TXT", _newData, strlen(_data));

        serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
        serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", _newData);

    } else {
        serial_printf("ATA failed to initialize!\r\n");

        return;
    }

    afterFile:

    serial_printf("Trying to read the file:\r\n");

    const char* data = "This is a test file.";
    char* newData[strlen(data)] = {};

    //serial_printf("===============Trying to read the file:=================\r\n");
    read_file_by_name("HELP.TXT", newData, strlen(data));

    //serial_printf(">>>>>>>>>>>>>>>File read successfully.\r\n");
    //serial_printf(">>>>>>>>>>>>>>>Data: %s\r\n", newData);
}