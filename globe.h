/*
 * globe.h
 *
 * Created: 22/02/2013 19:53:12
 *  Author: m4rCsi
 */ 
#ifndef		GLOBE_H
#define		GLOBE_H

#include	"RTClib.h"

// pin 4 is the SPI select pin for the SDcard
#define SD_CS 4
// pin 10 is the SPI select pin for the Ethernet
#define ETHER_CS 10

// RTC
extern RTC_DS1307	RTC;

//DEBUG
#ifdef DEBUGM4
	extern int freeRam ();
#endif

#endif