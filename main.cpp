/************************************************************************/
/*                                                                      */
/************************************************************************/
#include	"Arduino.h"
#include	"SD.h"
#include	"Ethernet.h"
#include	<Wire.h>
#include	"RTClib.h"
#include	"Flash.h"
#include	"Webserver.h"
#include	"Logging.h"
#include	"globe.h"

void		setup();
void		loop();

boolean		logging_state = false;
boolean		webserver_state = false;

RTC_DS1307 RTC;

/*void printDate()
{
	DateTime now = RTC.now();
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.println(now.second(), DEC);
}*/

void setup()
{
	// Serial
	Serial.begin(115200);
	
	// RTC
	Wire.begin(); 
	RTC.begin();
	
	//RTC.adjust(DateTime(__DATE__, __TIME__));
	
	logging_init();
	
	Serial.println("Start");
	
	webserver_state = webserver_init();
	if (!webserver_state)
	{
		Serial << F("Webserver Failed");
		while(1){};
	}
}

void pointtick()
{
	static unsigned int counter = 0;
	if (counter++ > 60000)
	{
		Serial.println(".");
		//printDate();
		counter = 0;
	}
}

void loop()
{
	logging_process();
	webserver_process();
	//pointtick();
}