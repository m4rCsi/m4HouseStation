/*
 * sdcard.cpp
 *
 * Created: 25/03/2013 21:19:17
 *  Author: m4rCsi
 */
#include	"SD_card.h"
#include	"Arduino.h"
#include	"globe.h"
#include	"IniFile.h"
#include	"Flash.h"
#include	"Webserver.h"
#include	"Logging.h"

// SD Stuff
Sd2Card		card;
SdVolume	volume;
SdFile		root;
SdFile		file;

/*
void printErrorMessage(uint8_t e, bool eol = true)
{
	switch (e) {
		case IniFile::errorNoError:
		Serial << F("no error");
		break;
		case IniFile::errorFileNotFound:
		Serial << F("file not found");
		break;
		case IniFile::errorFileNotOpen:
		Serial << F("file not open");
		break;
		case IniFile::errorBufferTooSmall:
		Serial << F("buffer too small");
		break;
		case IniFile::errorSeekError:
		Serial << F("seek error");
		break;
		case IniFile::errorSectionNotFound:
		Serial << F("section not found");
		break;
		case IniFile::errorKeyNotFound:
		Serial << F("key not found");
		break;
		case IniFile::errorEndOfFile:
		Serial << F("end of file");
		break;
		case IniFile::errorUnknownError:
		Serial << F("unknown error");
		break;
		default:
		Serial << F("unknown error value");
		break;
	}
	if (eol)
	Serial.println();
}*/

void print_list(Print& printto)
{
	dir_t* p;

	root.rewind();
	while (p = root.readDirCache())
	{
		// done if past last used entry
		if (p->name[0] == DIR_NAME_FREE) break;

		// skip deleted entry and entries for . and  ..
		if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') continue;

		// only list subdirectories and files
		if (!DIR_IS_FILE(p)) continue;

		// print file name with possible blank fill
		for (uint8_t i = 0; i < 11; i++)
		{
			if (p->name[i] == ' ') continue;
			if (i == 8)
			{
				printto.print('.');
			}
			printto.write(p->name[i]);
		}
		printto.println();
	}
}

int sd_init()
{
	//Serial << F("SD_init:") << freeRam() << "\r\n";
	// initialize the SD card.
	// pass over the speed and Chip select for the SD card
	if (!card.init(SPI_FULL_SPEED, SD_CS)) {
		return -10;
	}
	// initialize a FAT volume.
	if (!volume.init(&card)) {
		return -11;
	}
	if (!root.openRoot(&volume)) {
		return -12;
	}
	return 1;
}

int readIni()
{
	Serial << F("sd0:") << freeRam() << "\r\n";
	char flag = 1;
	file.open(&root, "config.ini", O_READ);
	if(!file.isOpen())
	{
		return -2;
	}
	
	Serial << F("sd1:") << freeRam() << "\r\n";
	
	const size_t bufferLen = 30;
	char buffer[bufferLen];
	IniFile ini(file);
	
	Serial << F("sd2:") << freeRam() << "\r\n";
	
	// not necessary, just uses memory
	/*if (!ini.validate(buffer, bufferLen))
	{
		//printErrorMessage(ini.getError());
		file.close();
		return -3;
	}*/
	
	//Serial << F("Ini validated\r\n");
	
	const char *section = "network";
	if(!ini.getIPAddress(section, "ip",buffer, bufferLen, webserver_ip))
	{
		//printErrorMessage(ini.getError());
		flag = -4;
	}
	
		Serial << F("sd2.5:") << freeRam() << "\r\n";
	
	if(!ini.getMACAddress(section, "mac",buffer, bufferLen,webserver_mac))
	{
		//printErrorMessage(ini.getError());
		flag = -5;
	}
	
	Serial << F("sd3:") << freeRam() << "\r\n";
	
	ini.getValue("gas","imp2meter",buffer,bufferLen);
	gas.increaseMeter = atoi(buffer);
	ini.getValue("ele","imp2meter",buffer,bufferLen);
	ele.increaseMeter = atoi(buffer);
	
	Serial << F("sd4:") << freeRam() << "\r\n";
	
	file.close();
	Serial << F("sd5:") << freeRam() << "\r\n";
	return flag;
}