//


#include "drv_canfdspi_api.h"
#include "drv_canfdspi_register.h"


//#include "system.h"

#include "CANFD_MCP2518FD.h"
#include "RPI3B_drv_spi.h"

//#include "..//DDE//DDE_func.h"

APP_DATA appData;

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

APP_Payload payload;

uint32_t txId = TX_RESPONSE_ID;
uint32_t jpgTxIdx = 0;
uint32_t jpgRxIdx = 0;
CAN_BITTIME_SETUP selectedBitTime = CAN_1000K_8M;// CAN_1000K_8M;//CAN_1000K_8M;//CAN_500K_2M;
uint8_t ledCount = 0, ledState = 0;

uint8_t i;
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

void value_to_tx_buff(unsigned char*buff,char*name,long  val);

int _counter;
char dev_ID[4];
void CANFD_MCP2528FD_Init()
{	
    // Reset device
    //DRV_CANFDSPI_Reset(_SPI0);
	//while(1){
	_counter =DRV_CANFDSPI_OperationModeGet(_SPI0);
	//TODO - check that after power on in CAN_CONFIGURATION_MODE
    if (_counter != CAN_CONFIGURATION_MODE)
        printf("SPI HW error at CANFD_MCP2528FD_Init ");
    //DRV_CANFDSPI_OperationModeSelect(_SPI0, CAN_CONFIGURATION_MODE);

	// Reset device
	DRV_CANFDSPI_Reset(_SPI0);
    
    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(_SPI0);

    if (!ramInitialized) {
        DRV_CANFDSPI_RamInit(_SPI0, 0xff);
        ramInitialized = true;
    }

    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 0;//1;
    config.StoreInTEF = 0;
    config.BitRateSwitchDisable=1;
    config.TXQueueEnable=1;
    config.RestrictReTxAttempts=1;
    DRV_CANFDSPI_Configure(_SPI0, &config);

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_8; //CAN_PLSIZE_64;
    txConfig.TxPriority = 1;
    txConfig.TxAttempts = 0;//1;
    txConfig.RemoteTREnable = 0;

    DRV_CANFDSPI_TransmitChannelConfigure(_SPI0, APP_TX_FIFO, &txConfig);

    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_8; //CAN_PLSIZE_64;

    DRV_CANFDSPI_ReceiveChannelConfigure(_SPI0, APP_RX_FIFO, &rxConfig);

    // Setup RX Filter
    fObj.word = 0;
    fObj.bF.SID = 0xda;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(_SPI0, CAN_FILTER0, &fObj.bF);

    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0;
    mObj.bF.MIDE = 1; // Only allow standard IDs
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(_SPI0, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(_SPI0, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    DRV_CANFDSPI_BitTimeConfigure(_SPI0, selectedBitTime, CAN_SSP_MODE_OFF, CAN_SYSCLK_40M);//CAN_SYSCLK_40M);
            
    // Setup Transmit and Receive Interrupts
    //DRV_CANFDSPI_GpioModeConfigure(_SPI0, GPIO_MODE_INT, GPIO_MODE_INT);
    DRV_CANFDSPI_TransmitChannelEventEnable(_SPI0, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
    DRV_CANFDSPI_ReceiveChannelEventEnable(_SPI0, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    //DRV_CANFDSPI_ModuleEventEnable(_SPI0, CAN_TX_EVENT | CAN_RX_EVENT);

	CAN_OSC_STATUS status;
	do {
	DRV_CANFDSPI_OscillatorStatusGet(_SPI0,&status);
	_counter =DRV_CANFDSPI_OperationModeGet(_SPI0);
	} while(status.OscReady==0);

	//read device ID


    DRV_CANFDSPI_OperationModeSelect(_SPI0,CAN_CLASSIC_MODE);// CAN_NORMAL_MODE);
//while (1)
    _counter = DRV_CANFDSPI_ReadByte(0,cREGADDR_OSC,&dev_ID[0]); //cREGADDR_DEVID

 	//do {
    //DRV_CANFDSPI_OscillatorStatusGet(_SPI0,&status);
    //_counter++;
    //
	_counter =DRV_CANFDSPI_OperationModeGet(_SPI0);
	//} while(status.SclkReady==0);

	payload.On=true;
	payload.Dlc=CAN_DLC_8;

    // Select Normal Mode
}

void APP_TransmitMessageQueue()
{
   // APP_LED_Set(APP_TX_LED);
	//TODO HANG IN TX - will hang after number of unsuccesfull attampts to transmite- we need to reset CAN or cleaan TX_Error
    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

    // Check if FIFO is not full
    do {

        DRV_CANFDSPI_TransmitChannelEventGet(_SPI0, APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            DRV_CANFDSPI_ErrorCountStateGet(_SPI0, &tec, &rec, &errorFlags);
            return;
        }
        attempts--;
    }
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = DRV_CANFDSPI_DlcToDataBytes(txObj.bF.ctrl.DLC);

    n=DRV_CANFDSPI_TransmitChannelLoad(_SPI0, APP_TX_FIFO, &txObj, txd, n, true);

}

volatile char _error_TX, _error_RX;
int _msg_counter=0;
int CANFD_MCP2528FD_Update()
{
int el_ID;

    /* Check the application's current state. */
    switch (appData.state) {
            /* Application's initial state. */
        case APP_STATE_INIT:
        {

           CANFD.init();
           appData.state = APP_STATE_INIT_TXOBJ;
            break;
        }

            /* Initialize TX Object */
        case APP_STATE_INIT_TXOBJ:
        {

            // Configure transmit message
            txObj.word[0] = 0;
            txObj.word[1] = 0;

            txObj.bF.id.SID = TX_RESPONSE_ID;
            txObj.bF.id.EID = 0;

            txObj.bF.ctrl.BRS = 1;
            txObj.bF.ctrl.DLC = CAN_DLC_8;
            txObj.bF.ctrl.FDF = 0;//1;
            txObj.bF.ctrl.IDE = 0;

            // Configure message data
            int i;
            for (i = 0; i < MAX_DATA_BYTES; i++)
            	txd[i] = txObj.bF.id.SID + i;

            appData.state = APP_STATE_SWITCH_CHANGED;
            break;
        }


            /* Receive a message */
        case APP_STATE_RECEIVE:
        {

            appData.state = APP_ReceiveMessage_Tasks();


            break;
        }

        case APP_STATE_PAYLOAD:
        {

            APP_PayLoad_Tasks();

            DRV_CANFDSPI_ErrorCountTransmitGet( 0, &_error_TX);

            // *****************************************************************************
            //! Receive Error Count Get

            DRV_CANFDSPI_ErrorCountReceiveGet( 0,&_error_RX);
            if (_msg_counter<1000) _msg_counter++;
            if (_msg_counter==1000) {_msg_counter=0;
            printf("_error_TX = %x",_error_TX);
            }
        	//check OSC is stable
        	//CANFDSPI_MODULE_ID index,
        	CAN_OSC_STATUS status;
        	DRV_CANFDSPI_OscillatorStatusGet(_SPI0,&status);

            appData.state = APP_STATE_SWITCH_CHANGED;
            //     appData.state = APP_STATE_RECEIVE;
            break;
        }

            /* Transmit changes in switch states */
        case APP_STATE_SWITCH_CHANGED:
        {
            //if (switchChanged)
        	{


                DRV_CANFDSPI_ErrorCountTransmitGet( 0, &_error_TX);

                // *****************************************************************************
                //! Receive Error Count Get

                DRV_CANFDSPI_ErrorCountReceiveGet( 0,&_error_RX);

                // Transmit new state
                txObj.bF.ctrl.DLC = CAN_DLC_8;
                txObj.bF.ctrl.IDE = 0;
                txObj.bF.ctrl.BRS = 0;// 0;//1;
                txObj.bF.ctrl.FDF = 0;// 0;//was 1;

               // txd[0] = 0xA5;
                //dde.tbl.el[0].value++;
                el_ID=0;
                while (el_ID<1) {

                	switch (el_ID){
                    case 0: txObj.bF.id.SID = M1_ADC_SID + el_ID; value_to_tx_buff(txd, "MAG", 1234); break;// dde.tbl.el[el_ID].value); break;
                		case 1: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ud1",5678);break;
						/*case 2: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ud2",dde.tbl.el[el_ID].value);break;
						case 3: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ubb",dde.tbl.el[el_ID].value);break;
						case 4: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ibb",dde.tbl.el[el_ID].value);break;
						case 5: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Id2",dde.tbl.el[el_ID].value);break;
						case 6: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"IA=",dde.tbl.el[el_ID].value);break;
						case 7: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"IB=",dde.tbl.el[el_ID].value);break;
						case 8: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"IC=",dde.tbl.el[el_ID].value);break;
						case 9: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Uab",dde.tbl.el[el_ID].value);break;
						case 10: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ubc",dde.tbl.el[el_ID].value);break;
						case 11: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Uca",dde.tbl.el[el_ID].value);break;
						case 12: txObj.bF.id.SID = M1_ADC_SID+el_ID; value_to_tx_buff(txd,"Ucn",dde.tbl.el[el_ID].value);break;*/

						//default: el_ID=100;break;
					}

                APP_TransmitMessageQueue();
                el_ID++;
                }
            }

            appData.state = APP_STATE_RECEIVE;

            break;
        }



            /* The default state should never be executed. */
        default:
        {

            appData.state = APP_STATE_INIT;

            break;
        }
    }
}


APP_STATES APP_ReceiveMessage_Tasks()
{
    APP_STATES nextState;
    uint8_t i;

    // Normally we got to APP_STATE_PAYLOAD
    nextState = APP_STATE_PAYLOAD;

    // Check if FIFO is not empty

    DRV_CANFDSPI_ReceiveChannelEventGet(_SPI0, APP_RX_FIFO, &rxFlags);
    if (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT) {

        // Get message
        DRV_CANFDSPI_ReceiveMessageGet(_SPI0, APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);

        switch (rxObj.bF.id.SID) {
            case TX_REQUEST_ID:

                // Check for TX request command

                txObj.bF.id.SID = TX_RESPONSE_ID;

                txObj.bF.ctrl.DLC = rxObj.bF.ctrl.DLC;
                txObj.bF.ctrl.IDE = rxObj.bF.ctrl.IDE;
                txObj.bF.ctrl.BRS = rxObj.bF.ctrl.BRS;
                txObj.bF.ctrl.FDF = rxObj.bF.ctrl.FDF;

                for (i = 0; i < MAX_DATA_BYTES; i++) txd[i] = rxd[i];

                APP_TransmitMessageQueue();
                break;


            case M3_CMD_LOGIC_OUT_SID:
                // Check for Payload command
            	 //IOWR(IO_SLAVE_BASE,0x0202,rxd[1]);
            	 //IOWR(IO_SLAVE_BASE,0x0201,rxd[0]);

//                payload.On = rxd[0];
//                payload.Dlc = rxd[1];
//                if (rxd[2] == 0) payload.ModeRandData = true;
//                else payload.ModeRandData = false;
//                payload.Counter = 0;
//                payload.Delay = rxd[3];
//                payload.BoudRateSwitched = rxd[4];

                break;


        }
    }


    return nextState;
}

void APP_PayLoad_Tasks()
{
    static uint8_t delayCount = 0;
    uint8_t i, n;

    // Send payload?
    if (payload.On) {
        // Delay transmission
        if (delayCount == 0) {
            delayCount = payload.Delay;

            // Prepare data

            txObj.bF.id.SID = M3_CMD_LOGIC_OUT_SID;

            txObj.bF.ctrl.DLC = payload.Dlc;
            txObj.bF.ctrl.IDE = 0;
            txObj.bF.ctrl.BRS = payload.BoudRateSwitched;
            txObj.bF.ctrl.FDF = 0;//1;

            n = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC) payload.Dlc);
          
            for (i = 0; i < n; i++) txd[i] = 0x55;
          

            APP_TransmitMessageQueue();
        } else {
            delayCount--;
        }
    } else {
        delayCount = 0;
    }
}

//void APP_TransmitBitTimeConfig(uint32_t cfg)
//{
//    txObj.bF.id.SID = cfg;
//    txObj.bF.ctrl.IDE = 0;
//    txObj.bF.ctrl.BRS = 0;//1;
//    txObj.bF.ctrl.FDF = 0;//1;
//
//    exit(-1);
//
//    uint8_t i;
//    uint8_t n = 0;
//
//    for (i = 0; i < 64; i++) txd[i] = 0;
//
//    switch (cfg) {
//
//        case BITTIME_CFG_1M_ID:
//            txd[n] = BitTimeConfig1M[0];
//            n++;
//            txd[n] = BitTimeConfig1M[1];
//            n++;
//
//            for (i = 0; i < txd[0]; i++) {
//                txd[n] = BitTimeConfig1M[i + 2] & 0xff;
//                n++;
//                txd[n] = (BitTimeConfig1M[i + 2] >> 8) & 0xff;
//                n++;
//            }
//
//            txObj.bF.ctrl.DLC = DRV_CANFDSPI_DataBytesToDlc(n);
//            APP_TransmitMessageQueue();
//
//            break;
//    }
//}



void value_to_tx_buff(unsigned char*buff,char*name,long val)
{

buff[0] = name[0];
buff[1] = name[1];
buff[2] = name[2];
//buff[3] = name[3];
//val = 122345;
if (val>0xffff) {
	buff[3] = '>';
	buff[4] = 'f';
	buff[5] = 'f';
	buff[6] = 'f';
	buff[7] = 'f';
}
else if (val>=0) sprintf(&buff[3],"%5.5d",val);

else {
	buff[3] = '-';
	buff[4] = '-';
	buff[5] = '-';
	buff[6] = '-';
	buff[7] = '-';
}
//itoa(val,&buff[3], 10); //10 в десятичном формате


}

