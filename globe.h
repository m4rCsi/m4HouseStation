/*
 * globe.h
 *
 * Created: 22/02/2013 19:53:12
 *  Author: m4rCsi
 */ 
#ifndef		GLOBE_H
#define		GLOBE_H

#include	"RTClib.h"
#include	"Sd2Card.h"
#include	"SdFat.h"

// RTC
extern RTC_DS1307	RTC;

// SD
extern boolean has_filesystem;
extern Sd2Card card;
extern SdVolume volume;
extern SdFile root;
extern SdFile file;

#endif