#pragma once

#include <string>
#include <stdint.h>


using namespace std;


// Index to SPI channel
// Used when multiple MCP25xxFD are connected to the same SPI interface, but with different CS
#define _SPI0                        0
#define DRV_CANFDSPI_INDEX_1         1

class RPI3B_SPI
{
private:
    const string SPI_PORT_NAME = "SPI0";
    int fd;
public:

	RPI3B_SPI();
	~RPI3B_SPI();

            //SpiDevice OurSpiPort;
            bool InitDone;
            uint32_t freqVal;

            int8_t Init(int ChipSelectId);

           // uint8_t SendAndRecieve(uint8_t cuint8_t, int RxLen);

            int8_t SendAndRecieve(uint8_t* TxBuffer, uint8_t* RxBuffer, uint16_t TxLen);


            bool SendCommand(uint8_t* TxBuffer);
            bool SendCommand(uint8_t cByte);
            bool SendIoCommand(uint8_t Address, uint8_t cByte);
            bool SendIoCommand(uint8_t Address, uint16_t cWord);
            bool SendIoCommand(uint8_t Address, uint32_t cWord);
            bool SendIoCommand(uint8_t Address, uint8_t* DataBuffer);
            bool SendIoCommand(uint8_t Address, uint64_t cData, int NoOfRxBytes);

            //----------------------------------------------------------------------------
            // IO Get Functions
            uint8_t GetIoReg8(uint8_t Address);
            uint16_t GetIoReg16(uint8_t Address);
            uint32_t GetIoReg24(uint8_t Address);
            uint32_t GetIoReg32(uint8_t Address);
            bool SetSpiBit(uint8_t Address, uint8_t bitNo, bool bitVal);
            bool GetSpiBit(uint8_t Address, uint8_t bitNo);
            void SetSpiPulse(uint8_t Address, uint8_t bitNo);



};

