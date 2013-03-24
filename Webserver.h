/*
 * webserver.h
 *
 * Created: 22/02/2013 19:13:57
 *  Author: m4rCsi
 */ 
#ifndef WEBSERVER
#define WEBSERVER

#include	"Arduino.h"
#include	<pins_arduino.h>
#include	<SPI.h>
#include	<Ethernet.h>
#include	<Flash.h>
#include	"globe.h"
#include	"TinyWebServer.h"


int			webserver_init();
void		webserver_process();

#endif