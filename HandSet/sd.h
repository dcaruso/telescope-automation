#include "config.h"
#include "fat.h"

// Prototipos de funciones
uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name); 
uint8_t print_disk_info(const struct fat_fs_struct* fs);
