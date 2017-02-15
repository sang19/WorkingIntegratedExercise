
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "altera_up_avalon_character_lcd.h"
#include "io.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "gps.h"


volatile char line1[MAXLINELENGTH];
volatile char line2[MAXLINELENGTH];

volatile int lineidx = 0;
volatile char *currentline;
volatile char *lastline;
volatile int receivedFlag;
volatile int inStandbyMode;
volatile int paused;
volatile char *format;

char* lastNMEA(void);
int newNMEAreceived(void);

void sendCommand(const char *);


int waitForSentence(const char *wait);
int parseData(char *response);
int parseHex(char c);


int wakeup(void);
int standby(void);



int hour, minute, seconds, year, month, day;
int milliseconds;
// Floating point latitude and longitude value in degrees.
float latitude, longitude;
// Fixed point latitude and longitude value with degrees stored in units of 1/100000 degrees,
// and minutes stored in units of 1/100000 degrees.  See pull #13 for more details:
//   https://github.com/adafruit/Adafruit-GPS-Library/pull/13
int32_t latitude_fixed, longitude_fixed;
float latitudeDegrees, longitudeDegrees;
float geoidheight, altitude;
float speed, angle, magvariation, HDOP;
char lat, lon, mag;
volatile int fix;
int fixquality, satellites;
int LOCUS_serial, LOCUS_records;
int LOCUS_type, LOCUS_mode, LOCUS_config, LOCUS_interval, LOCUS_distance, LOCUS_speed, LOCUS_status, LOCUS_percent;
int called_Init = 0;

int mode = 1;


void Init_GPSCHIP(void)
{
 // set up 6850 Control Register to utilise a divide by 16 clock,
 // set RTS low, use 8 bits of data, no parity, 1 stop bit,
 // transmitter interrupt disabled
 // program baud rate generator to use 115k baud

	//The 6850 Control Register write only
	// RS232_Control(7 DOWNTO 0) = |X|1|0|1|0|1|0|1| = 0b01010101 = 0x55 rts high & interrupt disable
	// RS232_Control(7 DOWNTO 0) = |X|0|0|1|0|1|0|1| = 0b00010101 = 0x15
	if(called_Init)
		return;


	GPSCHIP_Control = 0x15;
	GPSCHIP_Baud = 0x05; // program for 9600 baud
	printf("Initializing GPS CHIP");
	receivedFlag = 0;
	paused = 0;
	lineidx = 0;
	currentline = line1;
	lastline = line2;

	hour = minute = seconds = year = month = day =
	fixquality = satellites = 0; // int
	lat = lon = mag = 0; // char
	fix = 0; // boolean
	milliseconds = 0; // int
	latitude = longitude = geoidheight = altitude =
	speed = angle = magvariation = HDOP = 0.0; // float
	called_Init = 1;
	usleep(10000);
}


int parseData(char *data) {

	if(data[strlen(data) - 4] == '*'){
		int sum = parseHex(data[strlen(data) -3])*16;
		sum += parseHex(data[strlen(data) - 2]);

		//check checksum
		int i;
		int length = strlen(data) -4;
		for(i = 2; i < length; i++) {
			sum ^= data[i];
		}

		if(sum != 0) {
			return 0;
		}

	}

	int32_t degree;
	long minutes;
	char degreebuff[10];

	//GGA format
	if(strstr(data, "$GPGGA")) {
		format = "$GPGGA";
		char *p = data;
		p = strchr(p, ',') + 1;
		float timef = atof(p);
		int time = timef;
		hour = time/10000;
		minute = (time % 10000) / 100;
		seconds = (time % 100);

		milliseconds = fmod(timef, 1.0)*1000;

		//latitude

		p = strchr(p, ',') + 1;
		if(',' != *p)
		{
			strncpy(degreebuff, p ,2);
			p +=2;
			degreebuff[2] = '\0';
			degree = atol(degreebuff)*10000000;
			strncpy(degreebuff, p, 2);
			p += 3;
			strncpy(degreebuff + 2, p , 4);
			degreebuff[6] = '\0';
			minutes = 50*atol(degreebuff) / 3;
			latitude_fixed = degree + minutes;
			latitude = degree/100000 + minutes*0.000006F;
			latitudeDegrees = (latitude-100*(latitude/100))/60.0;
			latitudeDegrees += (latitude/100);
		}

	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      if (p[0] == 'S') latitudeDegrees *= -1.0;
	      if (p[0] == 'N') lat = 'N';
	      else if (p[0] == 'S') lat = 'S';
	      else if (p[0] == ',') lat = 0;
	      else return 0;
	    }

	    p = strchr(p, ',')+1;

	    if (',' != *p)
	    {
	       strncpy(degreebuff, p, 3);
	       p += 3;
	       degreebuff[3] = '\0';
	       degree = atol(degreebuff) * 10000000;
	       strncpy(degreebuff, p, 2); // minutes
	       p += 3; // skip decimal point
	       strncpy(degreebuff + 2, p, 4);
	       degreebuff[6] = '\0';
	       minutes = 50 * atol(degreebuff) / 3;
	       longitude_fixed = degree + minutes;
	       longitude = degree / 100000 + minutes * 0.000006F;
	       longitudeDegrees = (longitude-100*(longitude/100))/60.0;
	       longitudeDegrees += (longitude/100);
	    }

	    p = strchr(p, ',')+1;
	      if (',' != *p)
	      {
	        if (p[0] == 'W') longitudeDegrees *= -1.0;
	        if (p[0] == 'W') lon = 'W';
	        else if (p[0] == 'E') lon = 'E';
	        else if (p[0] == ',') lon = 0;
	        else return 0;
	      }

	      p = strchr(p, ',')+1;
	         if (',' != *p)
	         {
	           fixquality = atoi(p);
	         }

	         p = strchr(p, ',')+1;
	         if (',' != *p)
	         {
	           satellites = atoi(p);
	         }

	         p = strchr(p, ',')+1;
	         if (',' != *p)
	         {
	           HDOP = atof(p);
	         }

	         p = strchr(p, ',')+1;
	         if (',' != *p)
	         {
	           altitude = atof(p);
	         }

	         p = strchr(p, ',')+1;
	         p = strchr(p, ',')+1;
	         if (',' != *p)
	         {
	           geoidheight = atof(p);
	         }
	         return 1;
	}


	if (strstr(data, "$GPRMC")) {
	   // found RMC
	    char *p = data;
	    format = "$GPRMC";

	    // get time
	    p = strchr(p, ',')+1;
	    float timef = atof(p);
	    int time = timef;
	    hour = time / 10000;
	    minute = (time % 10000) / 100;
	    seconds = (time % 100);

	    milliseconds = fmod(timef, 1.0) * 1000;

	    p = strchr(p, ',')+1;
	    // Serial.println(p);
	    if (p[0] == 'A')
	      fix = 1;
	    else if (p[0] == 'V')
	      fix = 0;
	    else
	      return 0;

	    // parse out latitude
	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      strncpy(degreebuff, p, 2);
	      p += 2;
	      degreebuff[2] = '\0';
	      long degree = atol(degreebuff) * 10000000;
	      strncpy(degreebuff, p, 2); // minutes
	      p += 3; // skip decimal point
	      strncpy(degreebuff + 2, p, 4);
	      degreebuff[6] = '\0';
	      long minutes = 50 * atol(degreebuff) / 3;
	      latitude_fixed = degree + minutes;
	      latitude = degree / 100000 + minutes * 0.000006F;
	      latitudeDegrees = (latitude-100*(latitude/100))/60.0;
	      latitudeDegrees += (latitude/100);
	    }

	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      if (p[0] == 'S') latitudeDegrees *= -1.0;
	      if (p[0] == 'N') lat = 'N';
	      else if (p[0] == 'S') lat = 'S';
	      else if (p[0] == ',') lat = 0;
	      else return 0;
	    }

	    // parse out longitude
	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      strncpy(degreebuff, p, 3);
	      p += 3;
	      degreebuff[3] = '\0';
	      degree = atol(degreebuff) * 10000000;
	      strncpy(degreebuff, p, 2); // minutes
	      p += 3; // skip decimal point
	      strncpy(degreebuff + 2, p, 4);
	      degreebuff[6] = '\0';
	      minutes = 50 * atol(degreebuff) / 3;
	      longitude_fixed = degree + minutes;
	      longitude = degree / 100000 + minutes * 0.000006F;
	      longitudeDegrees = (longitude-100*(longitude/100))/60.0;
	      longitudeDegrees += (longitude/100);
	    }

	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      if (p[0] == 'W') longitudeDegrees *= -1.0;
	      if (p[0] == 'W') lon = 'W';
	      else if (p[0] == 'E') lon = 'E';
	      else if (p[0] == ',') lon = 0;
	      else return 0;
	    }
	    // speed
	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      speed = atof(p);
	    }

	    // angle
	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      angle = atof(p);
	    }

	    p = strchr(p, ',')+1;
	    if (',' != *p)
	    {
	      int fulldate = atof(p);
	      day = fulldate / 10000;
	      month = (fulldate % 10000) / 100;
	      year = (fulldate % 100);
	    }
	    // we dont parse the remaining, yet!
	    return 1;
	  }

	return 0;
}


int parseHex(char c){

	if(c < '0')
		return 0;
	if(c <= '9')
	{
		int i = c - '0';
		return i;
	}
	if(c < 'A')
		return 0;
	if(c <= 'F') {
		int i = c - 'A';
		int j = i + 10;
		return j;

	}
	return 0;

}


int waitForSentence(const char *data) {
	  char str[20];

	  int i=0;
	  while (i < MAXLINELENGTH) {
	    if (newNMEAreceived() == 1) {
	      char *nmea = lastNMEA();
	      strncpy(str, nmea, 20);
	      str[19] = 0;
	      i++;

	      if (strstr(str, data))
		return 1;
	    }
	  }

	  return 0;
}

int newNMEAreceived(void) {
  return receivedFlag;
}



char* lastNMEA(void) {
  receivedFlag = 0;
  return (char *)lastline;
}

int LOCUS_StartLogger(void) {
  sendCommand(PMTK_LOCUS_STARTLOG);
  receivedFlag = 0;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK);
}

int LOCUS_StopLogger(void) {
  sendCommand(PMTK_LOCUS_STOPLOG);
  receivedFlag = 0;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK);
}

int LOCUS_ReadStatus(void) {
  sendCommand(PMTK_LOCUS_QUERY_STATUS);

  if (! waitForSentence("$PMTKLOG"))
    return 0;

  char *response = lastNMEA();
  int parsed[10];
  int i;

  for (i=0; i<10; i++) parsed[i] = -1;

  response = strchr(response, ',');
  for (i=0; i<10; i++) {
    if (!response || (response[0] == 0) || (response[0] == '*'))
      break;
    response++;
    parsed[i]=0;
    while ((response[0] != ',') &&
	   (response[0] != '*') && (response[0] != 0)) {
      parsed[i] *= 10;
      int c = response[0];
      if (isdigit(c))
        parsed[i] += c - '0';
      else
        parsed[i] = c;
      response++;
    }
  }
  LOCUS_serial = parsed[0];
  LOCUS_type = parsed[1];
  if (isalpha(parsed[2])) {
    parsed[2] = parsed[2] - 'a' + 10;
  }
  LOCUS_mode = parsed[2];
  LOCUS_config = parsed[3];
  LOCUS_interval = parsed[4];
  LOCUS_distance = parsed[5];
  LOCUS_speed = parsed[6];
  LOCUS_status = !parsed[7];
  LOCUS_records = parsed[8];
  LOCUS_percent = parsed[9];

  return 1;
}


int standby(void) {
  if (inStandbyMode == 1) {
    return 0;  // Returns false if already in standby mode, so that you do not wake it up by sending commands to GPS
  }
  else {
    inStandbyMode = 1;
    sendCommand(PMTK_STANDBY);
    //return waitForSentence(PMTK_STANDBY_SUCCESS);  // don't seem to be fast enough to catch the message, or something else just is not working
    return 1;
  }
}

int wakeup(void) {
  if (inStandbyMode) {
   inStandbyMode = 0;
    sendCommand("");  // send byte to wake it up
    return waitForSentence(PMTK_AWAKE);
  }
  else {
      return 0;  // Returns false if not in standby mode, nothing to wakeup
  }
}




char *FloatToLatitudeConversion(int x) //output format is xx.yyyy
{
	static char buff[100] ;
	float *ptr = (float *)(&x) ; // cast int to float
	float f = *ptr ; // get the float
	sprintf(buff, "%2.4f", f); // write in string to an array
	return buff ;
}
char *FloatToLongitudeConversion(int x) // output format is (-)xxx.yyyy
{
	static char buff[100] ;
	float *ptr = (float *)(&x) ;
	float f = *ptr ;
	sprintf(buff, "%3.4f", f);
	return buff ;
}


char putcharGPSCHIP(char c)
{
// poll Tx bit in 6850 status register. Wait for it to become '1'

// write 'c' to the 6850 TxData register to output the character

	while (!(GPSCHIP_Status & 0x2));
	GPSCHIP_TxData = c;

	return c;
}

void sendCommand(const char *str){

	int length = sizeof(str);

	int i = 0;
		for(i = 0; i < length; i++){
			usleep(100000); //wait for 100ms
			putcharGPSCHIP(str[i]);
		}
}



char getcharGPSCHIP( void )
{
 // poll Rx bit in 6850 status register. Wait for it to become '1'
 // read received character from 6850 RxData register.
	while (!(GPSCHIP_Status & 0x1));

	if(GPSCHIP_RxData == '\n') {

		currentline[lineidx] = 0;

		if(currentline == line1) {
			currentline = line2;
			lastline = line1;
		} else {
			currentline = line1;
			lastline = line2;
		}

		lineidx = 0;
		receivedFlag = 1;

	}

	currentline[lineidx++] = GPSCHIP_RxData;
	if(lineidx >= MAXLINELENGTH)
		lineidx = MAXLINELENGTH - 1;


	return GPSCHIP_RxData;
}

// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int GPSCHIPTestForReceivedData(void)
{
 // Test Rx bit in 6850 serial comms chip status register
 // if RX bit is set, return TRUE, otherwise return FALSE

	// RS232_Status: XXXX XXXX
	// We want bit0: 0000 0001
	return (GPSCHIP_Status & 0x1);
}

int GPSCHIPTestForTransmitData(void) {
	// Test Tx bit in 6850 serial communications chip status register
	// if TX bit is set, return TRUE, otherwise return FALSE

	// RS232_Status: XXXX XXXX
	// We want bit1: 0000 0010
	return (GPSCHIP_Status & 0x2);
}


location getLocationData() {

	Init_GPSCHIP();

	location loc;
	while(1) {
	getcharGPSCHIP();

			if(newNMEAreceived() == 1) {
				if(parseData(lastNMEA())== 1){

					loc.latitude = latitude;
					loc.latitude_dir = lat;
					loc.longitude = longitude;
					loc.longitude_dir = lon;

					printf("Location (latitude, longitude): %.2f, %c, %.2f, %c", loc.latitude, loc.latitude_dir, loc.longitude, loc.longitude_dir);
					return loc;

				}
			}

	}
	return loc;
}

void test() {
	printf("Test GPSCHIP\n");
	Init_GPSCHIP();

	alt_up_character_lcd_dev *char_lcd_dev;

	char_lcd_dev = alt_up_character_lcd_open_dev ("/dev/character_lcd_0");

	if(char_lcd_dev == NULL)
		printf("Error: could not open character LCD device\n");
	else
		printf("Character LCD Device open\n");


	alt_up_character_lcd_init(char_lcd_dev);


	/**
	while(1) {

		int number = IORD_8DIRECT(keys, 0);
		if(number == keys_off) {
			continue;
		}

		int i = 0;
		for(i = 0; i < 4; ++i) {
			if((number & (1 << i))	== 0) {
				mode = i;
				break;
			}
		}

	}
	**/


	/**

	 sendCommand(PMTK_LOCUS_QUERY_DATA);

	 while(1) {

	 	 printf("%s", lastline);

	 }
	 */


	while(1){
		getcharGPSCHIP();

		if(newNMEAreceived() == 1) {
			if(parseData(lastNMEA())
					== 1){

				alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 0);


			//rintf("%s", lastline);
				/**
				if(mode == 1) {

					char * display_time;
					sprintf(display_time, "%s Time", format);
					alt_up_character_lcd_string(char_lcd_dev, display_time);
					alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
					char * time;
					sprintf(time, "%d:%d:%d",hour, minute, milliseconds);
					alt_up_character_lcd_string(char_lcd_dev, time);

				}

				if(mode == 2) {

					char * display_date;
					sprintf(display_date, "%s Date", format);
					alt_up_character_lcd_string(char_lcd_dev, display_date);
					alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
					char * date;
					sprintf(date, "%d:%d:%d", day, month, year);
					alt_up_character_lcd_string(char_lcd_dev, date);
				}

				if(mode == 3) {

					char * latitude_display;
					sprintf(latitude_display, "%s Latitude", format);
					alt_up_character_lcd_string(char_lcd_dev, latitude_display);
					alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
					char * latitude_str;
					sprintf(latitude_str, "%.2f %c", latitude, lat);
					alt_up_character_lcd_string(char_lcd_dev, latitude_str);
				}

				if(mode == 4) {

					char * longitude_display;
					sprintf(longitude_display, "%s Longitude", format);
					alt_up_character_lcd_string(char_lcd_dev, longitude_display);
					alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
					char * longitude_str;
					sprintf(longitude_str, "%.2f %c", longitude, lon);
					alt_up_character_lcd_string(char_lcd_dev, longitude_str);
				}

	**/

				char * format_str;
				sprintf(format_str, "%s", format);
				char * time;
				sprintf(time, "%sTime:%d:%d:%d", format,hour, minute, milliseconds);
				alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0,0);
				alt_up_character_lcd_string(char_lcd_dev, time);
				//char * date;
				//sprintf(date, "Date:%d:%d:%d", day, month, year);
				//alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 6, 0);
				//alt_up_character_lcd_string(char_lcd_dev, date);
				alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
				char * location_str = "";
				sprintf(location_str,"%.2f%c%.2f%c",latitude, lat, longitude, lon);
				alt_up_character_lcd_string(char_lcd_dev, location_str);

				printf("Format: %s", format);
				printf("Time (HH: MM:ss) : %d, %d, %d", hour, minute, seconds);
				printf("Date (day:month:year): %d, %d, %d", day, month, year);
				printf("Location (latitude, longitude):%f, %c, %f, %c", latitude, lat, longitude, lon);
				printf("Geoidheight : %.2f", geoidheight);
				printf("Altitude: %.2f", altitude);

			}
		}

	}

}
