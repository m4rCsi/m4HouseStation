#include	"Webserver.h"
#include	"Logging.h"
#include	"SD_card.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
uint8_t		webserver_mac[] = { 0, 0, 0, 0, 0, 0 };
IPAddress	webserver_ip(0,0,0,0);

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

/************************************************************************/
/*                                                                      */
/************************************************************************/
void printMAC()
{
	for(int i = 0; i < 6; i++)
	{
		Serial << webserver_mac[i];
		Serial << ":";
	}
	Serial.println();
}

int webserver_start()
{
	//Serial << F("webserver_start:") << freeRam() << "\r\n";
	webserver_ip.printTo(Serial); Serial.println();
	printMAC();
	Ethernet.begin(webserver_mac,webserver_ip);
	web.begin();
}

void webserver_process()
{
	web.process();
}