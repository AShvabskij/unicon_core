#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "RPI3B_SPI.h"

#define DEF_AWAIT 
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


static const char* device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 8000000; // 250000000 / 4; //50MHZ 
static uint16_t delay = 1;



RPI3B_SPI::RPI3B_SPI()
{
    //static SpiDevice OurSpiPort;
    InitDone = false;
    //public static uint32_t freqVal = (uint32_t)14e6; // 12.5 Mhz was ok. 16.0 Mhz limit due to Raspberry PI read speed 
    //static uint32_t freqVal = (uint32_t)5e6; // used for debug only
    //private const string SPI_PORT_NAME = "SPI0";
}
RPI3B_SPI:: ~RPI3B_SPI()
{
   // close(fd);
}


//-------------------------------------------------------------------------------------------------------------------------------------

static void pabort(const char* s)
{
    perror(s);
    abort();
}
 

int8_t RPI3B_SPI::Init(int ChipSelectId)
{
//try
//{
//    var settings = new SpiConnectionSettings(ChipSelectId);  // Create SPI initialization settings
//    {
//        settings.ClockFrequency = (int)freqVal;
//        settings.Mode = SpiMode.Mode0; //SpiMode.Mode3;
//       // settings.DataBitLength - RPI supports only 8
//    }

//    string spiAqs = SpiDevice.GetDeviceSelector(SPI_PORT_NAME);
//    var devicesInfo = DEF_AWAIT DeviceInformation.FindAllAsync(spiAqs);
//    OurSpiPort = DEF_AWAIT SpiDevice.FromIdAsync(devicesInfo[0].Id, settings);
//}
//catch (Exception e)
//{
//    System.Diagnostics.Debug.WriteLine("SPI Initialisation Error", e.Message);
//}
//finally
//{
//    InitialiseComplete = true;
//}

    int ret = 0;


    fd = open(device, O_RDWR);
    if (fd < 0) {
        pabort("can't open device");
    }

    /*
     * spi mode
     */
    mode = SPI_MODE_0;
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get d spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    //printf("spi mode: %d\n", mode);
    //printf("bits per word: %d\n", bits);
    //printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

    InitDone = true;

return 0;

}



//---------------------------------------------------------------------------------------------------------------------------------------




int8_t RPI3B_SPI::SendAndRecieve(uint8_t* TxBuffer, uint8_t* RxBuffer, uint16_t TxLen)
{
    //uint8_t RxBuffer[RxLen];
    if (InitDone)
    {
        int ret;
        int crc16_tx, crc16_rx;
       // uint8_t tx[(2 + 24) * 2];

//        std::srand(std::time(nullptr));
//        tx[0] = 0x01; tx[1] = 0x02; tx[2] = 0x03; tx[3] = 0x04;

        //for (int ii = 4; ii < sizeof(tx); ii++)
        //    tx[ii] = std::rand() & 0xff;

        //CRC16 crc16;
        //crc16_tx = crc16.Calc(tx, sizeof(tx));



        //uint8_t rx[ARRAY_SIZE(tx)] = { 0, };
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)TxBuffer,
            .rx_buf = (unsigned long)RxBuffer,
            .len = TxLen ,
            .speed_hz = speed,
            .delay_usecs = delay,
            .bits_per_word = bits,
        };



        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if (ret < 1) {
            pabort("can't send spi message");
            return -1;
        }
        else
            return 0;
        //crc16_rx = crc16.Calc(rx, sizeof(rx));


        //if (crc16_tx != crc16_rx) {
        //    ret = 100;
            //printf("SPI CRC16 error\n");
            //pabort("can't send spi message");
       // }


    }
    else
    return -1;

}

//uint8_t RPI3B_SPI::SendAndRecieve(uint8_t cuint8_t, int RxLen)
//{
//    //uint8_t TxBuffer[1];
//    //TxBuffer[0] = cuint8_t;
//
//    //uint8_t RxBuffer[RxLen];
//    //if (InitDone)
//    //{
//    //    try
//    //    {
//    //        OurSpiPort.TransferSequential(TxBuffer, RxBuffer);
//    //    }
//    //    catch (Exception e)
//    //    {
//    //        System.Diagnostics.Debug.WriteLine("Exception: {0}", e.Message);
//    //    }
//    //}
//    //return (RxBuffer);
//}

bool  RPI3B_SPI::SendCommand(uint8_t*TxBuffer)
{
    bool ok = false;
    if (InitDone)
    {
        //try
        //{
        //  //  OurSpiPort.Write(TxBuffer);

        //    ok = true;
        //}
        //catch (Exception e)
        //{
        //System.Diagnostics.Debug.WriteLine("Exception: {0}", e.Message);
        //}
    }
    return (ok);
}

bool RPI3B_SPI::SendCommand(uint8_t cuint8_t)
{
    uint8_t TxBuffer[1];
    TxBuffer[0] = cuint8_t;
    bool ok = DEF_AWAIT SendCommand(TxBuffer);
    return ok;
}

bool RPI3B_SPI::SendIoCommand(uint8_t Address, uint8_t cuint8_t)
{
    uint8_t TxBuffer[3];
    TxBuffer[0] = 0xC0;
    TxBuffer[1] = Address;
    TxBuffer[2] = cuint8_t;
    bool ok = DEF_AWAIT SendCommand(TxBuffer);
    return ok;
}

bool RPI3B_SPI::SendIoCommand(uint8_t Address, uint16_t cWord)
{
    uint8_t TxBuffer[4];
    TxBuffer[0] = 0xC0;
    TxBuffer[1] = Address;
    TxBuffer[2] = (uint8_t)(cWord>>8);
    TxBuffer[3] = (uint8_t)cWord;
    bool ok = DEF_AWAIT SendCommand(TxBuffer);
    return ok;
}

bool RPI3B_SPI::SendIoCommand(uint8_t Address, uint32_t cWord)
{
    uint8_t TxBuffer[6];
    TxBuffer[0] = 0xC0;
    TxBuffer[1] = Address;
    TxBuffer[2] = (uint8_t)(cWord >> 24);
    TxBuffer[3] = (uint8_t)(cWord >> 16);
    TxBuffer[4] = (uint8_t)(cWord >> 8);
    TxBuffer[5] = (uint8_t)cWord;
    bool ok = DEF_AWAIT SendCommand(TxBuffer);
    return ok;
}

bool RPI3B_SPI::SendIoCommand(uint8_t Address, uint8_t* DataBuffer)
{
    //uint8_t[] TxBuffer = new uint8_t[DataBuffer.Length + 2];
    //TxBuffer[0] = 0xC0;
    //TxBuffer[1] = Address;
    //for (int i = 2; i   TxBuffer.Length; i++) TxBuffer[i] = DataBuffer[i - 2];
    //bool ok = DEF_AWAIT Spi.SendCommand(TxBuffer);
   //return ok;
}

bool RPI3B_SPI::SendIoCommand(uint8_t Address, uint64_t cData, int NoOfRxuint8_ts)
{
//    uint8_t[] TxBuffer = new uint8_t[8 + 2];
//    TxBuffer[0] = 0xC0;
//    TxBuffer[1] = Address;
//
//    uint8_t[] DataBuffer = Util.uint64_tTouint8_tArray(cData, NoOfRxuint8_ts);
//    bool ok = DEF_AWAIT SendIoCommand(Address, TxBuffer);
//    return ok;
}

        //----------------------------------------------------------------------------
        // IO Get Functions

uint8_t RPI3B_SPI::GetIoReg8(uint8_t Address)
{
    uint8_t RxBuffer[4];
    uint8_t b[2];
    b[0] = 0xD0;  // Opcode for Read
    b[1] = Address;
    bool res = DEF_AWAIT SendAndRecieve(b, RxBuffer, 2);
    uint8_t cVal = RxBuffer[1];
    return (cVal);
}

uint16_t RPI3B_SPI::GetIoReg16(uint8_t Address)
{
    uint8_t RxBuffer[4];
    uint8_t b[2];
    b[0] = 0xD0;  // Opcode for Read
    b[1] = Address;
    bool res = DEF_AWAIT SendAndRecieve(b, RxBuffer, 2);
    uint16_t cVal = (uint16_t)(256 * RxBuffer[0] + RxBuffer[1]);
    return (cVal);
}

uint32_t RPI3B_SPI::GetIoReg24(uint8_t Address)
{   
    uint8_t RxBuffer[4];
    uint8_t b[2];
    b[0] = 0xD0;  // Opcode for Read
    b[1] = Address;
    bool res = DEF_AWAIT SendAndRecieve(b, RxBuffer, 4);
    uint32_t cVal = (uint32_t)((uint32_t)RxBuffer[3] + ((uint32_t)256 * RxBuffer[2]) + ((uint32_t)65536 * RxBuffer[1]));
    return (cVal);
}

uint32_t RPI3B_SPI::GetIoReg32(uint8_t Address)
{
    uint8_t RxBuffer[4];
    uint8_t b[2];
    b[0] = 0xD0;  // Opcode for Read
    b[1] = Address;
    bool res = DEF_AWAIT SendAndRecieve(b, RxBuffer, 4);
    uint32_t cVal = (uint32_t)((uint32_t)RxBuffer[3] + ((uint32_t)256 * RxBuffer[2]) + ((uint32_t)65536 * RxBuffer[1]) + ((uint32_t)(65536 * 256) * RxBuffer[0]));
    return (cVal);
}

        //----------------------------------------------------------------------------

bool RPI3B_SPI::SetSpiBit(uint8_t Address, uint8_t bitNo, bool bitVal)
{
   // uint16_t cData = DEF_AWAIT IO.GetUIn16(Address);
    //Util.SetBitU16(ref cData, bitNo, bitVal);
    //bool isOk = DEF_AWAIT IO.SetUIn16(Address, cData, true);
   // return isOk;
}

bool RPI3B_SPI::GetSpiBit(uint8_t Address, uint8_t bitNo)
{
    bool cBit = false;
    //uint8_t[] b = new uint8_t[2];
    //b[0] = 0xD0;  // Opcode for Read
    //b[1] = Address;
    //var RxBuffer = DEF_AWAIT SendAndRecieve(b, 2);
    //uint16_t cVal = (uint16_t)(256 * RxBuffer[0] + RxBuffer[1]);
    ////cBit = Util.BitVal(cVal, bitNo);
    return cBit;
}

void RPI3B_SPI::SetSpiPulse(uint8_t Address, uint8_t bitNo)
{
    //DEF_AWAIT Spi.SetSpiBit(Address, bitNo, false);
    //DEF_AWAIT Spi.SetSpiBit(Address, bitNo, true);
    //DEF_AWAIT Spi.SetSpiBit(Address, bitNo, false);
}
