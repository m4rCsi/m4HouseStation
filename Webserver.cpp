﻿#include	"Webserver.h"
#include	"Logging.h"
#include	"IniFile.h"

// pin 4 is the SPI select pin for the SDcard
const int SD_CS = 4;

// pin 10 is the SPI select pin for the Ethernet
const int ETHER_CS = 10;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
static uint8_t mac[] = { 0, 0, 0, 0, 0, 0 };
IPAddress ip(0,0,0,0);

// SD Stuff
boolean has_filesystem = true;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

/************************************************************************/
/*  Handlers                                                            */
/************************************************************************/
boolean file_handler(TinyWebServer& web_server);
boolean now_handler(TinyWebServer& web_server);
boolean post_handler(TinyWebServer& web_server);
boolean index_handler(TinyWebServer& web_server);
boolean list_handler(TinyWebServer& web_server);

TinyWebServer::PathHandler handlers[] = {
  // Work around Arduino's IDE preprocessor bug in handling /* inside
  // strings.
  //
  // `put_handler' is defined in TinyWebServer
  {"/",		TinyWebServer::GET, &index_handler },
  /*{"/" "?*",TinyWebServer::GET, &index_handler },*/
  {"/" "now.txt", TinyWebServer::GET, &now_handler },
  {"/" "list.txt", TinyWebServer::GET, &list_handler },
  {"/" "*", TinyWebServer::GET, &file_handler },
  {"/" "*", TinyWebServer::POST, &post_handler },
  {NULL},
};

const char* headers[] = {
  "Content-Length",
  NULL
};

/************************************************************************/
/*  Webserver                                                           */
/************************************************************************/
TinyWebServer web = TinyWebServer(handlers, headers);

void send_file_name(TinyWebServer& web_server, const char* filename) {
  if (!filename) {
    web_server.send_error_code(404);
  } else {
    if (file.open(&root, filename, O_READ)) 
	{
		TinyWebServer::MimeType mime_type
		= TinyWebServer::get_mime_type_from_filename(filename);
		web_server.send_error_code(200);
		web_server.send_content_type(mime_type);
		web_server.end_headers();
		web_server.send_file(file);
		file.close();
    } else {
		web_server.send_error_code(404);
    }
  }
}

boolean index_handler(TinyWebServer& web_server) {
	send_file_name(web_server, "index.htm");
	return true;
}

boolean file_handler(TinyWebServer& web_server) {
  char* filename = TinyWebServer::get_file_from_path(web_server.get_path());
  send_file_name(web_server, filename);
  //Serial.println(filename);
  free(filename);
  return true;
}

boolean now_handler(TinyWebServer& web_server) 
{
    web_server.send_error_code(200);
    web_server.send_content_type("text/plain");
    web_server.end_headers();
    sendLoggingStats(web_server);
    return true;
}

void print_list(TinyWebServer& web_server)
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
			  web_server.print('.');
		  }
		  web_server.write(p->name[i]);
	  }
	  web_server.println();
   }
}

boolean list_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/plain");
	web_server.end_headers();
	print_list(web_server);
	return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
enum parseState {
	START,
	W4_E,
	VALUE,
};

enum	parseState		pState = START;
unsigned long			parseValue = 0;
char					parseMeter = 0;

void parsePost(char c, int available)
{
	#ifdef DEBUGM4
		Serial.print("p: ");
		Serial.print(c);
		Serial.print(" a: ");
		Serial.println(available);
	#endif
	
	switch (pState)
	{
		case START:
				{
					if (c == 'g')
					{
						parseMeter = 'g';
						pState = W4_E; break;
					}					
						
					if (c == 'e')
					{
						parseMeter = 'e';
						pState = W4_E; break;
					}
					break;						
				}
		case W4_E:
				{
					if (c == '=')
					{
						pState = VALUE;
						parseValue = 0;
					}					
					break;
				}
		case VALUE:
				{
					if (c >= '0' && c <= '9')
					{
						c -= '0';
						parseValue *= 10;
						parseValue += c;
					}
					
					if (available == 0 || c == '&')
					{
						#ifdef DEBUGM4
							Serial.println(parseValue);
						#endif
						
						setMeters(parseMeter,parseValue);
						pState = START;
						parseMeter = 0;
						parseValue = 0;
					}
					break;
				}
		default: break;
	}
}

boolean post_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.send_content_type("text/plain");
  web_server.end_headers();
  Client& client = web_server.get_client();
  pState = START;
  while (client.available())
  {
	  parsePost((char)client.read(),client.available());
  }
  return true;
};

void printErrorMessage(uint8_t e, bool eol = true)
{
	switch (e) {
		case IniFile::errorNoError:
		Serial.print("no error");
		break;
		case IniFile::errorFileNotFound:
		Serial.print("file not found");
		break;
		case IniFile::errorFileNotOpen:
		Serial.print("file not open");
		break;
		case IniFile::errorBufferTooSmall:
		Serial.print("buffer too small");
		break;
		case IniFile::errorSeekError:
		Serial.print("seek error");
		break;
		case IniFile::errorSectionNotFound:
		Serial.print("section not found");
		break;
		case IniFile::errorKeyNotFound:
		Serial.print("key not found");
		break;
		case IniFile::errorEndOfFile:
		Serial.print("end of file");
		break;
		case IniFile::errorUnknownError:
		Serial.print("unknown error");
		break;
		default:
		Serial.print("unknown error value");
		break;
	}
	if (eol)
	Serial.println();
}

int readIni()
{
	Serial << F("readIni:") << freeRam() << "\r\n";
	//read config.ini
	file.open(&root, "config.ini", O_READ);
	if(file.isOpen())
	{
		const size_t bufferLen = 20;
		char buffer[bufferLen];
			
		IniFile ini(file);
			
		if (!ini.validate(buffer, bufferLen))
		{
			printErrorMessage(ini.getError());
			return -3;
		}
			
		Serial << F("Ini validated");
		uint8_t ip2[4];
			
		if(!ini.getIPAddress("network", "ip2",buffer, bufferLen, ip2))
		{
			return -4;
		}
			
		#ifdef DEBUGM4
		//ip.printTo(Serial);
		Serial << ip2[0];
		Serial.println();
		#endif
			
		if(ini.getMACAddress("network", "mac",buffer, bufferLen,mac))
		{
			return -5;
		}
		Serial << mac[0];
		Serial.println("");
	}
	else
	{
		return -2;
	}

}

int webserver_start()
{
	Serial << F("webserver_start:") << freeRam() << "\r\n";
	Ethernet.begin(mac,ip);
	web.begin();
}

int sd_init()
{
	Serial << F("SD_init:") << freeRam() << "\r\n";
	// initialize the SD card.
	//Serial << F("Setting up SD card...\n");
	// pass over the speed and Chip select for the SD card
	if (!card.init(SPI_FULL_SPEED, SD_CS)) {
		has_filesystem = false;
		return -10;
	}
	// initialize a FAT volume.
	if (!volume.init(&card)) {
		has_filesystem = false;
		return -11;
	}
		
	Serial << freeRam() << "\r\n";
	if (!root.openRoot(&volume)) {
		has_filesystem = false;
		return -12;
	}
	Serial << F("SD present\n");
	return 1;
}

void webserver_init()
{
	pinMode(SS_PIN, OUTPUT);	// set the SS pin as an output
	// (necessary to keep the board as
	// master and not SPI slave)
	digitalWrite(SS_PIN, HIGH); // and ensure SS is high

	// Ensure we are in a consistent state after power-up or a reset
	// button These pins are standard for the Arduino w5100 Rev 3
	// ethernet board They may need to be re-jigged for different boards
	pinMode(ETHER_CS, OUTPUT);		// Set the CS pin as an output
	digitalWrite(ETHER_CS, HIGH);	// Turn off the W5100 chip! (wait for
	// configuration)
	pinMode(SD_CS, OUTPUT);			// Set the SDcard CS pin as an output
	digitalWrite(SD_CS, HIGH);		// Turn off the SD card! (wait for
	// configuration)
}

void webserver_process()
{
	web.process();
}