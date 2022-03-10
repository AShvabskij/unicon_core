#pragma once

/*******************************************************************************
 Simple SPI Transfer function

  File Name:
    drv_spi.h

  Summary:
    .

  Description:
    .

  Remarks:

 *******************************************************************************/


#include <stdbool.h>
#include <stdint.h>


//#include <string.h>


// Index to SPI channel
// Used when multiple MCP25xxFD are connected to the same SPI interface, but with different CS
#define _SPI0                        0
#define DRV_CANFDSPI_INDEX_1         1

#define SPI_DEFAULT_BUFFER_LENGTH   128
// *****************************************************************************
// *****************************************************************************
// Section: Variables



void DRV_SPI_Initialize();

//! SPI Read/Write Transfer

int8_t DRV_SPI_TransferData(uint8_t spiSlaveDeviceIndex, uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t XferSize);

//! SPI Chip Select assert/de-assert

//int8_t HAL_SPI_ChipSelectAssert(uint8_t spiSlaveDeviceIndex, bool assert);

//========================================================//
