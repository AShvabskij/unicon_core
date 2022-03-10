
//#include <socketcan.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "DDE_UAVCAN_MASTER.h"
#include "DDE_UAVCAN_defs.h"
#include "CAN_layer/socketcan.h"

static CanardInstance canard;
static CanardTxQueue queue;
static int sock;

TDDE_UAVCAN_MASTER uavcan_master = DDE_UAVCAN_MASTER_DEFAULTS;

static int print_frame(CanardFrame*const frame)
{
    return 0;
//    printf("\r");
    //printf("\33[2K\r");
    //printf("%c[2K", 27);

    printf("RX frame %x %d   data:", frame->extended_can_id, frame->payload_size);
    for (int ii = 0; ii < frame->payload_size; ii++)
        printf(" %c", ((char*)frame->payload)[ii]);
    printf("\n");
}

static void* canardAllocate(CanardInstance* const ins, const size_t amount)
{
    (void)ins;
    return malloc(amount);
}

static void canardFree(CanardInstance* const ins, void* const pointer)
{
    (void)ins;
    free(pointer);
}

void DDE_UAVCAN_MASTER_init()
{
   

    // The libcanard instance requires the allocator for managing protocol states.
    canard = canardInit(&canardAllocate, &canardFree);
    //canard.user_reference = ;  // Make the state reachable from the canard instance.
    canard.node_id = CANARD_NODE_ID_MAX;// this should b somee devise as canard says // Anonymous service transfers are not allowed.
                                                                                     //// Anonymous multi-frame message trs are not allowed.
                                                                                      //was CANARD_NODE_ID_UNSET at initail tests

    queue = canardTxInit(100,         // Limit the size of the queue at 100 frames.
        CANARD_MTU_CAN_CLASSIC);                         // Set MTU = 64 bytes. There is also CANARD_MTU_CAN_CLASSIC.

    // Configure the transport by reading the appropriate standard registers.
#ifdef DEFINE_DDE_NIOS_NODE
    uavcan_master.canInit();
#else
    int state_canard_mtu_bytes = CANARD_MTU_CAN_CLASSIC;
    // We also need the bitrate configuration register. In this demo we can't really use it but an embedded application
    // shall define "uavcan.can.bitrate" of type natural32[2]; the second value is zero/ignored if CAN FD not supported.
    sock = socketcanOpen("vcan0", state_canard_mtu_bytes > CANARD_MTU_CAN_CLASSIC);
    if (sock < 0)
    {
        return -sock;
    }
#endif
    
    
    // we need to subscribe :

    static CanardRxSubscription heartbeat_subscription;
    (void)canardRxSubscribe(&canard,   // Subscribe to messages uavcan.node.Heartbeat.
                            CanardTransferKindMessage,
                            uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,      // The fixed Subject-ID of the Heartbeat message type (see DSDL definition).
                            16,        // The extent (the maximum possible payload size) provided by Nunavut.
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                            &heartbeat_subscription);

    static CanardRxSubscription serv_READ_GROUP_PARAM_RESP_subscription;
    (void)canardRxSubscribe(&canard,   // Subscribe to an arbitrary service response.
                            CanardTransferKindResponse,                         // Specify that we want service responses, not requests.
                            UAVCAN_SUBJECT_ID_DDE_READ_GROUP_PARAM_RESP,       // The Service-ID whose responses we will receive.
                            1024,      // The extent (see above).
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                            &serv_READ_GROUP_PARAM_RESP_subscription);

    static CanardRxSubscription serv_READ_SINGLE_PARAM_RESP_subscription;
    (void)canardRxSubscribe(&canard,   // Subscribe to an arbitrary service response.
                            CanardTransferKindResponse,                     // Specify that we want service responses, not requests.
                            UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_RESP,   // The Service-ID whose responses we will receive.
                            1024,      // The extent (see above).
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                            &serv_READ_SINGLE_PARAM_RESP_subscription);

    static CanardRxSubscription serv_WRITE_SINGLE_PARAM_RESP_subscription;
    (void)canardRxSubscribe(&canard,   // Subscribe to an arbitrary service response.
                            CanardTransferKindResponse,                         // Specify that we want service responses, not requests.
                            UAVCAN_SUBJECT_ID_DDE_WRITE_SINGLE_PARAM_RESP,       // The Service-ID whose responses we will receive.
                            1024,      // The extent (see above).
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                            &serv_READ_GROUP_PARAM_RESP_subscription);

    static CanardRxSubscription serv_READ_FILE_RESP_subscription;
    (void)canardRxSubscribe(&canard,   // Subscribe to an arbitrary service response.
                            CanardTransferKindResponse,                     // Specify that we want service responses, not requests.
                            UAVCAN_SUBJECT_ID_DDE_READ_FILE_RESP,   // The Service-ID whose responses we will receive.
                            1024,      // The extent (see above).
                            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                            &serv_READ_FILE_RESP_subscription);



    // // Configure the library to listen for register access service requests.
    // CanardRxSubscription srv_register_access;
    // (void) canardRxSubscribe(&canard,
    //                          CanardTransferKindRequest,
    //                          RegisterAccessServiceID,
    //                          1024U,  // Larger buffers are OK.
    //                          CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
    //                          &srv_register_access);

  
       
    //}
}
//-------------------------------------------------------------------------------------------------------------------------

void DDE_UAVCAN_MASTER_slow_calc(TDDE_UAVCAN_MASTER* p)
{

    // The main loop: publish messages and process service requests.
    const time_t boot_ts = time(NULL);
    time_t       next_1hz_at = boot_ts;


    //TransmitTxQueue();



}
//-------------------------------------------------------------------------------------------------------------------------


int ReciveRxCanard(CanardRxTransfer* rx_transfer)
{
    // this is how we receive transfers :
        static CanardMicrosecond rx_timestamp_usec;

    // // Process received frames, if any.
        CanardFrame received_frame;
        uint8_t  buffer[CANARD_MTU_CAN_FD] = { 0 };
        
        //const CanardRxTransfer transfer;
        (void)memset(&received_frame, 0, sizeof(CanardFrame));

        #ifdef DEFINE_DDE_NIOS_NODE
        received_frame.payload = &buffer;
        int res = uavcan_master.canPop(&received_frame);// 

        #else
        int16_t res = socketcanPop(sock, &received_frame, sizeof(buffer), buffer, 1000, NULL);  // Error handling not implemented
        #endif  

        if (res > 0)
        {

            //CanardRxTransfer transfer;

            const int8_t result = canardRxAccept(&canard,
                rx_timestamp_usec++,          // When the frame was received, in microseconds.
                &received_frame,            // The CAN frame received from the bus.
                0,                          // If the transport is not redundant, use 0.
                rx_transfer,
                NULL);
            

            if (result < 0)
            {
                // An error has occurred: either an argument is invalid or we've ran out of memory.
                // It is possible to statically prove that an out-of-memory will never occur for a given application if
                // the heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
                // Reception of an invalid frame is NOT an error.
                return -1;

            }
            else if (result == 1)
            {       //         if ((transfer.transfer_kind == CanardTransferKindRequest) &&
               //             (transfer.port_id == RegisterAccessServiceID))
               //         {
               ////             handleRegisterAccess(&canard, &transfer);
               //         }
                
                
                //processReceivedTransfer(&transfer);  // A transfer has been received, process it.
                

                //canard.memory_free(&canard, transfer.payload);                  // Deallocate the dynamic memory afterwards.
                return 1;
            }
            else
            {
                return 0;

                // Nothing to do.
                // The received frame is either invalid or it's a non-last frame of a multi-frame transfer.
                // Reception of an invalid frame is NOT reported as an error because it is not an error.
            }
        }
}
//-------------------------------------------------------------------------------------------------------------------------

int DDE_UAVCAN_MASTER_ReceiveProcess(CanardRxTransfer* rx_transfer)
{
  
    int res = ReciveRxCanard(rx_transfer);
    
    return res;

}

int DDE_UAVCAN_MASTER_FreeReceiveBuff(CanardRxTransfer* rx_transfer) 
{
    canard.memory_free(&canard, rx_transfer->payload);                  // Deallocate the dynamic memory afterwards.
}




int DDE_UAVCAN_MASTER_set_param(uint16_t node_id, uint16_t param_id, uint32_t ivalue)
{
    assert(node_id < UAVCAN_DEVICES_MAX);
    assert(param_id < UAVCAN_PARAMS_ID_MAX);

    static uint8_t buff[8];

    buff[0] = param_id & 0xff;
    buff[1] = param_id >> 8;    
    buff[2] = ivalue >> 24;
    buff[3] = ivalue >> 16;
    buff[4] = ivalue >> 8;
    buff[5] = ivalue;

    static CanardTransferID my_message_transfer_id;  // Must be static or heap-allocated to retain state between calls.
    const CanardTransferMetadata transfer_metadata = {
                                                    .priority = CanardPriorityNominal,
                                                    .transfer_kind = CanardTransferKindRequest,
                                                    .port_id = UAVCAN_SUBJECT_ID_DDE_WRITE_SINGLE_PARAM_REQ,      // This is the subject-ID.
                                                    .remote_node_id = node_id,       // Messages cannot be unicast, so use UNSET.
                                                    .transfer_id = my_message_transfer_id,
    };

    ++my_message_transfer_id;  // The transfer-ID shall be incremented after every transmission on this subject.
    int32_t result = canardTxPush(&queue,               // Call this once per redundant CAN interface (queue).
        &canard,
        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,     // Zero if transmission deadline is not limited.
        &transfer_metadata,
        6,                   // Size of the message payload (see Nunavut transpiler).
        &buff); 
    if (result < 0)
    {
        // An error has occurred: either an argument is invalid, the TX queue is full, or we've run out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
    }

}


int DDE_UAVCAN_MASTER_get_param(uint16_t node_id, uint16_t param_id)
{
    assert(node_id < UAVCAN_DEVICES_MAX);
    assert(param_id < UAVCAN_PARAMS_ID_MAX);

    static CanardTransferID my_message_transfer_id;  // Must be static or heap-allocated to retain state between calls.
    const CanardTransferMetadata transfer_metadata = {
                                                    .priority = CanardPriorityNominal,
                                                    .transfer_kind = CanardTransferKindRequest,
                                                    .port_id = UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_REQ,      // This is the subject-ID.
                                                    .remote_node_id = node_id,       // Messages cannot be unicast, so use UNSET.
                                                    .transfer_id = my_message_transfer_id,
    };

    ++my_message_transfer_id;  // The transfer-ID shall be incremented after every transmission on this subject.
    int32_t result = canardTxPush(&queue,               // Call this once per redundant CAN interface (queue).
        &canard,
        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,     // Zero if transmission deadline is not limited.
        &transfer_metadata,
        2,                   // Size of the message payload (see Nunavut transpiler).
        &param_id);// "\x2D\x00" "DDE_UAVCAN_MASTER_request_param"); //TODO no need for any data to pass 
    if (result < 0)
    {
        // An error has occurred: either an argument is invalid, the TX queue is full, or we've run out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
    }


}


int DDE_UAVCAN_MASTER_file_request(uint16_t node_id, uint16_t file_id, uint16_t addr, uint16_t size)
{
    assert(node_id < UAVCAN_DEVICES_MAX);
    //assert(param_id < UAVCAN_PARAMS_ID_MAX);
    static uint8_t msg[8];// = { file_id, addr, size };
    uint8_t* p = &msg[8];
    *((uint16_t*)&msg[0]) = file_id;
    *((uint16_t*)&msg[2]) = addr;
    *((uint16_t*)&msg[4]) = size;

    static CanardTransferID my_message_transfer_id;  // Must be static or heap-allocated to retain state between calls.
    const CanardTransferMetadata transfer_metadata = {
                                                    .priority = CanardPriorityNominal,
                                                    .transfer_kind = CanardTransferKindRequest,
                                                    .port_id = UAVCAN_SUBJECT_ID_DDE_READ_FILE_REQ,      // This is the subject-ID.
                                                    .remote_node_id = node_id,       // Messages cannot be unicast, so use UNSET.
                                                    .transfer_id = my_message_transfer_id,
    };

    ++my_message_transfer_id;  // The transfer-ID shall be incremented after every transmission on this subject.
    int32_t result = canardTxPush(&queue,               // Call this once per redundant CAN interface (queue).
        &canard,
        CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,     // Zero if transmission deadline is not limited.
        &transfer_metadata,
        sizeof(msg),                   // Size of the message payload (see Nunavut transpiler).
        &msg);// "\x2D\x00" "DDE_UAVCAN_MASTER_request_param"); //TODO no need for any data to pass 
    
    if (result < 0)
    {
        // An error has occurred: either an argument is invalid, the TX queue is full, or we've run out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
        return -1;
    }
    
    return result;


}


// Transmit pending frames from the prioritized TX queue managed by libcanard.
// You can service an arbitrary number of redundant interfaces here!

int DDE_UAVCAN_MASTER_TransmitProcess()
{
    //this was func TransmitTxQueue(); no need

    uint16_t tx_counter = 0;

    for (const CanardTxQueueItem* ti = NULL; (ti = canardTxPeek(&queue)) != NULL;)  // Peek at the top of the queue.
    {
        //if ((0U == ti->tx_deadline_usec) || (ti->tx_deadline_usec > getCurrentMicroseconds()))  // Check the deadline.
        {

            const int16_t result = uavcan_master.canPush(&ti->frame);////socketcanPush(sock, &ti->frame, 0);  // Non-blocking write attempt.
            if (result == 0)
            {
                break;  // The queue is full, we will try again on the next iteration.
            }
            if (result < 0)
            {
                return -result;  // SocketCAN interface failure (link down?)
            }
        }
        tx_counter++;
        // After the frame is transmitted or if it has timed out while waiting, pop it from the queue and deallocate:
        canard.memory_free(&canard, canardTxPop(&queue, ti));
    }


}





//-------------------------------------------------------------------------------------------------------------------------




//this is template funct
long DDE_UAVCAN_MASTER_canInit()
{
    return -1;
}
//this is template funct
long DDE_UAVCAN_MASTER_canPop(CanardFrame* frame) 
{
    return -1;
}

//this is template funct
long DDE_UAVCAN_MASTER_canPush(CanardFrame* const frame) 
{
    return -1;
}