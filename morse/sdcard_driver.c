
#include "sdcard_driver.h"

#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


//#################################### SD Card
#include "sdcard/fat.h"
#include "sdcard/fat_config.h"
#include "sdcard/partition.h"
#include "sdcard/sd_raw.h"
#include "sdcard/sd_raw_config.h"

#define DEBUG 0

struct partition_struct* partition;
struct fat_fs_struct* fs;
struct fat_dir_struct* dd;
struct fat_dir_entry_struct directory;
struct fat_file_struct* fd;
struct fat_dir_entry_struct dir_entry;
struct fat_dir_entry_struct file_entry;

uint8_t	file_open = 0,
		card_open = 0;


//##########################################################################################
uint16_t file_getAnzLine(void)	// anzahl Zeilen aktuelles file
{
	uint16_t counterLines=0;
	if(fd)
	{
		uint8_t buffer[30];
		intptr_t count;
		while((count = fat_read_file(fd, buffer, sizeof(buffer))) > 0)
		{
			for(intptr_t i = 0; i < count; ++i)
			{
			    if(buffer[i] == '\r')	{ counterLines++;}
			}
		}
	}
	return counterLines+1;
}
//##########################################################################################
uint8_t file_getLine(uint16_t lineNr,char* zielbuffer)// anzahl Zeilen aktuelles file
{
	if(fd)
	{
		if (lineNr < 1 ) {return 0;}
		uint8_t     bufferlen			= 30;// maximale Zeilenlänge von 30 Zeichen
		uint8_t 	arbeitsbuffer[bufferlen];
		uint16_t 	counterLines		= 0;
		int32_t 	pos_after_last_enter= 0;
		uint8_t		count;

		//########## Anfang der n-ten Zeile suchen und position in "pos_after_last_enter" zurückgeben
		//########## es werden alle '\r' gezählt bis die Anfangsposition der gesuchten Zeile gefunden ist
		
		
		while(counterLines < lineNr-1)
			{
			if(!fat_seek_file(fd, &pos_after_last_enter, FAT_SEEK_SET)) {/* error*/}
			
			count = fat_read_file(fd, arbeitsbuffer, bufferlen);
			for(intptr_t i = 0; i < count; ++i)
				{
				if(arbeitsbuffer[i] == '\r')	
					{
					counterLines++;
					pos_after_last_enter+=i+1;// Zeiger auf Zeichen nach dem letzten Enter
					break;
					}
				}
			}
	
		//########## Daten auslesen und in Zielbuffer schreiben		
		if(!fat_seek_file(fd, &pos_after_last_enter, FAT_SEEK_SET)) {/* error*/}
		count = fat_read_file(fd, arbeitsbuffer, bufferlen);
		
		for(intptr_t i=0; i<count; ++i)
			{
			zielbuffer[i] = arbeitsbuffer[i];			
			if(arbeitsbuffer[i] == '\r')	
				{
				zielbuffer[i+1] = 0;
				return i; // return Zeilenlänge
				}
			}
	}
	return 0;
}
//##########################################################################################
uint32_t strtolong(const char* str)
{
    uint32_t l = 0;
    while(*str >= '0' && *str <= '9')
        l = l * 10 + (*str++ - '0');

    return l;
}
//##########################################################################################
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
//##########################################################################################
struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry))
        return 0;

    return fat_open_file(fs, &file_entry);
}
//##########################################################################################
uint8_t print_disk_info(const struct fat_fs_struct* fs)
{

  
	
	return 1;	
}

//##########################################################################################
void sd_get_Directory(void)
{	
	
	
}	
//########################################################################################## 
void sd_card_open(void) //Logging File auf SD-Karte oeffnen und auf das Ende positionieren. 
{
	if (card_open == 1) return;
	
    //############# setup sd card slot 
	if(!sd_raw_init())	
	{
		printf("#MMC/SD initialization failed\r");
	}
	else
	{   
	card_open=1;
//############# open first partition 
	partition = partition_open(	sd_raw_read,sd_raw_read_interval,sd_raw_write,sd_raw_write_interval,0);
	
	// If the partition did not open, assume the storage device is a "superfloppy", i.e. has no MBR.
	if(!partition)
		{   
		partition = partition_open(sd_raw_read,sd_raw_read_interval,sd_raw_write,sd_raw_write_interval,-1);
		if(!partition)	
			{
#if DEBUG			
			printf("#opening partition failed\r");
#endif			
			}
		}

	//############# open file system 
	fs = fat_open(partition);
	if(!fs)  	
		{
#if DEBUG		
		usart_puts(&USARTD0_data,"#opening filesystem failed\r");
#endif		
		}

	//############# open root directory
	fat_get_dir_entry_of_path(fs, "/", &directory);

	dd = fat_open_dir(fs, &directory);
	if(!dd)		
		{
#if DEBUG	
		usart_puts(&USARTD0_data,"#opening root directory failed\r");
#endif	
		}
	
#if DEBUG
	print_disk_info(fs);	//print some card information as a boot message 
	sd_get_Directory();		//print directory entry
#endif	
	
//############
	printf("#Card open\r"); 		
	}
}
//##################################################################################################
void sd_card_close(void)
{
	if(card_open == 1 && card_open == 0)			// Zur Sicherheit noch mal einen sync, damit ist das FS konsistent
		{
		fat_close_dir(dd);// close directory 
		printf("#close: directory\r");
		fat_close(fs);// close file system 
	
		partition_close(partition);// close partition 
		//usart_puts(&USARTD0_data,"#close: partition\r");
		card_open = 0;		
		}
}  
//##################################################################################################
void sd_file_open(const char* filename)
{
	if(file_open == 1) 
		{

		return;
		}
	
	fd = open_file_in_dir(fs, dd, filename);
	if(!fd)
	{
	
	}
	else
	{
		file_open = 1;
		
	}

} 
//*read*
void sd_file_read(char * data)
{
	/*uint8_t buffer[200];
	intptr_t count;
	if(file_open == 1)
	{

		count = fat_read_file(fd, buffer, sizeof(buffer));
		
	
		
	}
	strcpy( data, buffer );
*/
	
   
}
//##################################################################################################
void sd_file_new(const char* filename)
{
	// Nun ein neues File erstellen
	if(!fat_create_file(dd, filename , &file_entry))
	{
		
	}
	else
	{
		sd_raw_sync();
	}
} 
//##################################################################################################
void sd_file_close(void)
{
	if(file_open == 1)			// Zur Sicherheit noch mal einen sync, damit ist das FS konsistent
		{
		sd_raw_sync();		
		fat_close_file(fd);
	
		file_open=0;	
		}   			
} 
//########################################################################################## 
void sd_file_write(const char* line)
{
	if(file_open == 1)
	{
	// Auf das Ende des Files positionieren
	// FAT_SEEK_END und offset=0 heissen, vom Ende der Datei aus 0 Bytes lesen,
	// mit anderen Worten, an das Ende des Files gehen.
	int32_t write_offset = 0;
	fat_seek_file(fd, &write_offset, FAT_SEEK_END);
	
	uint8_t data_len = strlen(line);
	if(fat_write_file(fd, (uint8_t*) line, data_len) != data_len)
		{
	printf("#error writing to file\r");
			return;
		}	
	sd_raw_sync();	
	}
}
//##########################################################################################
uint8_t sd_get_disk_info(void)
{
	return print_disk_info(fs);
}	



