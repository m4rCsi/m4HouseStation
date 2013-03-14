/*
 * logging.h
 *
 * Created: 22/02/2013 19:34:40
 *  Author: m4rCsi
 */
#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include "Webserver.h"


void GasImpulse();
void EleImpulse();
void logging_init();
void logging_process();
void sendLoggingStats(TinyWebServer& web_server);
void setMeters(char meter, unsigned long value);

#endif
