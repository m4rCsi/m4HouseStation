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

RTC_DS1307	RTC;

//#define ADJUSTTIME

void setup()
{
	boolean		webserver_state = false;
	// Serial
	#ifdef DEBUGM4
		Serial.begin(115200);
		Serial.println("Setup");
	#endif
	
	// RTC
	Wire.begin(); 
	RTC.begin();
	
	#ifdef ADJUSTTIME
		RTC.adjust(DateTime(__DATE__, __TIME__));
	#endif
	
	logging_init();
	
	webserver_state = webserver_init();
	if (!webserver_state)
	{
		#ifdef DEBUGM4
			Serial << F("Webserver Failed");
		#endif
		while(1){};
	}
	#ifdef DEBUGM4
		Serial.println("Setup finished...Starting");
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
		logging_process();
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