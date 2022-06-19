/*
 * Title:			AGON MOS - SPI
 * Author:			Cocoacrumbs
 * Modified by:		Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	19/06/2022
 *
 * Modinfo:
 */

#ifndef SPI_H
#define SPI_H

void init_hw();
void mode_spi(int d);

BYTE spi_transfer(BYTE d);

void SD_CS_enable();
void SD_CS_disable();


#endif SPI_H
