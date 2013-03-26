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


void GasImpulse();
void EleImpulse();
void logging_init();
void logging_process();
void sendLoggingStats(Print& to);
void setMeters(char meter, unsigned long value);

#endif
