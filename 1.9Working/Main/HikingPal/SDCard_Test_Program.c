#include <stdio.h>
#include <stdlib.h>
#include <altera_up_sd_card_avalon_interface.h>
#include "SDCard_Test_Program.h"


void TestSDCard(){
	alt_up_sd_card_dev *device_reference = NULL;
	int connected = 0;
	printf("--------------------------------\n");
	printf("Opening SDCard\n");
	if((device_reference = alt_up_sd_card_open_dev("/dev/Altera_UP_SD_Card_Avalon_Interface_0")) == NULL)
	{
		printf("SDCard Open FAILED\n");
	}
	else
		printf("SDCard Open PASSED\n");


	if (device_reference != NULL ) {
		while(1) {
			if ((connected == 0) && (alt_up_sd_card_is_Present())){
				printf("Card connected.\n");
				if (alt_up_sd_card_is_FAT16()) {
					printf("FAT16 file system detected.\n");

					#define MAX_NAME_LEN 256
					#define DIR_NAME "A"

					char file_name[MAX_NAME_LEN];

					if (alt_up_sd_card_find_first("", file_name) == 0) {
						printf("Found file: %s\n", file_name);
						while (alt_up_sd_card_find_next(file_name) == 0) {
							printf("Found file: %s\n", file_name);
						}
					}
				}
				else {
					printf("Unknown file system.\n");
				}
				connected = 1;
			} else if((connected == 1) && (alt_up_sd_card_is_Present() == false)){
				printf("Card disconnected.\n");
				connected =0;
			}
		}
	}
	else
		printf("Can't open device\n");
}

void WriteToFile(){
	short int myFileHandle;
	int i;
	if(alt_up_sd_card_is_Present() && alt_up_sd_card_is_FAT16()) {
		if((myFileHandle = alt_up_sd_card_fopen("test.txt", true)) != -1) {
			printf("File Opened\n");
			for(i = 0; i < 1024; i ++){
				if(alt_up_sd_card_write(myFileHandle,'A') == false){
					printf("Error writing to file...\n");
					return;
				}
			}
			printf("Done!!!\n");
			alt_up_sd_card_fclose(myFileHandle);
		}
		else
			printf("File NOT Opened\n");
		}
}
