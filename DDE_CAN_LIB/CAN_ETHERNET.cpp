#include <net/if.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <thread>

#include "CAN_ETHERNET.h"
#include "CAN_layer/socketcan.h"

#define PORT 50502
CAN_ETHERNET::CAN_ETHERNET(uint32_t IP4, uint16_t port)
{

}

CAN_ETHERNET::~CAN_ETHERNET()
{
}

long CAN_ETHERNET::init(CAN_ETHERNET* obj)
{

}

long CAN_ETHERNET::init()
{ 

	// Configure the transport by reading the appropriate standard registers.
	//uavcan_register_Value_1_0_select_natural16_(&val);
	//val.natural16.value.count = 1;
	//val.natural16.value.elements[0] = CANARD_MTU_CAN_FD;
	//registerRead("uavcan.can.mtu", &val);
	//assert(uavcan_register_Value_1_0_is_natural16_(&val) && (val.natural16.value.count == 1));
	auto state_canard_mtu_bytes = CANARD_MTU_CAN_FD;// val.natural16.value.elements[0];
	// We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
	// shall define "uavcan.can.bitrate" of type natural32[2]; the second value is zero/ignored if CAN FD not supported.
	sock = socketcanOpen("vcan0", state_canard_mtu_bytes > CANARD_MTU_CAN_CLASSIC);
	if (sock < 0)
	{
		return -sock;
	}

	uint8_t mode = 0;
	//std::thread thr_server(&CAN_ETHERNET::thread_server_proc, this, mode);

	//std::thread thr_client(&CAN_ETHERNET::thread_client_proc, this, mode);

	//thr_server.join();
	//thr_client.join();




	return _return_OK;
}
/**
 * Spawns n threads
// */
//void spawnThreads(int n)
//{
//	std::vector<thread> threads(n);
//	// spawn n threads:
//	for (int i = 0; i < n; i++) {
//		threads[i] = thread(doSomething, i + 1);
//	}
//
//	for (auto& th : threads) {
//		th.join();
//	}
//}



//-----------------------------------------------------------------------------------

int CAN_ETHERNET::thread_server_proc(int mode)
{
	init_server(0);
	return _return_OK;
}


int CAN_ETHERNET::thread_client_proc(int mode)
{
	init_client(mode);
}

_dde_func_return_t CAN_ETHERNET::init_server(int mode) {


}

_dde_func_return_t CAN_ETHERNET::init_client(int mode) {}

_dde_func_return_t CAN_ETHERNET::canPop(CanardFrame* frame) {

	static uint8_t     buffer[64]; //!!! this should be static!!!

	uint8_t res = socketcanPop(sock, frame, sizeof(buffer), buffer, 1000, NULL);
	//if (res > 0) print_frame(frame);

	return res;




	// Process received frames by feeding them from SocketCAN to libcanard.
// You can service an arbitrary number of redundant interfaces here, libcanard supports that!

		const uint16_t         max_frames_to_process_per_iteration = 1000;
	const CanardMicrosecond loop_resolution = 100;
	//CanardFrame frame = { 0 };
	uint8_t     buf[CANARD_MTU_CAN_FD] = { 0 };
	//for (uint16_t i = 0; i < max_frames_to_process_per_iteration; ++i)
	{
		const int16_t socketcan_result = socketcanPop(sock, frame, sizeof(buf), buf, loop_resolution, NULL);
		if (socketcan_result == 0)  // The read operation has timed out with no frames, nothing to do here.
		{
			//break;
		}
		if (socketcan_result < 0)  // The read operation has failed. This is not a normal condition.
		{
			return _return_FAIL;
		}
	}
	
return _return_OK;
}

_dde_func_return_t CAN_ETHERNET::canPush(CanardFrame * frame) 
{

	// Transmit pending frames from the prioritized TX queue managed by libcanard.
		   // You can service an arbitrary number of redundant interfaces here!
	//{
		//const CanardFrame* frame = canardTxPeek(&state.canard);  // Take the highest-priority frame from TX queue.
		//while (frame != NULL)
		//{
			// Attempt transmission only if the frame is not yet timed out while waiting in the TX queue.
			// Otherwise just drop it and move on to the next one.
			//if ((frame->timestamp_usec == 0) || (frame->timestamp_usec > monotonic_time))
			{
				const int16_t result = socketcanPush(sock, frame, 0);  // Non-blocking write attempt.
				if (result == 0)
				{
					//break;  // The queue is full, we will try again on the next iteration.
				}
				if (result < 0)
				{
					return _return_FAIL;  // SocketCAN interface failure (link down?)
				}
			}
		//	canardTxPop(&state.canard);
		//	state.canard.memory_free(&state.canard, (void*)frame);
		//	frame = canardTxPeek(&state.canard); take next frame
		//}
	//}

	return _return_OK;
}



// old revision with direct socket. Was replaced with linux based VCAN virtual can
/*

_dde_func_return_t CAN_ETHERNET::init_server(int mode)
{
	struct sockaddr_in addr; // структура с адресом
	int new_socket;
	int res;

	int master_socket =  socket(AF_INET, SOCK_STREAM, 0);
	
	if (master_socket == 0) {perror("socket failed");exit(EXIT_FAILURE);}
	
	int opt = 1;
	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {perror("setsockopt");exit(EXIT_FAILURE);}

	struct timeval tv;
	tv.tv_sec = 0;  // 0 Secs Timeout 
	tv.tv_usec = 10000; // 10ms timeout 
	res=setsockopt(master_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
	if (res < 0) {perror("setsockopt"); exit(EXIT_FAILURE); }

	//type of socket created
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	auto addrlen = sizeof(addr);
	puts("Waiting for connections ...");


	new_socket = accept(master_socket, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
	if (new_socket < 0) { perror("accept"); exit(EXIT_FAILURE); }
	puts("connected +1\n");

	char sendBuff[] = "data from server";
	char recvBuff[100];

	//simulate data exchange CAN 
	//CAN_MSG can_msg_tx;
	//CAN_MSG can_msg_rx;

	//while (1)
	//{
	//	// procees request
	//	res=uavcan_master->proceed_req(can_msg_tx);
	//	if(res!=0)
	//	write(new_socket, sendBuff, strlen(sendBuff));


	//	// procees responce
	//	res = read(new_socket, recvBuff, sizeof(recvBuff));
	//	if (read != 0) {
	//		socket2can_msg(recvBuff,res, can_msg_rx)
	//		uavcan_master->proceed_resp(can_msg);
	//	}

	//}


}





_dde_func_return_t CAN_ETHERNET::init_client(int mode)
{
	struct sockaddr_in addr; // структура с адресом
	struct hostent* hostinfo;
	int port = PORT;
	sock = socket(AF_INET, SOCK_STREAM, 0); // создание TCP-сокета

	if (sock < 0) { perror("socket"); exit(1);}

	// Указываем параметры сервера
	addr.sin_family = AF_INET; // домены Internet
	addr.sin_port = htons(port); // или любой другой порт...
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);// inet_addr("127.0.0.1");
	// установка соединения с сервером
	int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

	if (res < 0) {perror("if (connect(sock, ...");exit(2);}


	char sendBuff[] = "data from client";
	char recvBuff[100];

	//simulate data exchange CAN 
	//while (1)
	//{
	//	res = read(sock, recvBuff, sizeof(recvBuff));



	//	write(sock, sendBuff, strlen(sendBuff));
	//}


}
//-----------------------------------------------------------------------------------------------------------------------



_dde_func_return_t CAN_ETHERNET::read_can(TCAN_MSG* frame)
{
	//TODO add serialisation here
	uint32_t res=read(sock, frame, sizeof(frame));


}
//-----------------------------------------------------------------------------------------------------------------------



_dde_func_return_t CAN_ETHERNET::write_can(TCAN_MSG* frame)
{
	
	//TODO add serialisation here
	uint32_t res = write(sock, frame, sizeof(frame));
}
//-----------------------------------------------------------------------------------------------------------------------

//std::thread* thr_params;
*/