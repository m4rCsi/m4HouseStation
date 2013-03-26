/*
 * logging.h
 *
 * Created: 22/02/2013 19:34:40
 *  Author: m4rCsi
 */
#ifndef LOGGING_H
#define LOGGING_H

#include <Printable.h>
#include <Arduino.h>
#include "Webserver.h"

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

void GasImpulse();
void EleImpulse();
void logging_init();
void logging_process();
void printLoggingStats(Print& to);
void setMeters(char meter, unsigned long value);

extern volatile Meter	ele;
extern volatile Meter	gas;

#endif
