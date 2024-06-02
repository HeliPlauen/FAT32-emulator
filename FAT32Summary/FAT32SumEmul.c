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
#define ATTR_ARCHIVE 0x20
#define ROOT_DIR_CLUSTER 2

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

typedef struct {
    uint32_t current_cluster;
    char path[256];
} directory_state;


/*

cd FAT32Summary
clang FAT32SumEmul.c

./a backup20_1.img

cd Debug
./FAT32Summary

./FAT32SumEmul backup20.img
./FAT32SumEmul backup20_TEST.img
./FAT32SumEmul backup20_TEST_2.img
./FAT32SumEmul fat32.img
./FAT32SumEmul fat32_20MB.img

./FAT32SumEmul 1.txt
./FAT32SumEmul 1_2.txt
./FAT32SumEmul 1.img

*/

void copy_file_system(const char* source, const char* destination);
void read_sector(FILE* file, uint32_t sector, void* buffer, uint16_t sector_size);

void update_fat(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t value);
void create_file_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name, uint32_t cluster, uint8_t attr);
void touch_file(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name);

uint32_t get_first_sector_of_cluster(fat32_boot_sector* boot_sector, uint32_t cluster);
uint32_t get_next_cluster(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster);
void ls_directory(FILE* file, fat32_boot_sector* boot_sector, directory_state* dir_state);
void cd_directory(FILE* file, fat32_boot_sector* boot_sector, directory_state* dir_state, const char* path);

int check_fat32_image(const char* image_path);
void write_boot_sector(FILE* file);
void write_fsinfo_sector(FILE* file);
void write_empty_sectors(FILE* file, int count);
void write_fat_tables(FILE* file, int fat_size);
int create_fat32_image(const char* filename);

void write_sector(FILE* file, uint32_t sector, const void* buffer, uint16_t sector_size);
uint32_t find_free_cluster(FILE* file, fat32_boot_sector* boot_sector);
void update_fat_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t value);
void create_directory_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t dir_cluster, const char* name, uint32_t first_cluster);
void initialize_directory_cluster(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t parent_cluster);
void create_directory(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name);

void read_cluster(FILE* file, uint32_t first_sector, void* buffer, uint16_t sector_size, uint8_t sectors_per_cluster);
void write_cluster(FILE* file, uint32_t first_sector, void* buffer, uint16_t sector_size, uint8_t sectors_per_cluster);

void initialize_boot_sector(fat32_boot_sector* bs);
void initialize_fs_info(fat32_fsinfo* fs_info);

int main(int argc, char* argv[]) {    
    int result = 0;

    char command[COMMAND_SIZE];
    char* cmd;
    char* arg;

    int isFat32 = 0;

    if (argc == 1) {
        const char* source = "\\\\.\\F:";
        //const char* source = "\\\\.\\D:";
        const char* filename = "fat32_CREATED.img";
        copy_file_system(source, filename);
    }
    else if (argc == 2) {
        if (access(argv[1], F_OK) != 0) {
            printf("Cannot open specefied file!");
            return 1;
        }

        if (!check_fat32_image(argv[1])) {
            isFat32 = 0;
        }
        else {
            isFat32 = 1;
        }
    }
    else {
        printf("Wrong number of arguments!\n");
        return 1;
    } 
    printf("File was formatted successfully or was already FILE32 image.\n");

    FILE* file = fopen(argv[1], "rb+");
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
    printf("Bytes per sector: %u\n", boot_sector.bytes_per_sector);
    printf("Sectors per cluster: %u\n", boot_sector.sectors_per_cluster);
    printf("Reserved sector count: %u\n", boot_sector.reserved_sector_count);
    printf("Number of FATs: %u\n", boot_sector.table_count);
    printf("Total sectors (32-bit): %u\n", boot_sector.total_sectors_32);
    printf("FAT size (32-bit): %u\n", boot_sector.table_size_32);
    printf("Root cluster: %u\n", boot_sector.root_cluster);

    // Initialize the directory state to the root directory
    directory_state dir_state = { .current_cluster = boot_sector.root_cluster };
    strcpy(dir_state.path, "/");

    while (1) {

        // Current path
        printf("%s > ", dir_state.path);

        if (fgets(command, COMMAND_SIZE, stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = 0;
        cmd = strtok(command, " ");
        arg = strtok(NULL, " ");

        if (cmd == NULL) {
            continue;
        }

        if (isFat32 == 0) {
            if (strcmp(cmd, "format") == 0) {
                create_fat32_image(argv[1]);
                isFat32 = 1;
            }
            else {
                printf("You have to format this fail in FAT32 first...\n");
            }
        }
        else {
            if (strcmp(cmd, "cd") == 0) {
                cd_directory(file, &boot_sector, &dir_state, arg);
            }
            else if (strcmp(cmd, "format") == 0) {
                create_fat32_image(argv[1]);
            }
            else if (strcmp(cmd, "ls") == 0) {
                ls_directory(file, &boot_sector, &dir_state);
            }
            else if (strcmp(cmd, "mkdir") == 0) {
                create_directory(file, &boot_sector, boot_sector.root_cluster, arg);
            }
            else if (strcmp(cmd, "touch") == 0) {
                touch_file(file, &boot_sector, boot_sector.root_cluster, arg);
            }
            else {
                printf("Unknown command: %s\n", cmd);
            }
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
    return 0;
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



/*
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
*/

// !!!!!!!
void initialize_boot_sector(fat32_boot_sector* bs) {
    memset(bs, 0, sizeof(fat32_boot_sector));
    memcpy(bs->jump_boot, "\xEB\x58\x90", 3);
    memcpy(bs->oem_name, "MSWIN4.1", 8);
    bs->bytes_per_sector = SECTOR_SIZE;
    bs->sectors_per_cluster = SECTORS_PER_CLUSTER;
    bs->reserved_sector_count = RESERVED_SECTORS;
    bs->table_count = FAT_COUNT;
    bs->total_sectors_32 = FILE_SIZE / SECTOR_SIZE;
    bs->table_size_32 = (FILE_SIZE / SECTOR_SIZE / SECTORS_PER_CLUSTER / FAT_COUNT) + 1;
    bs->media_type = 0xF8;
    bs->sectors_per_track = 63;
    bs->head_side_count = 255;
    bs->root_cluster = ROOT_DIR_CLUSTER;
    bs->fat_info = 1;
    bs->backup_boot_sector = 6;
    bs->drive_number = 0x80;
    bs->boot_signature = 0x29;
    bs->volume_id = 0x12345678;
    memcpy(bs->volume_label, "NO NAME    ", 11);
    memcpy(bs->fat_type_label, "FAT32   ", 8);
}

void initialize_fs_info(fat32_fsinfo* fs_info) {
    memset(fs_info, 0, sizeof(fat32_fsinfo));
    fs_info->lead_signature = 0x41615252;
    fs_info->structure_signature = 0x61417272;
    fs_info->free_count = 0xFFFFFFFF;
    fs_info->next_free = 0xFFFFFFFF;
    fs_info->trail_signature = 0xAA550000;
}

int create_fat32_image(const char* filename) {
    FILE* file = fopen(filename, "wb+");
    if (!file) {
        perror("Failed to create image file");
        return 0;
    }

    // Create and write boot sector
    fat32_boot_sector boot_sector;
    initialize_boot_sector(&boot_sector);
    if (fwrite(&boot_sector, sizeof(fat32_boot_sector), 1, file) != 1) {
        perror("Failed to write boot sector");
        fclose(file);
        return 0;
    }

    // Write reserved sectors
    uint8_t reserved[SECTOR_SIZE * (RESERVED_SECTORS - 1)] = { 0 };
    if (fwrite(reserved, sizeof(reserved), 1, file) != 1) {
        perror("Failed to write reserved sectors");
        fclose(file);
        return 0;
    }

    // Create and write FSInfo sector
    fat32_fsinfo fs_info;
    initialize_fs_info(&fs_info);
    if (fwrite(&fs_info, sizeof(fat32_fsinfo), 1, file) != 1) {
        perror("Failed to write FSInfo sector");
        fclose(file);
        return 0;
    }

    // Write backup boot sector and FSInfo sector
    if (fwrite(&boot_sector, sizeof(fat32_boot_sector), 1, file) != 1 ||
        fwrite(reserved, sizeof(reserved), 1, file) != 1 ||
        fwrite(&fs_info, sizeof(fat32_fsinfo), 1, file) != 1) {
        perror("Failed to write backup boot and FSInfo sectors");
        fclose(file);
        return 0;
    }

    // Calculate FAT size in sectors
    uint32_t fat_size = boot_sector.table_size_32;

    // Write FAT tables
    //uint8_t fat[SECTOR_SIZE * fat_size];
    uint8_t* fat = malloc(SECTOR_SIZE * fat_size * sizeof(uint8_t));
    if (fat == NULL) {
        printf("fat in create_fat32_image aborted!\n");
        exit(1);
    }

    memset(fat, 0, sizeof(fat));
    fat[0] = 0xF8; fat[1] = 0xFF; fat[2] = 0xFF; fat[3] = 0xFF; // FAT signature
    if (fwrite(fat, sizeof(fat), 1, file) != 1 || fwrite(fat, sizeof(fat), 1, file) != 1) {
        perror("Failed to write FAT tables");
        fclose(file);
        return 0;
    }

    // Write clusters for root directory
    uint8_t root_dir[SECTOR_SIZE * SECTORS_PER_CLUSTER] = { 0 };
    if (fwrite(root_dir, sizeof(root_dir), 1, file) != 1) {
        perror("Failed to write root directory cluster");
        fclose(file);
        return 0;
    }

    // Write padding until the end of the file
    size_t remaining_size = FILE_SIZE - ftell(file);
    uint8_t* padding = calloc(remaining_size, 1);
    if (!padding) {
        perror("Failed to allocate memory for padding");
        fclose(file);
        return 0;
    }

    if (fwrite(padding, remaining_size, 1, file) != 1) {
        perror("Failed to write padding");
        free(padding);
        fclose(file);
        return 0;
    }

    free(fat);
    free(padding);
    fclose(file);
    return 1;
}
// !!!!!!!!



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


uint32_t get_first_sector_of_cluster(fat32_boot_sector* boot_sector, uint32_t cluster) {
    uint32_t first_data_sector = boot_sector->reserved_sector_count + (boot_sector->table_count * boot_sector->table_size_32);
    return first_data_sector + ((cluster - 2) * boot_sector->sectors_per_cluster);
}


uint32_t get_next_cluster(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster) {
    // Check if the values used in division are valid
    if (boot_sector->bytes_per_sector == 0) {
        fprintf(stderr, "Error: bytes_per_sector is zero.\n");
        exit(EXIT_FAILURE);
    }

    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = boot_sector->reserved_sector_count + (fat_offset / boot_sector->bytes_per_sector);
    uint32_t ent_offset = fat_offset % boot_sector->bytes_per_sector;
    uint8_t buffer[SECTOR_SIZE];

    printf("fat_offset: %u, fat_sector: %u, ent_offset: %u\n", fat_offset, fat_sector, ent_offset);

    read_sector(file, fat_sector, buffer, boot_sector->bytes_per_sector);

    return *(uint32_t*)&buffer[ent_offset] & 0x0FFFFFFF;  // Mask to get the cluster number
}


void ls_directory(FILE* file, fat32_boot_sector* boot_sector, directory_state* dir_state) {
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint32_t cluster = dir_state->current_cluster;
    uint8_t buffer[SECTOR_SIZE];

    //printf("Listing directory: %s\n", dir_state->path);

    do {
        uint32_t first_sector = get_first_sector_of_cluster(boot_sector, cluster);

        for (uint32_t i = 0; i < boot_sector->sectors_per_cluster; i++) {
            read_sector(file, first_sector + i, buffer, sector_size);

            for (uint32_t j = 0; j < sector_size / sizeof(fat32_dir_entry); j++) {
                fat32_dir_entry* dir_entry = (fat32_dir_entry*)(buffer + j * sizeof(fat32_dir_entry));
                if (dir_entry->name[0] == 0x00) {
                    return;  // No more entries
                }
                if (dir_entry->name[0] == 0xE5 || (dir_entry->attr & ATTR_HIDDEN) || dir_entry->attr == ATTR_VOLUME_ID) {
                    continue;  // Entry is free, hidden, or volume ID
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

        cluster = get_next_cluster(file, boot_sector, cluster);
    } while (cluster < 0x0FFFFFF8);  // End of cluster chain
}


void cd_directory(FILE* file, fat32_boot_sector* boot_sector, directory_state* dir_state, const char* path) {
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint32_t cluster = dir_state->current_cluster;
    uint8_t buffer[SECTOR_SIZE];
    uint32_t new_cluster = 0;

    if (strcmp(path, ".") == 0) {
        return;  // No change for current directory
    }

    if (strcmp(path, "..") == 0) {
        // For simplicity, assume root is the only directory above
        if (strcmp(dir_state->path, "/") != 0) {
            dir_state->current_cluster = boot_sector->root_cluster;
            strcpy(dir_state->path, "/");
        }
        return;
    }

    do {
        uint32_t first_sector = get_first_sector_of_cluster(boot_sector, cluster);

        for (uint32_t i = 0; i < boot_sector->sectors_per_cluster; i++) {
            read_sector(file, first_sector + i, buffer, sector_size);

            for (uint32_t j = 0; j < sector_size / sizeof(fat32_dir_entry); j++) {
                fat32_dir_entry* dir_entry = (fat32_dir_entry*)(buffer + j * sizeof(fat32_dir_entry));
                if (dir_entry->name[0] == 0x00) {
                    return;  // No more entries
                }
                if (dir_entry->name[0] == 0xE5 || (dir_entry->attr & ATTR_HIDDEN) || dir_entry->attr == ATTR_VOLUME_ID) {
                    continue;  // Entry is free, hidden, or volume ID
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

                // Check if this is the directory we want to change to
                if ((dir_entry->attr & ATTR_DIRECTORY) && strcmp(name, path) == 0) {
                    new_cluster = (dir_entry->fst_clus_hi << 16) | dir_entry->fst_clus_lo;
                    dir_state->current_cluster = new_cluster;
                    if (strcmp(dir_state->path, "/") == 0) {
                        snprintf(dir_state->path, sizeof(dir_state->path), "/%s", path);
                    }
                    else {
                        snprintf(dir_state->path + strlen(dir_state->path), sizeof(dir_state->path) - strlen(dir_state->path), "/%s", path);
                    }
                    return;
                }
            }
        }

        cluster = get_next_cluster(file, boot_sector, cluster);
    } while (cluster < 0x0FFFFFF8);  // End of cluster chain

    printf("Directory not found: %s\n", path);
}


void write_sector(FILE* file, uint32_t sector, const void* buffer, uint16_t sector_size) {
    if (fseek(file, (long)sector * sector_size, SEEK_SET) != 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }
    if (fwrite(buffer, sector_size, 1, file) != 1) {
        perror("fwrite failed");
        exit(EXIT_FAILURE);
    }
}


uint32_t find_free_cluster(FILE* file, fat32_boot_sector* boot_sector) {
    uint32_t fat_sector = boot_sector->reserved_sector_count;
    uint32_t fat_size = boot_sector->table_size_32;
    uint8_t buffer[SECTOR_SIZE];
    for (uint32_t sector = 0; sector < fat_size; sector++) {
        read_sector(file, fat_sector + sector, buffer, SECTOR_SIZE);
        for (uint32_t i = 0; i < SECTOR_SIZE; i += FAT_ENTRY_SIZE) {
            uint32_t entry;
            memcpy(&entry, &buffer[i], FAT_ENTRY_SIZE);
            if (entry == 0) {
                // Found a free cluster
                return (sector * (SECTOR_SIZE / FAT_ENTRY_SIZE)) + (i / FAT_ENTRY_SIZE);
            }
        }
    }
    return 0xFFFFFFFF; // No free cluster found
}


void update_fat_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t value) {
    uint32_t fat_sector = boot_sector->reserved_sector_count;
    uint32_t fat_size = boot_sector->table_size_32;
    uint32_t fat_offset = cluster * FAT_ENTRY_SIZE;
    uint32_t sector = fat_sector + (fat_offset / SECTOR_SIZE);
    uint32_t offset = fat_offset % SECTOR_SIZE;
    uint8_t buffer[SECTOR_SIZE];

    read_sector(file, sector, buffer, SECTOR_SIZE);
    memcpy(&buffer[offset], &value, FAT_ENTRY_SIZE);
    write_sector(file, sector, buffer, SECTOR_SIZE);
}


void create_directory_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t dir_cluster, const char* name, uint32_t first_cluster) {
    uint32_t first_data_sector = boot_sector->reserved_sector_count + (boot_sector->table_count * boot_sector->table_size_32);
    uint32_t sector = first_data_sector + ((dir_cluster - 2) * boot_sector->sectors_per_cluster);
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint8_t buffer[SECTOR_SIZE];
    read_sector(file, sector, buffer, sector_size);

    for (uint32_t i = 0; i < sector_size; i += sizeof(fat32_dir_entry)) {
        fat32_dir_entry* dir_entry = (fat32_dir_entry*)(buffer + i);
        if (dir_entry->name[0] == 0x00 || dir_entry->name[0] == 0xE5) {
            // Found an empty entry
            memset(dir_entry, 0, sizeof(fat32_dir_entry));
            memset(dir_entry->name, ' ', 11); // Fill with spaces
            memcpy(dir_entry->name, name, strlen(name));
            dir_entry->attr = ATTR_DIRECTORY;
            dir_entry->fst_clus_hi = (first_cluster >> 16) & 0xFFFF;
            dir_entry->fst_clus_lo = first_cluster & 0xFFFF;
            write_sector(file, sector, buffer, sector_size);
            return;
        }
    }
}


void initialize_directory_cluster(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t parent_cluster) {
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint32_t first_data_sector = boot_sector->reserved_sector_count + (boot_sector->table_count * boot_sector->table_size_32);
    uint32_t sector = first_data_sector + ((cluster - 2) * boot_sector->sectors_per_cluster);
    uint8_t buffer[SECTOR_SIZE];
    memset(buffer, 0, sector_size);

    // Create "." entry
    fat32_dir_entry* dot_entry = (fat32_dir_entry*)buffer;
    memset(dot_entry, ' ', 11);
    dot_entry->name[0] = '.';
    dot_entry->attr = ATTR_DIRECTORY;
    dot_entry->fst_clus_hi = (cluster >> 16) & 0xFFFF;
    dot_entry->fst_clus_lo = cluster & 0xFFFF;

    // Create ".." entry
    fat32_dir_entry* dotdot_entry = (fat32_dir_entry*)(buffer + sizeof(fat32_dir_entry));
    memset(dotdot_entry, ' ', 11);
    dotdot_entry->name[0] = '.';
    dotdot_entry->name[1] = '.';
    dotdot_entry->attr = ATTR_DIRECTORY;
    dotdot_entry->fst_clus_hi = (parent_cluster >> 16) & 0xFFFF;
    dotdot_entry->fst_clus_lo = parent_cluster & 0xFFFF;

    write_sector(file, sector, buffer, sector_size);
}


void create_directory(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name) {
    printf("%s\n", name);

    // Find a free cluster
    uint32_t new_cluster = find_free_cluster(file, boot_sector);
    if (new_cluster == 0xFFFFFFFF) {
        printf("No free cluster found\n");
        return;
    }

    // Mark the cluster as used
    update_fat_entry(file, boot_sector, new_cluster, 0x0FFFFFFF);

    // Create directory entry in the parent directory
    create_directory_entry(file, boot_sector, parent_cluster, name, new_cluster);

    // Initialize the directory cluster
    initialize_directory_cluster(file, boot_sector, new_cluster, parent_cluster);
}


/*
void write_sector(FILE* file, uint32_t sector, void* buffer, uint16_t sector_size) {
    if (fseek(file, (long)sector * sector_size, SEEK_SET) != 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }
    if (fwrite(buffer, sector_size, 1, file) != 1) {
        perror("fwrite failed");
        exit(EXIT_FAILURE);
    }
}
*/


void read_cluster(FILE* file, uint32_t first_sector, void* buffer, uint16_t sector_size, uint8_t sectors_per_cluster) {
    for (uint8_t i = 0; i < sectors_per_cluster; i++) {
        read_sector(file, first_sector + i, (uint8_t*)buffer + (i * sector_size), sector_size);
    }
}


void write_cluster(FILE* file, uint32_t first_sector, void* buffer, uint16_t sector_size, uint8_t sectors_per_cluster) {
    for (uint8_t i = 0; i < sectors_per_cluster; i++) {
        write_sector(file, first_sector + i, (uint8_t*)buffer + (i * sector_size), sector_size);
    }
}


/*
uint32_t find_free_cluster(FILE* file, fat32_boot_sector* boot_sector) {
    uint32_t fat_start = boot_sector->reserved_sector_count;
    uint32_t fat_size = boot_sector->table_size_32;
    uint32_t total_sectors = boot_sector->total_sectors_32;
    uint32_t num_clusters = total_sectors / boot_sector->sectors_per_cluster;
    uint32_t* fat = malloc(num_clusters * sizeof(uint32_t));
    if (fat == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Reading the entire FAT
    for (uint32_t i = 0; i < fat_size; i++) {
        read_sector(file, fat_start + i, ((uint8_t*)fat) + i * SECTOR_SIZE, SECTOR_SIZE);
    }

    for (uint32_t i = 2; i < num_clusters; i++) {
        if (fat[i] == 0x00000000) { // Free cluster
            free(fat);
            return i;
        }
    }

    free(fat);
    return 0; // No free cluster found
}
*/

void update_fat(FILE* file, fat32_boot_sector* boot_sector, uint32_t cluster, uint32_t value) {
    uint32_t fat_start = boot_sector->reserved_sector_count;
    uint32_t fat_size = boot_sector->table_size_32;
    uint32_t total_sectors = boot_sector->total_sectors_32;
    uint32_t num_clusters = total_sectors / boot_sector->sectors_per_cluster;
    uint32_t* fat = malloc(num_clusters * sizeof(uint32_t));
    if (fat == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Reading the entire FAT
    for (uint32_t i = 0; i < fat_size; i++) {
        read_sector(file, fat_start + i, ((uint8_t*)fat) + i * SECTOR_SIZE, SECTOR_SIZE);
    }

    fat[cluster] = value;

    // Writing the entire FAT back to the disk
    for (uint32_t i = 0; i < fat_size; i++) {
        write_sector(file, fat_start + i, ((uint8_t*)fat) + i * SECTOR_SIZE, SECTOR_SIZE);
    }

    free(fat);
}


void create_file_entry(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name, uint32_t cluster, uint8_t attr) {
    uint32_t first_data_sector = boot_sector->reserved_sector_count + (boot_sector->table_count * boot_sector->table_size_32);
    uint32_t sector_size = boot_sector->bytes_per_sector;
    uint32_t cluster_size = boot_sector->sectors_per_cluster * sector_size;
    uint32_t sector = first_data_sector + (parent_cluster - 2) * boot_sector->sectors_per_cluster;

    // Dynamically allocate the buffer for the cluster size
    uint8_t* buffer = malloc(cluster_size);
    if (buffer == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Read the entire cluster containing the parent directory
    read_cluster(file, sector, buffer, sector_size, boot_sector->sectors_per_cluster);

    // Find a free entry in the directory
    for (uint32_t i = 0; i < cluster_size / sizeof(fat32_dir_entry); i++) {
        fat32_dir_entry* dir_entry = (fat32_dir_entry*)(buffer + i * sizeof(fat32_dir_entry));
        if (dir_entry->name[0] == 0x00 || dir_entry->name[0] == 0xE5) {
            memset(dir_entry, 0, sizeof(fat32_dir_entry));
            strncpy((char*)dir_entry->name, name, 11);
            dir_entry->attr = attr;
            dir_entry->fst_clus_lo = cluster & 0xFFFF;
            dir_entry->fst_clus_hi = (cluster >> 16) & 0xFFFF;
            write_cluster(file, sector, buffer, sector_size, boot_sector->sectors_per_cluster);
            free(buffer);
            return;
        }
    }

    free(buffer);
}


void touch_file(FILE* file, fat32_boot_sector* boot_sector, uint32_t parent_cluster, const char* name) {
    uint32_t free_cluster = find_free_cluster(file, boot_sector);
    if (free_cluster == 0) {
        printf("No free clusters available.\n");
        return;
    }

    // Mark the new cluster as the end of a chain in the FAT
    update_fat(file, boot_sector, free_cluster, 0x0FFFFFFF);

    // Create a file entry in the parent directory
    create_file_entry(file, boot_sector, parent_cluster, name, free_cluster, ATTR_ARCHIVE);

    printf("File '%s' created with cluster %u.\n", name, free_cluster);
}



