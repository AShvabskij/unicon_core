#pragma once
class interface_SPI
{
public:

	virtual ~interface_SPI() {};
    
    virtual uint8_t init(int spi_mode) = 0;
    virtual uint8_t SendAndRecieve(uint8_t* TxBuffer, uint8_t* RxBuffer, uint16_t TxLen) = 0;
    //virtual _dde_func_return_t write(char* pBuff, uint16_t length) = 0;

};

