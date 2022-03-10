/*******************************************************************************
 Simple SPI Transfer function
 for NIOS

 Author A&D
*/

#include "RPI3B_drv_spi.h"

//#include <system.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "RPI3B_SPI.h"


static RPI3B_SPI* spi;



//#include "system_config.h"
//#include "system_definitions.h"

// NO need for chip select as it is done in HW. Left for compatibility only
//int8_t DRV_SPI_ChipSelectAssert(uint8_t spiSlaveDeviceIndex, bool assert)
//{
//    int8_t error = 0;
//// Select Chip Select
////    switch (spiSlaveDeviceIndex) {
////        case DRV_CANFDSPI_INDEX_0:
////            if (assert) SYS_PORTS_PinClear(PORTS_ID_0, SPI_CS0_PORT_ID, SPI_CS0_PORT_PIN);
////            else SYS_PORTS_PinSet(PORTS_ID_0, SPI_CS0_PORT_ID, SPI_CS0_PORT_PIN);
////            break;
////        case DRV_CANFDSPI_INDEX_1:
////            if (assert) SYS_PORTS_PinClear(PORTS_ID_0, SPI_CS1_PORT_ID, SPI_CS1_PORT_PIN);
////            else SYS_PORTS_PinSet(PORTS_ID_0, SPI_CS1_PORT_ID, SPI_CS1_PORT_PIN);
////            break;
////        default:
////            error = -1;
////            break;
////    }
//
//    return error;
//}




void DRV_SPI_Initialize()
{

    spi = new RPI3B_SPI();
    spi->Init(0);

    return;
}

int8_t DRV_SPI_TransferData(uint8_t spiSlaveDeviceIndex, uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize)
{
    int8_t error = 0;
    int8_t return_code=0;
    if (spiTransferSize > SPI_DEFAULT_BUFFER_LENGTH) perror("spiTransferSize>SPI_DEFAULT_BUFFER_LENGTH");
    
    error = spi->SendAndRecieve(SpiTxData, SpiRxData, spiTransferSize);
    
    return error;


//    bool continueLoop;
//    uint16_t txcounter = 0;
//    uint16_t rxcounter = 0;
//    uint8_t unitsTxed = 0;
//    const uint8_t maxUnits = 16;
//
//    // Assert CS
//    error = HAL_SPI_ChipSelectAssert(spiSlaveDeviceIndex, true);
//    if (error != 0) return error;
//
//    // Loop until spiTransferSize
//    do {
//        continueLoop = false;
//        unitsTxed = 0;
//
//        // Fill transmit FIFO
//        if (PLIB_SPI_TransmitBufferIsEmpty(SPI_ID_1)) {
//            while (!PLIB_SPI_TransmitBufferIsFull(SPI_ID_1) && (txcounter < spiTransferSize) && unitsTxed != maxUnits) {
//                PLIB_SPI_BufferWrite(SPI_ID_1, SpiTxData[txcounter]);
//                txcounter++;
//                continueLoop = true;
//                unitsTxed++;
//            }
//        }
//
//        // Read as many bytes as were queued for transmission
//        while (txcounter != rxcounter) {
//            while (PLIB_SPI_ReceiverFIFOIsEmpty(SPI_ID_1));
//            SpiRxData[rxcounter] = PLIB_SPI_BufferRead(SPI_ID_1);
//            rxcounter++;
//            continueLoop = true;
//        }
//
//        // Make sure data gets transmitted even if buffer wasn't empty when we started out with
//        if ((txcounter > rxcounter) || (txcounter < spiTransferSize)) {
//            continueLoop = true;
//        }
//
//    } while (continueLoop);
//
//    // De-assert CS
//    error = HAL_SPI_ChipSelectAssert(spiSlaveDeviceIndex, false);


//    This function performs a control sequence on the SPI bus. It supports only SPI masters with data
//    width less than or equal to 8 bits. A single call to this function writes a data buffer of arbitrary
//    length to the mosi port, and then reads back an arbitrary amount of data from the miso port. The
//    function performs the following actions:
//    (1) Asserts the slave select output for the specified slave. The first slave select output is 0.
//    (2) Transmits write_length bytes of data from wdata through the SPI interface, discarding the
//    incoming data on the miso port.
//    (3) Reads read_length bytes of data and stores the data into the buffer pointed to by read_data.
//    The mosi port is set to zero during the read transaction.
//    (4) De-asserts the slave select output, unless the flags field contains the value
//    ALT_AVALON_SPI_COMMAND_MERGE. If you want to transmit from scattered buffers, call the
//    function multiple times and specify the merge flag on all the accesses except the last.
//    To access the SPI bus from more than one thread, you must use a semaphore or mutex to ensure
//    that only one thread is executing within this function at any time.
//	retruns The number of bytes stored in the read_data buffer.

 
}

//volatile int _counter;
//int my_avalon_spi_command(alt_u32 base, alt_u32 slave,
//                           alt_u32 write_length, const alt_u8 * write_data,
//                           alt_u32 read_length, alt_u8 * read_data,
//                           alt_u32 flags)
//{
//  const alt_u8 * write_end = write_data + write_length;
//  alt_u8 * read_end = read_data + read_length;
//
//  alt_u32 write_zeros = read_length;
//  alt_u32 read_ignore = 0;//write_length;
//  alt_u32 status;
//
//  /* We must not send more than two bytes to the target before it has
//   * returned any as otherwise it will overflow. */
//  /* Unfortunately the hardware does not seem to work with credits > 1,
//   * leave it at 1 for now. */
//  alt_32 credits = 1;
//
//  /* Warning: this function is not currently safe if called in a multi-threaded
//   * environment, something above must perform locking to make it safe if more
//   * than one thread intends to use it.
//   */
//
//  IOWR_ALTERA_AVALON_SPI_SLAVE_SEL(base, 1 << slave);
//
//  /* Set the SSO bit (force chipselect) only if the toggle flag is not set */
//  //if ((flags & ALT_AVALON_SPI_COMMAND_TOGGLE_SS_N) == 0)
//    IOWR_ALTERA_AVALON_SPI_CONTROL(base, ALTERA_AVALON_SPI_CONTROL_SSO_MSK);
//
//
//  /*
//   * Discard any stale data present in the RXDATA register, in case
//   * previous communication was interrupted and stale data was left
//   * behind.
//   */
//  IORD_ALTERA_AVALON_SPI_RXDATA(base);
//
//  /* Keep clocking until all the data has been processed. */
//  for ( ; ; )
//  {
//	 _counter=0;
//    do
//    {
//      status = IORD_ALTERA_AVALON_SPI_STATUS(base);
//      _counter++;
//    }
//    while (((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0 || credits == 0) &&
//            (status & ALTERA_AVALON_SPI_STATUS_RRDY_MSK) == 0);
//
//    if ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) != 0 && credits > 0)
//	 {
//      credits--;
//
//      if (write_data < write_end)
//        IOWR_ALTERA_AVALON_SPI_TXDATA(base, *write_data++);
//      else if (write_zeros > 0)
//      {
//        write_zeros--;
//        IOWR_ALTERA_AVALON_SPI_TXDATA(base, 0);
//      }
//      else
//        credits = -1024;
//    };
//
//    if ((status & ALTERA_AVALON_SPI_STATUS_RRDY_MSK) != 0)
//    {
//      alt_u32 rxdata = IORD_ALTERA_AVALON_SPI_RXDATA(base);
//
//      if (read_ignore > 0)
//        read_ignore--;
//      else
//        *read_data++ = (alt_u8)rxdata;
//      credits++;
//
//      if (read_ignore == 0 && read_data == read_end)
//        break;
//    }
//
//  }
////  _counter=0;
//  /* Wait until the interface has finished transmitting */
////  do
////  {
////    status = IORD_ALTERA_AVALON_SPI_STATUS(base);
////    _counter++;
////  }
////  while ((status & ALTERA_AVALON_SPI_STATUS_TMT_MSK) == 0);
//
//  /* Clear SSO (release chipselect) unless the caller is going to
//   * keep using this chip
//   */
//  //if ((flags & ALT_AVALON_SPI_COMMAND_MERGE) == 0)
//    IOWR_ALTERA_AVALON_SPI_CONTROL(base, 0);
//
//  return read_length;
//}
