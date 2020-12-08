/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene algunas funciones para acceder a la memoria SD                *
/*                                                                         *
/***************************************************************************/
#include "sd.h"
#include <string.h>
#include "sd_raw.h"
#include "util.h"
#include "lcd.h"

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
 while(fat_read_dir(dd, dir_entry))
	{
	 if(strcmp(dir_entry->long_name, name) == 0)
		{
		 fat_reset_dir(dd);
		 return 1;
		}
	}

 return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
 struct fat_dir_entry_struct file_entry;
 if(!find_file_in_dir(fs, dd, name, &file_entry))
	return 0;

 return fat_open_file(fs, &file_entry);
}

uint8_t print_disk_info(const struct fat_fs_struct* fs)
{
 if(!fs)
	return 0;

 struct sd_raw_info disk_info;
 if(!sd_raw_get_info(&disk_info))
	return 0;

 lcd_string_P(PSTR("manuf:  0x")); int2str(disk_info.manufacturer);
 lcd_string_P(PSTR("\nserial: 0x")); int2str(disk_info.serial);
 lcd_string_P(PSTR("\nsize:   ")); int2str(disk_info.capacity / 1024 / 1024); lcd_string_P(PSTR("MB\n"));
 return 1;
}

#if FAT_DATETIME_SUPPORT
void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    *year = 2007;
    *month = 1;
    *day = 1;
    *hour = 0;
    *min = 0;
    *sec = 0;
}
#endif
