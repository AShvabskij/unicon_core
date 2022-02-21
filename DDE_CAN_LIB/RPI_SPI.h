#pragma once

#include "DDE/DDE_TYPES.h"
#include "interface_SPI.h"

class RPI_SPI :public interface_SPI
{
public:

    RPI_SPI(int spi_channel);
    virtual ~RPI_SPI();

    virtual uint8_t init(int spi_mode);

    virtual uint8_t read(char*pBuff, uint16_t length);
    virtual uint8_t write(char* pBuff, uint16_t length);
    
protected: // Protected members are accessible in the class that defines them and in classes that inherit from that class.
    //IDDE_PARAMS* m_params;
    //IDDE_OSC* m_osc;
    //IDDE_OSC* m_mvcp;
    //IDDE_EVLOG* m_evlog;

};

