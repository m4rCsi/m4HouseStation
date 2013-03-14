#include	"Webserver.h"
#include	"Logging.h"

// pin 4 is the SPI select pin for the SDcard
const int SD_CS = 4;

// pin 10 is the SPI select pin for the Ethernet
const int ETHER_CS = 10;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,42);

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

/*void printDirectory(File dir, int numTabs) {
	while(true) {
		
		File entry =  dir.openNextFile();
		if (! entry) {
			// no more files
			//Serial.println("**nomorefiles**");
			break;
		}
		for (uint8_t i=0; i<numTabs; i++) {
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory()) {
			Serial.println("/");
			printDirectory(entry, numTabs+1);
		} else {
			// files have sizes, directories do not
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}
}*/

boolean list_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/plain");
	web_server.end_headers();
	
	//printDirectory(new File('/'),2);
	//root.ls();
	
	
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
	Serial.print("p: ");
	Serial.print(c);
	Serial.print(" a: ");
	Serial.println(available);
	
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
						Serial.println(parseValue);
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

boolean webserver_init()
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

	// initialize the SD card.
	//Serial << F("Setting up SD card...\n");
	// pass over the speed and Chip select for the SD card
	if (!card.init(SPI_FULL_SPEED, SD_CS)) {
		has_filesystem = false;
	}
	// initialize a FAT volume.
	if (!volume.init(&card)) {
		has_filesystem = false;
	}
	if (!root.openRoot(&volume)) {
		has_filesystem = false;
	}

	if (has_filesystem) 
	{
		//Serial << F("has filesystem \n");
		
		/*file.open(&root, "datalog.txt", FILE_WRITE);
		if (file.isOpen())
		{
			file.seekEnd();
			file << F("started logging");
			file.close();
		}
		else
		{
			Serial << F("not able to write in datalog.txt\n");
		}	*/
	}
	else
	{
		return false;
	}
	
	// Initialize the Ethernet.
	//Serial << F("Setting up the Ethernet card...\n");
	Ethernet.begin(mac,ip);
	/*{
		Serial.println("DHCP failed");
		return false;	
	};//, ip);*/
	
	//Serial.println(Ethernet.localIP());

	// Start the web server.
	//Serial << F("Web server starting...\n");
	web.begin();

	//Serial << F("Server started.\n");
	
	return true;
}

void webserver_process()
{
	web.process();
}