/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */
#include <chrono>
#include <stdint.h>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#include <getopt.h>

//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/types.h>
//#include <linux/spi/spidev.h>

#include <thread>
#include "CRC16.h"


#include "it_logger.h"

#include "RPI3B_SPI.h"
#include "RPI3B_drv_spi.h"
#include "CANFD_MCP2518FD.h"

//RPI3B_SPI*spi;


CANFD_MCP2528FD CANFD = CANFD_MCP2528FD_DEFAULTS;


extern RPI3B_SPI* spi;


void thread_proc_spibus();

int main(int argc, char *argv[])
{
	int ret = 0;

	IT::Logger::setLevel(IT::Logger::INFO);
	LOG(INFO, "RPI3B_SPI bridge started");

	//parse_opts(argc, argv);

	//spi = new RPI3B_SPI();

	//spi->Init(0);
	DRV_SPI_Initialize();

	
	//CAN_MCP2518FD* can_mcp2518fd = new CAN_MCP2518FD();
	CANFD.init();



	//can =new CAN_MCP2518FD(spi0)
	//	
	//can_init->Init(12);


	//can = new CAN_ON_SOCKET(10);
	//can->init(10);



	//using namespace std;
	// Create thread for modbus 
	std::thread thr_spibus(thread_proc_spibus);

	thr_spibus.join();


	getchar();
	return ret;
}


static uint8_t TxBuff[51 * 2 * 8] = { 0 }; //(1status + 48ch)
static uint8_t RxBuff[51 * 2 * 8] = { 0 }; //(1status + 48ch)
static uint16_t buff[51 * 400] = { 0 };

static float fRxBuff[50 * 2 * 8] = { 0 }; //(1status + 48ch)
static float fscale[4] = { 0.6,100.4,22.01,0.001 };


void thread_proc_spibus()
{
	bool buff_empty = false;
	printf("TEST SPI MCP2515\n");
	uint16_t buff_read_counter = 0;
	uint16_t buff_pointer = 0;
	
	while (1)
	{
		auto begin = std::chrono::steady_clock::now();

		CANFD.update();

		std::this_thread::sleep_for(std::chrono::milliseconds(1)); //
		//std::cout << "DONE" << std::endl;
	}

}



//static uint8_t TxBuff[51*2*8] = {0}; //(1status + 48ch)
//static uint8_t RxBuff[51*2*8] = { 0 }; //(1status + 48ch)
//static uint16_t buff[51 * 400] = { 0 }; 
//
//static float fRxBuff[50 * 2 * 8] = { 0 }; //(1status + 48ch)
//static float fscale[4] = { 0.6,100.4,22.01,0.001 };
//void thread_proc_spibus()
//{
//	bool buff_empty = false;
//	printf("TEST SPI\n");
//	uint16_t buff_read_counter = 0;
//	uint16_t buff_pointer = 0;
//	while (1)
//	{
//		auto begin = std::chrono::steady_clock::now();
//		
//		TxBuff[0] = 0xE0;
//		TxBuff[1] = 0x00;
//
//		//TxBuff[2] = 0x5A;
//		printf("MGX  005A 0000 ");
//		for (int ii = 0; ii < 48; ii++)
//			printf("CH%.2d ", ii);
//		printf("\n");
//
//		buff_read_counter = 0;
//		buff_empty = false;
//		while (!buff_empty) 
//		{
//			spi->SendAndRecieve(TxBuff, RxBuff, sizeof(TxBuff));
//			
//			for (int zz = 0; zz < 8; zz++) {
//				if ((RxBuff[51 * zz + 2] == 0x1) && (RxBuff[51 * zz + 3] == 0x5A)) {
//					buff_empty = true;
//					break;
//				}
//				buff_pointer++;
//			}
//			
//			for (int zz = 0; zz < (sizeof(TxBuff) >> 1); zz++)
//				buff[zz] = (RxBuff[2 * zz + 1] << 8) | RxBuff[2 * zz];
//
//
//			/*for (int zz = 0; zz < (50 * 2 * 8); zz++)
//				fRxBuff[zz] = fscale[zz&0x3] * RxBuff[zz];*/
//
//
//			if (RxBuff[2] & 0x01 != 0) {}
//			//	buff_empty = true;
//			//	break;
//			//}
//			//else
//			buff_read_counter++;
//
//		//	if (buff_read_counter > 10)
//		//		break;
//		}
//
//
//		auto end = std::chrono::steady_clock::now();
//		auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
//		double us = elapsed_us.count();
//		
//		printf("Rx0Rx1 = %.2x%.2x\n", RxBuff[0], RxBuff[1]);
//
//		for (int cc = 0; cc < 7; cc++) {
//			for (int ii = cc*51; ii < cc*51+51; ii++)
//				printf("%.2x%.2x ", RxBuff[2 * ii+2], RxBuff[2 * ii + 3]); //skip1 it is 0xfff
//			printf("\n");
//		}
//
//		std::this_thread::sleep_for(std::chrono::milliseconds(1)); //
//		//std::cout << "DONE" << std::endl;
//	}	
//
//}





