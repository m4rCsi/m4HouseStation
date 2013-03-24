/************************************************************************/
/*                                                                      */
/************************************************************************/
#include	"Arduino.h"
#include	"Ethernet.h"
#include	<Wire.h>
#include	"RTClib.h"
#include	"Flash.h"
#include	"Webserver.h"
#include	"Logging.h"
#include	"globe.h"

RTC_DS1307 RTC;

int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

//#define ADJUSTTIME
#define DISABLELOGGING

void end(int flag)
{
	#ifdef DEBUGM4
	Serial << F("Webserver Failed ");
	Serial << (int)flag;
	Serial << "\r\n";
	#endif
while(1){};
}

void setup()
{
	// Serial
	#ifdef DEBUGM4
		Serial.begin(115200);
		Serial << F("Setup\r\n");
	#endif
	
	// RTC
	Wire.begin(); 
	RTC.begin();
	
	#ifdef ADJUSTTIME
		RTC.adjust(DateTime(__DATE__, __TIME__));
	#endif
	
	#ifndef DISABLELOGGING
		logging_init();
	#endif
	
	//delay(1000);
	Serial << freeRam() << "\r\n";
	
	webserver_init();
	
	int flag = sd_init();
	if(flag <  1)
	{
		end(flag);
	};
	
	//flag = readIni();
	if (flag < 1)
	{
		end(flag);
	}

	webserver_start();
	
	#ifdef DEBUGM4
		Serial << F("Setup finished...program starting\r\n");
	#endif
}

#ifdef DEBUGTICK
	void pointtick()
	{
		static unsigned int counter = 0;
		if (counter++ > 60000)
		{
			Serial.println(".");
			counter = 0;
		}
	}
#endif

/************************************************************************/
/* Custom main function					                                */
/************************************************************************/
int main(void)
{
	init();

	#ifdef DEBUGM4
		#if defined(USBCON)
		USBDevice.attach();
		#endif
	#endif
	
	setup();
	
	for (;;) 
	{
		#ifndef DISABLELOGGING
			logging_process();
		#endif
		webserver_process();
		
		#ifdef DEBUGTICK
			pointtick();
		#endif
		
		#ifdef DEBUGM4
			if (serialEventRun) serialEventRun();
		#endif
	}
	return 0;
}