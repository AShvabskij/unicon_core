
#include <stdio.h>
#include <cstring>

#include "CAN_MCP2518FD.h"
#include "drv_canfdspi_api.h"
#include "drv_canfdspi_register.h"
#include "RPI3B_SPI/RPI3B_drv_spi.h"

//#include "system.h"

#include "CAN_MCP2518FD.h"
//#include "RPI3B_drv_spi.h"


CAN_CONFIG config;
CAN_OPERATION_MODE opMode;

// Transmit objects
CAN_TX_FIFO_CONFIG txConfig;
CAN_TX_FIFO_EVENT txFlags;
CAN_TX_MSGOBJ txObj;
uint8_t txd[MAX_DATA_BYTES];

// Receive objects
CAN_RX_FIFO_CONFIG rxConfig;
REG_CiFLTOBJ fObj;
REG_CiMASK mObj;
CAN_RX_FIFO_EVENT rxFlags;
CAN_RX_MSGOBJ rxObj;
uint8_t rxd[MAX_DATA_BYTES];

uint32_t delayCount;

REG_t reg;

bool txFromFlash;

bool ramInitialized = false;


//uint32_t txId = TX_RESPONSE_ID;

CAN_BITTIME_SETUP selectedBitTime = CAN_1000K_8M;// CAN_1000K_8M;//CAN_1000K_8M;//CAN_500K_2M;


uint32_t sizeOfTxData = 0;

CAN_BUS_DIAGNOSTIC busDiagnostics;

uint8_t tec;
uint8_t rec;

CAN_ERROR_STATE errorFlags;
bool errorDetected = false;

// Bit time configurations
const uint16_t BitTimeConfig125K[] = {
    1, 0x11, 500
};

const uint16_t BitTimeConfig250K[] = {
    7, 0x08, 500, 833, 1000, 1500, 2000, 3076, 4000
};

const uint16_t BitTimeConfig500K[] = {
    8, 0x00, 1000, 2000, 3076, 4000, 5000, 6666, 8000, 10000
};

const uint16_t BitTimeConfig1M[] = {
    2, 0x0f, 4000, 8000
};
CAN_MCP2518FD::CAN_MCP2518FD()
{
    
}

CAN_MCP2518FD::~CAN_MCP2518FD()
{


}

_dde_func_return_t CAN_MCP2518FD::init()
{
    uint8_t _counter;
    // Reset device
    //DRV_CANFDSPI_Reset(_SPI0);
    // 
    DRV_SPI_Initialize();

    //while(1){
    _counter = DRV_CANFDSPI_OperationModeGet(0);
    //TODO - check that after power on in CAN_CONFIGURATION_MODE
    if (_counter != CAN_CONFIGURATION_MODE)
        printf("SPI HW error at CANFD_MCP2528FD_Init\n ");
    //DRV_CANFDSPI_OperationModeSelect(_SPI0, CAN_CONFIGURATION_MODE);

    // Reset device
    DRV_CANFDSPI_Reset(0);

    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(0);

    if (!ramInitialized) {
        DRV_CANFDSPI_RamInit(0, 0xff);
        ramInitialized = true;
    }

    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 0;//1;
    config.StoreInTEF = 0;
    config.BitRateSwitchDisable = 1;
    config.TXQueueEnable = 1;
    config.RestrictReTxAttempts = 1;
    DRV_CANFDSPI_Configure(0, &config);

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_8; //CAN_PLSIZE_64;
    txConfig.TxPriority = 1;
    txConfig.TxAttempts = 0;//1;
    txConfig.RemoteTREnable = 0;

    DRV_CANFDSPI_TransmitChannelConfigure(0, APP_TX_FIFO, &txConfig);

    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_8; //CAN_PLSIZE_64;

    DRV_CANFDSPI_ReceiveChannelConfigure(0, APP_RX_FIFO, &rxConfig);

    // Setup RX Filter
    fObj.word = 0;
    fObj.bF.SID = 0xda;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(0, CAN_FILTER0, &fObj.bF);

    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0;
    mObj.bF.MIDE = 1; // Only allow standard IDs
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(0, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(0, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    DRV_CANFDSPI_BitTimeConfigure(0, selectedBitTime, CAN_SSP_MODE_OFF, CAN_SYSCLK_40M);//CAN_SYSCLK_40M);

    // Setup Transmit and Receive Interrupts
    //DRV_CANFDSPI_GpioModeConfigure(_SPI0, GPIO_MODE_INT, GPIO_MODE_INT);
    DRV_CANFDSPI_TransmitChannelEventEnable(0, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
    DRV_CANFDSPI_ReceiveChannelEventEnable(0, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    //DRV_CANFDSPI_ModuleEventEnable(_SPI0, CAN_TX_EVENT | CAN_RX_EVENT);

    CAN_OSC_STATUS status;
    do {
        DRV_CANFDSPI_OscillatorStatusGet(0, &status);
        _counter = DRV_CANFDSPI_OperationModeGet(0);
    } while (status.OscReady == 0);

    //read device ID


    DRV_CANFDSPI_OperationModeSelect(0, CAN_CLASSIC_MODE);// CAN_NORMAL_MODE);
//while (1) check dev ID
    uint8_t dev_ID[4];
    _counter = DRV_CANFDSPI_ReadByte(0, cREGADDR_OSC, &dev_ID[0]); //cREGADDR_DEVID

    //do {
    //DRV_CANFDSPI_OscillatorStatusGet(_SPI0,&status);
    //_counter++;
    //
    _counter = DRV_CANFDSPI_OperationModeGet(0);
    //} while(status.SclkReady==0);

    //payload.On = true;
    //payload.Dlc = CAN_DLC_8;

    // Select Normal Mode

}



_dde_func_return_t CAN_MCP2518FD::canPop(CanardFrame* frame)
{

    //! Receive Error Count Get

    DRV_CANFDSPI_ErrorCountReceiveGet(0, &_error_RX);

    DRV_CANFDSPI_ReceiveChannelEventGet(0, APP_RX_FIFO, &rxFlags);
    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {
        // Get message
        DRV_CANFDSPI_ReceiveMessageGet(0, APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);

        return _return_OK;
    }

}

//TODO was 50
#define MAX_TXQUEUE_ATTEMPTS 1
_dde_func_return_t CAN_MCP2518FD::canPush(CanardFrame * frame)
{


    txObj.word[0] = 0;
    txObj.word[1] = 0;

    txObj.bF.id.SID = frame->extended_can_id;
    txObj.bF.id.EID = 0;

    txObj.bF.ctrl.BRS = 1;
    txObj.bF.ctrl.DLC = CAN_DLC_8;// frame->payload_size;// CAN_DLC_8;
    txObj.bF.ctrl.FDF = 0;//1;
    txObj.bF.ctrl.IDE = 0;

    std::memcpy(txd, frame->payload, frame->payload_size);


    //TODO HANG IN TX - will hang after number of unsuccesfull attampts to transmite- we need to reset CAN or cleaan TX_Error
    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

    // Check if FIFO is not full
    do {

        DRV_CANFDSPI_TransmitChannelEventGet(0, APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            DRV_CANFDSPI_ErrorCountStateGet(0, &tec, &rec, &errorFlags);
            return _return_FAIL;
        }
        attempts--;
    } while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = frame->payload_size;// DRV_CANFDSPI_DlcToDataBytes(txObj.bF.ctrl.DLC);

    n = DRV_CANFDSPI_TransmitChannelLoad(0, APP_TX_FIFO, &txObj, txd, n, true);

    DRV_CANFDSPI_ErrorCountTransmitGet(0, &_error_TX);

    return _return_OK;
}