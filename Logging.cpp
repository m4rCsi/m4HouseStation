/*
 * logging.cpp
 *
 * Created: 22/02/2013 19:34:31
 *  Author: m4rCsi
 */ 
#include "Logging.h"
//#include "Arduino.h"
#include "globe.h"
#include "EEPROM.h"

// estimated gas_meter, 1 imp = +1
// ex. 221043 = 2210.43  
volatile unsigned long	gas_meter = 0;
volatile int			gas_timedelta = 0;	// in ms
volatile int			gas_power = 0;		// in W = 397822784/delta ca.(0.01*11*1000*60*60*1000)
											// in m3/h = 0.01*1000*60*60/delta (36000/delta)
								
volatile unsigned long	last_GasInterrupt = 0;


// estimated ele meter, every 80 imp = +1
// ex. 52360 = 5236.0 kWh
volatile unsigned long	ele_meter = 0;
volatile int			ele_counter = 0;
volatile int			ele_timedelta = 0;	// in ms
volatile int			ele_power = 0;		// in W = (1/800 * 1000 * 60 * 60 * 1000)/delta

volatile unsigned char	gas_newImp = 0;
volatile unsigned char	ele_newImp = 0;

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
		gas_meter = value;
	}
	else if (meter == 'e')
	{
		ele_meter = value;
	}
	backupMeters();
}

/************************************************************************/
/*  Gas Interrupt                                                       */
/************************************************************************/
ISR (INT0_vect)
{
	unsigned long time = millis();
	unsigned long delta = time-last_GasInterrupt;
	if (delta > 1000)
	{
		gas_timedelta = delta;
		last_GasInterrupt = time;
		gas_power = 36000000/delta;
		
		gas_newImp++;
		gas_meter++;
	}
}

/************************************************************************/
/* Ele Interrupt                                                        */
/************************************************************************/
ISR (INT1_vect)
{
	static unsigned long last_interrupt = 0;
	unsigned long time = millis();
	unsigned long delta = time-last_interrupt;
	if (delta > 100)
	{
		ele_timedelta = delta;
		last_interrupt = time;
		ele_power = 4500000/delta;
		
		ele_newImp++;
		
		ele_counter++;
		if (ele_counter >= 80)
		{
			ele_meter++;
			ele_counter = 0;
		}
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void sendLoggingStats(TinyWebServer& web_server)
{
	web_server << F("gas_meter, ") << gas_meter << F("\n");
	web_server << F("gas_power, ") << gas_power << F("\n");
	web_server << F("ele_meter, ") << ele_meter << F("\n");
	web_server << F("ele_power, ") << ele_power << F("\n");
	web_server << F("temp, ") << temperature << F("\n");
	web_server << F("hum, ") << humidity << F("\n");
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
	if (gas_power > 0)
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
	gas_meter = EEPROM_readlong(0);
	ele_meter = EEPROM_readlong(4);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void getFileName(char * buf, char prefix, DateTime time)
{
	buf[0] = prefix;
	
	String year = String(time.year());
	//Serial << year;
	buf[1] = year.charAt(2);
	buf[2] = year.charAt(3);
	
	String month = String(time.month());
	if (month.length() == 2)
	{
		buf[3] = month.charAt(0);
		buf[4] = month.charAt(1);
	}
	else
	{
		buf[3] = '0';
		buf[4] = month.charAt(0);
	}
	
	String day = String(time.day());
	if (day.length() == 2)
	{
		buf[5] = day.charAt(0);
		buf[6] = day.charAt(1);
	}
	else
	{
		buf[5] = '0';
		buf[6] = day.charAt(0);
	}
	
	buf[7] = '.';
	buf[8] = 't';
	buf[9] = 'x';
	buf[10] = 't';
	buf[11] = '\0';
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void check_newDay()
{
	static byte day = 0;
	
	DateTime time = RTC.now();
	
	if (time.hour() == 0 &&
		time.minute() <= 1 &&
		time.day() != day )
	{
		day = time.day();
		file.open(&root, "meter.txt", FILE_WRITE);
		// if the file is available, write to it:
		if (file.isOpen())
		{
			file.seekEnd();
			
			file << F("gasmeter, ");
			file.print(gas_meter);
			file.print(", ");
			file.print(time.unixtime());
			file.println("");
			
			file << F("elemeter, ");
			file.print(ele_meter);
			file.print(", ");
			file.print(time.unixtime());
			file.println("");
			
			file.close();
			//Serial << F("datalog.txt written");
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
	//Serial.println("+");
	/*if (ele_newImp > 0)
	{
		Serial.print("ele: ");
		Serial.println(ele_newImp);
		ele_newImp = 0;
	}
	if (gas_newImp > 0)
	{
		Serial.print("gas: ");
		Serial.println(gas_newImp);
		gas_newImp = 0;
	}*/
	if (has_filesystem)
	{	
		char filename[12];
		DateTime time = RTC.now();
		
		getFileName(filename,'M',time);
	
		file.open(&root, filename, FILE_WRITE);
		// if the file is available, write to it:
		if (file.isOpen()) 
		{
			file.seekEnd();
			if (gas_newImp)
			{
				file << F("Gas, ");
				file.print(gas_newImp);
				file.print(", ");
				file.print(time.unixtime());
				file.println("");
				gas_newImp = 0;
			}
			if (ele_newImp)
			{
				file << F("Ele, ");
				file.print(ele_newImp);
				file.print(", ");
				file.print(time.unixtime());
				file.println("");
				ele_newImp = 0;
			}
			file.close();
			//Serial << F("datalog.txt written");
		}		
		else 
		{
			// if the file isn't open, pop up an error:
			Serial << F("error opening M");
		}
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
	EEPROM_writelong(0,gas_meter);
	EEPROM_writelong(4,ele_meter);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void logging_Environment()
{
	
	temperature = readTemp();
	humidity = readHum();
	
	if (has_filesystem)
	{
		char filename[12];
		DateTime time = RTC.now();
		
		getFileName(filename,'E',time);
		
		//Serial.println(filename);
			
		file.open(&root, filename, FILE_WRITE);
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
			Serial << F("error writing E");
		}
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void logging_process()
{
	if (gas_newImp + ele_newImp > 0)
	{
		logging_write();
	}
	else if (big_intervall)	// 15 minutes
	{	
		logging_Environment();
		backupMeters();
	
		big_intervall = false;
	}
	else if (med_intervall) // 1 minute
	{
		check_newDay();
		med_intervall = false;
	}
	else if (small_intervall)	// 5 seconds
	{
		if (millis() - last_GasInterrupt > 25000) // over 25 seconds
		{
			gas_power = 0;
		}
		small_intervall = false;
	}
}