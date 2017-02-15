#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define WifiDongle_Control (*(volatile unsigned char *)(0x84000240))
#define WifiDongle_Status (*(volatile unsigned char *)(0x84000240))
#define WifiDongle_TxData (*(volatile unsigned char *)(0x84000242))
#define WifiDongle_RxData (*(volatile unsigned char *)(0x84000242))
#define WifiDongle_Baud (*(volatile unsigned char *)(0x84000244))

void Wifi_Init(void) {
	// Reset
	WifiDongle_Control = 0x03;
	WifiDongle_Control = 0x15;
	WifiDongle_Baud = 0x01;
}


char getcharWifi(void) {
	while((WifiDongle_Status & 0x01) != 0x01) {}
	return WifiDongle_RxData;
}

void putcharWifi(char val) {
	while((WifiDongle_Status & 0x02) != 0x02) {}
	WifiDongle_TxData = val;
}

void WifiWait(void) {
	char val = getcharWifi();
	printf (val);
	while(val != '>') {
		val = getcharWifi();
		printf("%c", val);
	}

	printf("\n");
}

void Wifi_SendCommand(const char * command) {
	int i;
	int length =strlen(command);
	for(i=0; i<length;i++) {
		putcharWifi(command[i]);
	}
}


int main(char*start, char *path)
{
	Wifi_Init();

	printf("Start\n");

	Wifi_SendCommand("\r\n");
	Wifi_SendCommand("\r\n");
//	Wifi_SendCommand("\r\n");
//	WifiWait();
//	Wifi_SendCommand("dofile(\"send_text_message.lua\")\r\n");
//	WifiWait();
//	Wifi_SendCommand("check_wifi()\r\n");

	WifiWait();
	Wifi_SendCommand("dofile(\"googleMap.lua\")\r\n");
	WifiWait();
	//main("49.2606050,-123.2459940","|49.2606,-123.24")
	Wifi_SendCommand("main(\"49.2606050,-123.2459940\",\"|49.2606,-123.24\")");
	Wifi_SendCommand("\r\n");

	while(1) {
		char val = getcharWifi();
		printf("%c", val);
	}
	usleep(1000000);
	printf("Done\n");
	return 0;
}
