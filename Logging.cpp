/*
 * logging.cpp
 *
 * Created: 22/02/2013 19:34:31
 *  Author: m4rCsi
 */ 
#include "Logging.h"
#include "globe.h"
#include "EEPROM.h"
#include "SD_card.h"

#define DISABLE_ENV

struct Meter{
	char *		identifier;
	uint32_t	meter;
	uint32_t	lastPulse;	
	uint16_t	timedelta;
	uint16_t	counter;
	uint8_t		increaseMeter;
	uint16_t	min;
	uint8_t		newImp;
};

DateTime		rtc_time = NULL;

volatile Meter	ele = {"ele",0,0,0,0,1,100,0};
volatile Meter	gas = {"gas",0,0,0,0,80,1000,0};

// estimated gas_meter, 1 imp = +1
// ex. 221043 = 2210.43  
/*volatile unsigned long	gas_meter = 0;
volatile int			gas_timedelta = 0;	// in ms								
volatile unsigned long	last_GasInterrupt = 0;

// estimated ele meter, every 80 imp = +1
// ex. 52360 = 5236.0 kWh
volatile unsigned long	ele_meter = 0;
volatile int			ele_counter = 0;
volatile int			ele_timedelta = 0;	// in ms

volatile unsigned char	gas_newImp = 0;
volatile unsigned char	ele_newImp = 0;

#ifndef DISABLE_LIVE_PC
	volatile int			gas_power = 0;		// in W = 397822784/delta ca.(0.01*11*1000*60*60*1000)
												// in m3/h = 0.01*1000*60*60/delta (36000/delta)
	volatile int			ele_power = 0;		// in W = (1/800 * 1000 * 60 * 60 * 1000)/delta
#endif*/

// Temperature
volatile int				temperature = 0;
volatile unsigned char		humidity = 0;

#define TEMP_INTERVAL_MIN	15
#define TEMP_TIMER			5				//in seconds
#define BIG_INTERVAL		TEMP_INTERVAL_MIN*60/TEMP_TIMER

// Intervalls
volatile int				intervall_cnt = 0;
volatile boolean			small_intervall = false;
volatile boolean			med_intervall = false;
volatile boolean			big_intervall = true;

void backupMeters();

/************************************************************************/
/*                                                                      */
/************************************************************************/
void setMeters(char meter, unsigned long value)
{
	if (meter == 'g')
	{
		gas.meter = value;
	}
	else if (meter == 'e')
	{
		ele.meter = value;
	}
	backupMeters();
}

/************************************************************************/
/* process Impulse                                                      */
/************************************************************************/
void process_Impulse(volatile Meter *m)
{
	unsigned long time = millis();
	unsigned long delta = time-m->lastPulse;
	if (delta > m->min)
	{
		m->timedelta = delta;
		m->lastPulse = time;
		
		m->counter++;
		if (m->counter >= m->increaseMeter)
		{
			m->meter++;
			m->counter = 0;	
		}
		m->newImp++;
	}
}

/************************************************************************/
/*  Gas Interrupt                                                       */
/************************************************************************/
ISR (INT0_vect)
{
	process_Impulse(&gas);
}

/************************************************************************/
/* Ele Interrupt                                                        */
/************************************************************************/
ISR (INT1_vect)
{
	process_Impulse(&ele);
}

void printMeterData(Print &to, volatile Meter* m)
{
	to << m->identifier << F("_meter, ") << m->meter << F("\n");
	to << m->identifier << F("_td, ") << m->timedelta << F("\n");
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void printLoggingStats(Print& to)
{
	printMeterData(to, &gas);
	printMeterData(to, &ele);
	
	#ifndef DISABLE_ENV
		to << F("temp, ") << (temperature/10) << F(".") << (temperature%10) << F("\n");
		to << F("hum, ") << humidity << F("\n");
	#endif
	
	to << F("time, ") << rtc_time.unixtime() << F("\n");
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
ISR(TIMER1_OVF_vect)
{
	intervall_cnt++;
	if (intervall_cnt > BIG_INTERVAL) // 15 minutes
	{
		intervall_cnt = 0;
		big_intervall = true;
	}
	if (intervall_cnt%12 == 0)
	{
		med_intervall = true;
	}
	if (gas.timedelta > 0)
	{
		small_intervall = true;
	}
}

/************************************************************************/
/*  EEPROM Help with long                                               */
/************************************************************************/
 unsigned int EEPROM_readint(int address) {
	 unsigned int word = word(EEPROM.read(address), EEPROM.read(address+1));
	 return word;
 }

 // read double word from EEPROM, give starting address
 unsigned long EEPROM_readlong(int address) {
	 //use word read function for reading upper part
	 unsigned long dword = EEPROM_readint(address);
	 //shift read word up
	 dword = dword << 16;
	 // read lower word from EEPROM and OR it into double word
	 dword = dword | EEPROM_readint(address+2);
	 return dword;
 }

 //write word to EEPROM
 void EEPROM_writeint(int address, int value) {
	 EEPROM.write(address,highByte(value));
	 EEPROM.write(address+1 ,lowByte(value));
 }
 
 //write long integer into EEPROM
 void EEPROM_writelong(int address, unsigned long value) {
	 //truncate upper part and write lower part into EEPROM
	 EEPROM_writeint(address+2, word(value));
	 //shift upper part down
	 value = value >> 16;
	 //truncate and write
	 EEPROM_writeint(address, word(value));
 }
 
 /************************************************************************/
 /*                                                                      */
 /************************************************************************/
 inline void initTimer()
 {
	 char oldSREG;					// To hold Status Register while ints disabled
		 
	 TCCR1A = 0;                 // clear control register A
	 TCCR1B = _BV(WGM13);        // set mode 8: phase and frequency correct pwm, stop the timer
		
	 oldSREG = SREG;
	 cli();							// Disable interrupts for 16 bit register access
	 ICR1 = 39062;                                          // ICR1 is TOP in p & f correct pwm mode
	 SREG = oldSREG;
		
	 TIMSK1 = _BV(TOIE1);                                     // sets the timer overflow interrupt enable bit
		     
	 TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
	 TCCR1B |= (_BV(CS12) | _BV(CS10));  // prescale by /1024 
 }
 
 /************************************************************************/
 /*                                                                      */
 /************************************************************************/
 inline void initInterrupt()
 {
	 DDRD &= ~((1 << DDD2)&(1 << DDD3)); // Input
	 PORTD |= ((1 << PORTD3)&(1 << PORTD2)); // Pull Up
	 EICRA |= ((1 << ISC00)|(1 << ISC01)|(1 << ISC10)|(1 << ISC11)); // Rising Edge
	 EIMSK |= (1 << INT0);     // Turns on INT0
	 EIMSK |= (1 << INT1);     // Turns on INT0

	 sei();                    // turn on interrupts
 }

/************************************************************************/
/* INIT                                                                 */
/************************************************************************/
void logging_init()
{
	initInterrupt();
	initTimer();
	
	// read saved meters
	gas.meter = EEPROM_readlong(0);
	ele.meter = EEPROM_readlong(4);
}

void twoDigits(String str, char * buf)
{
	if (str.length() == 2)
	{
		buf[0] = str.charAt(0);
		buf[1] = str.charAt(1);
	}
	else
	{
		buf[0] = '0';
		buf[1] = str.charAt(0);
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void getFileName(char * buf, char prefix, DateTime time)
{
	buf[0] = prefix;
	
	String year = String(time.year());
	buf[1] = year.charAt(2);
	buf[2] = year.charAt(3);
	
	String month = String(time.month());
	twoDigits(month, &buf[3]);
	
	String day = String(time.day());
	twoDigits(day, &buf[5]);
	
	buf[7] = '.';
	buf[8] = 't';
	buf[9] = 'x';
	buf[10] = 't';
	buf[11] = '\0';
}

void writeLogLine(Print &to, char * identifier, char * idsuffix, uint32_t val1, uint32_t val2 = NULL)
{
	to.print(identifier);
	if (idsuffix != NULL) to.print(idsuffix);
	to.print(", ");
	to.print(val1);
	if (val2 != NULL)
	{
		to.print(", ");
		to.print(val2);
	}
	to.println();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void check_newDay()
{
	static byte day = 0;
	
	rtc_time = RTC.now();
	
	if (rtc_time.hour() == 0 &&
		rtc_time.minute() <= 1 &&
		rtc_time.day() != day )
	{
		day = rtc_time.day();
		file.open(&root, "meter.txt", (O_READ | O_WRITE | O_CREAT));
		// if the file is available, write to it:
		if (file.isOpen())
		{
			file.seekEnd();
			writeLogLine(file, gas.identifier, "meter", gas.meter, rtc_time.unixtime());
			writeLogLine(file, ele.identifier, "meter", ele.meter, rtc_time.unixtime());
			file.close();
		}
		else
		{
			// if the file isn't open, pop up an error:
			//Serial << F("error opening Meter");
		}
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void logging_write()
{
	char filename[12];
	rtc_time = RTC.now();
		
	getFileName(filename,'M',rtc_time);
	
	file.open(&root, filename, (O_READ | O_WRITE | O_CREAT));
	// if the file is available, write to it:
	if (file.isOpen()) 
	{
		file.seekEnd();
		if (gas.newImp)
		{
			writeLogLine(file, gas.identifier, NULL, gas.newImp, rtc_time.unixtime());
			gas.newImp = 0;
		}
		if (ele.newImp)
		{
			writeLogLine(file, ele.identifier, NULL, ele.newImp, rtc_time.unixtime());
			ele.newImp = 0;
		}
		file.close();
	}		
	else 
	{
		// if the file isn't open, pop up an error:
		#ifdef DEBUGM4
			Serial << F("error opening M");
		#endif
	}

}

/************************************************************************/
/*  Temp C° = (100 * V) - 50                                            */
/************************************************************************/
int readTemp()
{
	unsigned long sensorVal = analogRead(A0);
	sensorVal *= 5000;
	sensorVal /= 1024;
	sensorVal -= 500;
	
	return (int)sensorVal;
}

/************************************************************************/
/*   //(VOUT - 0.826)/0.0315                                            */
/************************************************************************/
unsigned char readHum()
{
	unsigned long sensorVal = analogRead(A1);
	
	sensorVal *= 5000;
	sensorVal /= 1024;
	if (sensorVal > 826)
	{
		sensorVal -= 826;
		sensorVal /= 31;
		if (sensorVal > 100) sensorVal = 100;
	}
	else
	{
		sensorVal = 0; 
	}
	return (unsigned char)sensorVal;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void backupMeters()
{
	EEPROM_writelong(0,gas.meter);
	EEPROM_writelong(4,ele.meter);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void logging_Environment()
{
	temperature = readTemp();
	humidity = readHum();

	char filename[12];
	DateTime time = RTC.now();
		
	getFileName(filename,'E',time);
		
	//Serial.println(filename);
			
	file.open(&root, filename, (O_READ | O_WRITE | O_CREAT));
	// if the file is available, write to it:
	if (file.isOpen())
	{
		file.seekEnd();
					
		file << F("Temp, ");
		file.print(temperature/10);
		file.print(".");
		file.print(temperature%10);
		file.print(", ");
		file.println(time.unixtime());
			
		file << F("Hum, ");
		file.print(humidity);
		file.print(", ");
		file.println(time.unixtime());
			
		file.close();
	}
	else
	{
		// if the file isn't open, pop up an error:
		#ifdef DEBUGM4
			Serial << F("error writing E");
		#endif
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void logging_process()
{
	if (gas.newImp + ele.newImp > 0)
	{
		logging_write();
	}
	else if (big_intervall)	// 15 minutes
	{	
	#ifndef DISABLE_ENV
		logging_Environment();
	#endif
		backupMeters();
	
		big_intervall = false;
	}
	else if (med_intervall) // 1 minute
	{
		check_newDay();
		med_intervall = false;
	}
	#ifndef DISABLE_LIVE_PC
		else if (small_intervall)	// 5 seconds
		{
			if (millis() - gas.lastPulse > 25000) // over 25 seconds
			{
				gas.timedelta = 0;
			}
			small_intervall = false;
		}
	#endif
}