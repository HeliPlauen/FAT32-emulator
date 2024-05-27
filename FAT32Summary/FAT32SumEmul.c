#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

#define COMMAND_SIZE 256
#define BUFFER_SIZE 4096
#define FILE_SIZE (20 * 1024 * 1024) // 20 MB
#define SECTOR_SIZE 512
#define RESERVED_SECTORS 32
#define FAT_COUNT 2
#define SECTORS_PER_CLUSTER 8
#define ROOT_CLUSTER 2
#define FAT_ENTRY_SIZE 4
#define ATTR_DIRECTORY 0x10
#define ATTR_VOLUME_ID 0x08
#define ATTR_HIDDEN 0x02  // Define the hidden attribute

#pragma pack(push, 1)
typedef struct {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;
    uint32_t table_size_32;
    uint16_t extended_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved_0[12];
    uint8_t  drive_number;
    uint8_t  reserved_1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fat_type_label[8];
    //} __attribute__((packed)) fat32_boot_sector;
} fat32_boot_sector;

typedef struct {
    uint32_t lead_signature;
    uint8_t  reserved_0[480];
    uint32_t structure_signature;
    uint32_t free_count;
    uint32_t next_free;
    uint8_t  reserved_1[12];
    uint32_t trail_signature;
    //} __attribute__((packed)) fat32_fsinfo;
} fat32_fsinfo;

typedef struct {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
    //} __attribute__((packed)) fat32_dir_entry;
} fat32_dir_entry;
#pragma pack(pop)


/*

cd FAT32Summary
clang FAT32SumEmul.c

cd Debug
./FAT32Summary

./FAT32SumEmul backup20.img
./FAT32SumEmul backup20_TEST.img
./FAT32SumEmul backup20_TEST_2.img
./FAT32SumEmul fat32.img
./FAT32SumEmul fat32_20MB.img

./FAT32SumEmul 1.txt
./FAT32SumEmul 1.img

*/

void copy_file_system(const char* source, const char* destination);
void read_sector(FILE* file, uint32_t sector, void* buffer, uint16_t sector_size);
void ls_root_directory(FILE* file, fat32_boot_sector* boot_sector);
int check_fat32_image(const char* image_path);
void write_boot_sector(FILE* file);
void write_fsinfo_sector(FILE* file);
void write_empty_sectors(FILE* file, int count);
void write_fat_tables(FILE* file, int fat_size);
int create_fat32_image(const char* filename);

int main(int argc, char* argv[]) {    
    int result = 0;

    char command[COMMAND_SIZE];
    char* cmd;
    char* arg;

    if (argc == 1) {
        const char* source = "\\\\.\\F:";
        //const char* source = "\\\\.\\D:";
        const char* filename = "fat32_CREATED.img";
        copy_file_system(source, filename);

        /*
        result = create_fat32_image(filename);
        if (!result) {
            fprintf(stderr, "Failed to create FAT32 image in NEW file\n");
            return 1;
        }
        */
    }
    else if (argc == 2) {
        if (access(argv[1], F_OK) != 0) {
            printf("Cannot open specefied file!");
            return 1;
        }

        if (!check_fat32_image(argv[1])) {
            result = create_fat32_image(argv[1]);
            if (!result) {
                fprintf(stderr, "Failed to create FAT32 image in EXISTING file\n");
                return 1;
            }
        }
    }
    else {
        printf("Wrong number of arguments!\n");
        return 1;
    } 
    printf("File was formatted successfully or was already FILE32 image.\n");

    while (1) {

        // Current path
        //char buff[MAX_PATH];
        //GetCurrentDirectory(MAX_PATH, buff);
        char buff[] = "TEST";

        printf("/ %s > ", buff);
        if (fgets(command, COMMAND_SIZE, stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = 0;
        cmd = strtok(command, " ");
        arg = strtok(NULL, " ");

        if (cmd == NULL) continue;

        if (strcmp(cmd, "cd") == 0) {
            //cmd_cd(fs, arg);
        }
        else if (strcmp(cmd, "format") == 0) {
            // Implement format command
        }
        else if (strcmp(cmd, "ls") == 0) {
            FILE* file = fopen(argv[1], "rb");
            if (!file) {
                perror("Failed to open image file");
                return EXIT_FAILURE;
            }

            // Read the boot sector
            fat32_boot_sector boot_sector;
            if (fread(&boot_sector, sizeof(fat32_boot_sector), 1, file) != 1) {
                perror("Failed to read boot sector");
                fclose(file);
                return EXIT_FAILURE;
            }

            // Print boot sector information for debugging
            //printf("Bytes per sector: %u\n", boot_sector.bytes_per_sector);
            //printf("Sectors per cluster: %u\n", boot_sector.sectors_per_cluster);
            //printf("Reserved sector count: %u\n", boot_sector.reserved_sector_count);
            //printf("Number of FATs: %u\n", boot_sector.table_count);
            //printf("Total sectors (32-bit): %u\n", boot_sector.total_sectors_32);
            //printf("FAT size (32-bit): %u\n", boot_sector.table_size_32);
            //printf("Root cluster: %u\n", boot_sector.root_cluster);

            // List the root directory
            printf("Root Directory:\n");
            ls_root_directory(file, &boot_sector);

            fclose(file);
        }
        else if (strcmp(cmd, "mkdir") == 0) {
            // Implement mkdir command
        }
        else if (strcmp(cmd, "touch") == 0) {
            // Implement touch command
        }
        else {
            printf("Unknown command: %s\n", cmd);
        }
    }

    return 0;
}


void copy_file_system(const char* source, const char* destination) {

    // Open the source file system (drive or image)
    HANDLE hSource = CreateFileA(
        source, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hSource == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening source file: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    // Open the destination file
    HANDLE hDestination = CreateFileA(
        destination, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );

    if (hDestination == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening destination file: %lu\n", GetLastError());
        CloseHandle(hSource);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    DWORD totalBytesCopied = 0;

    // Read from source and write to destination, up to MAX_COPY_SIZE
    while (totalBytesCopied < FILE_SIZE && ReadFile(hSource, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        DWORD bytesToWrite = bytesRead;
        if (totalBytesCopied + bytesRead > FILE_SIZE) {
            bytesToWrite = FILE_SIZE - totalBytesCopied;
        }

        if (!WriteFile(hDestination, buffer, bytesToWrite, &bytesWritten, NULL) || bytesWritten != bytesToWrite) {
            fprintf(stderr, "Error writing to destination file: %lu\n", GetLastError());
            CloseHandle(hSource);
            CloseHandle(hDestination);
            exit(EXIT_FAILURE);
        }

        totalBytesCopied += bytesWritten;
    }

    // Close handles
    CloseHandle(hSource);
    CloseHandle(hDestination);

    printf("File system copy completed successfully. Total bytes copied: %lu\n", totalBytesCopied);
}


int check_fat32_image(const char* image_path) {    
    HANDLE file_handle;
    BOOL result;
    DWORD bytes_returned;
    BYTE buffer[512];

    file_handle = CreateFileA(image_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open file: %s\n", image_path);
        return 0;
    }

    // Read the first sector (boot sector)
    result = ReadFile(file_handle, buffer, sizeof(buffer), &bytes_returned, NULL);
    if (!result || bytes_returned != sizeof(buffer)) {
        fprintf(stderr, "Failed to read boot sector: %s\n", image_path);
        CloseHandle(file_handle);
        return 0;
    }

    // Check the file system type in the boot sector
    if (memcmp(buffer + 0x52, "FAT32", 5) == 0) {
        CloseHandle(file_handle);
        return 1;  // This is a FAT32 file system
    }

    CloseHandle(file_handle);
    
    /*
    unsigned char buffer[512];
    FILE* file;

    file = fopen(image_path, "r");
    fread(buffer, 1, 512, file);

    // Check the file system type in the boot sector
    if (memcmp(buffer + 0x52, "FAT32", 5) == 0) {
        fclose(file);
        printf("This is a FILE32 image.\n");
        return 1;  // This is a FAT32 file system
    }
    else {
        printf("This is NOT a FILE32 image.\n");
        fclose(file);
        return 0;  // Not a FAT32 file system
    }
    */
    return 1;
}


void write_boot_sector(FILE* file) {
    fat32_boot_sector boot_sector = { 0 };

    memcpy(boot_sector.jump_boot, "\xEB\x58\x90", 3);
    memcpy(boot_sector.oem_name, "MSWIN4.1", 8);
    boot_sector.bytes_per_sector = SECTOR_SIZE;
    boot_sector.sectors_per_cluster = SECTORS_PER_CLUSTER;
    boot_sector.reserved_sector_count = RESERVED_SECTORS;
    boot_sector.table_count = FAT_COUNT;
    boot_sector.media_type = 0xF8;
    boot_sector.total_sectors_32 = FILE_SIZE / SECTOR_SIZE;
    boot_sector.table_size_32 = (FILE_SIZE / SECTOR_SIZE) / (SECTORS_PER_CLUSTER * FAT_COUNT);
    boot_sector.root_cluster = ROOT_CLUSTER;
    boot_sector.drive_number = 0x80;
    boot_sector.boot_signature = 0x29;
    boot_sector.volume_id = (uint32_t)time(NULL);
    memcpy(boot_sector.volume_label, "NO NAME    ", 11);
    memcpy(boot_sector.fat_type_label, "FAT32   ", 8);

    fwrite(&boot_sector, sizeof(fat32_boot_sector), 1, file);
    fseek(file, SECTOR_SIZE - sizeof(fat32_boot_sector), SEEK_CUR); // Padding to sector size
}


void write_fsinfo_sector(FILE* file) {
    fat32_fsinfo fsinfo = { 0 };

    fsinfo.lead_signature = 0x41615252;
    fsinfo.structure_signature = 0x61417272;
    fsinfo.free_count = 0xFFFFFFFF;
    fsinfo.next_free = 0xFFFFFFFF;
    fsinfo.trail_signature = 0xAA550000;

    fwrite(&fsinfo, sizeof(fat32_fsinfo), 1, file);
    fseek(file, SECTOR_SIZE - sizeof(fat32_fsinfo), SEEK_CUR); // Padding to sector size
}


void write_empty_sectors(FILE* file, int count) {
    char empty[SECTOR_SIZE] = { 0 };
    for (int i = 0; i < count; i++) {
        fwrite(empty, SECTOR_SIZE, 1, file);
    }
}


void write_fat_tables(FILE* file, int fat_size) {

    // Write two FAT tables
    for (int i = 0; i < FAT_COUNT; i++) {

        // FAT table starts with media type
        uint8_t fat_start[4] = { 0xF8, 0xFF, 0xFF, 0xFF };
        fwrite(fat_start, 4, 1, file);

        // The rest of the FAT table is empty
        write_empty_sectors(file, fat_size - 1);
    }
}


int create_fat32_image(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to create image file");
        return 0;
    }

    // Write boot sector
    write_boot_sector(file);

    // Write FSInfo sector
    write_fsinfo_sector(file);

    // Write empty sectors until the FAT tables
    write_empty_sectors(file, RESERVED_SECTORS - 2); // -2 for boot sector and FSInfo

    // Calculate the size of each FAT table
    int fat_size = (FILE_SIZE / SECTOR_SIZE) / (SECTORS_PER_CLUSTER * FAT_COUNT);
    write_fat_tables(file, fat_size);

    // Write empty clusters until the root directory
    write_empty_sectors(file, (SECTORS_PER_CLUSTER * (ROOT_CLUSTER - 2)) - (RESERVED_SECTORS + (fat_size * FAT_COUNT)));

    // Close the file
    fclose(file);

    printf("The FAT32 formatting finished successfully!\n");
    return 1;
}


void read_sector(FILE* file, uint32_t sector, void* buffer, uint16_t sector_size) {
    if (fseek(file, (long)sector * sector_size, SEEK_SET) != 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }
    if (fread(buffer, sector_size, 1, file) != 1) {
        perror("fread failed");
        exit(EXIT_FAILURE);
    }
    //printf("Read sector %u\n", sector);
}


void ls_root_directory(FILE* file, fat32_boot_sector* boot_sector) {
    uint32_t first_data_sector = boot_sector->reserved_sector_count + (boot_sector->table_count * boot_sector->table_size_32);
    uint32_t root_dir_sector = first_data_sector + ((boot_sector->root_cluster - 2) * boot_sector->sectors_per_cluster);
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint8_t buffer[SECTOR_SIZE];

    printf("First data sector: %u\n", first_data_sector);
    printf("Root directory sector: %u\n", root_dir_sector);

    for (uint32_t i = 0; i < boot_sector->sectors_per_cluster; i++) {
        read_sector(file, root_dir_sector + i, buffer, sector_size);
        for (uint32_t j = 0; j < sector_size / sizeof(fat32_dir_entry); j++) {
            fat32_dir_entry* dir_entry = (fat32_dir_entry*)(buffer + j * sizeof(fat32_dir_entry));
            if (dir_entry->name[0] == 0x00) {
                // No more entries
                return;
            }
            if ((dir_entry->name[0] == 0xE5) || (dir_entry->attr & ATTR_HIDDEN)) {
                // Entry is free
                continue;
            }
            if (dir_entry->attr == ATTR_VOLUME_ID) {
                // Volume ID
                continue;
            }

            // Print the file/directory name
            char name[12];
            memset(name, 0, sizeof(name));
            memcpy(name, dir_entry->name, 11);
            // Remove trailing spaces
            for (int k = 10; k >= 0; k--) {
                if (name[k] == ' ') {
                    name[k] = '\0';
                }
                else {
                    break;
                }
            }
            printf("%s\n", name);
        }
    }
}
