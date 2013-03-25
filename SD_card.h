/*
 * sdcard.h
 *
 * Created: 25/03/2013 21:19:32
 *  Author: m4rCsi
 */ 


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Printable.h>

void print_list(Print& printto);
int sd_init();
int readIni();


#endif /* SDCARD_H_ */