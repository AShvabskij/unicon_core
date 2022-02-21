
//

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include "register.h"
//#include <o1heap.h>


#ifdef DEFINE_DDE_NIOS_NODE
#include "../libcanard/canard.h"
#include "DDE_UAVCAN_defs.h"
#include "DDE_NODE.h"
#include "NIOS_CAN.h"
#else
#include "../CAN_layer/socketcan.h"
#include "../libcanard-2/libcanard/canard.h"
#include "../DDE_UAVCAN_defs.h"

#include "DDE_NODE_EMUL.h"

#endif


#include "DDE_UAVCAN_NODE.h"


//#include "RPI_SPI.h"
//#include "CAN_MCP2518FD.h"
//#include "CAN_ETHERNET.h"

static CanardInstance canard;
static CanardTxQueue queue;

#ifdef NIOS_NODE
    DDE_NODE dde_node = DDE_NODE_DEFAULTS;
#else
    DDE_NODE_EMUL dde_node = DDE_NODE_EMUL_DEFAULTS;
#endif

int do_many_canardRxSubscribe();
int TransmitTxQueue();
int ReceiveRxFrames();
void PublishHeartbeat();
void DDE_UAVCAN_SLAVE_ReceiveProcess(CanardRxTransfer* transfer);

static int sock;


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


//------------------------------------------------------ALL THIS SW IS IN pure  C---------------------------------
int DDE_UAVCAN_NODE_init(int node_id)
{

    // The libcanard instance requires the allocator for managing protocol states.
    canard = canardInit(&canardAllocate, &canardFree);
    //canard.user_reference = ;  // Make the state reachable from the canard instance.
    canard.node_id = node_id;

    queue = canardTxInit(100,         // Limit the size of the queue at 100 frames.
        CANARD_MTU_CAN_CLASSIC);// CANARD_MTU_CAN_FD);     // Set MTU = 64 bytes. There is also CANARD_MTU_CAN_CLASSIC.

    // Configure the transport by reading the appropriate standard registers.
    int state_canard_mtu_bytes = CANARD_MTU_CAN_CLASSIC;// CANARD_MTU_CAN_FD;

    #ifdef DEFINE_DDE_NIOS_NODE
        sock = nioscanOpen(state_canard_mtu_bytes > CANARD_MTU_CAN_CLASSIC);
    #else
        sock = socketcanOpen("vcan0", state_canard_mtu_bytes > CANARD_MTU_CAN_CLASSIC);
    #endif;

    // Load the port-IDs from the registers. You can implement hot-reloading at runtime if desired.
    // Publications:

    // Subscriptions:
    do_many_canardRxSubscribe();


}
    
int DDE_UAVCAN_NODE_1ms_calc()
{

        PublishHeartbeat();

}

int DDE_UAVCAN_NODE_update()
{

        ReceiveRxFrames();
        TransmitTxQueue();
}





void PublishHeartbeat() {


    char heartbeat[] = "heartbeat";
    static uint32_t monotonic_time;
    static uint8_t my_message_transfer_id = 0;  // Must be static or heap-allocated to retain state between calls.
    {



        const CanardTransferMetadata transfer = {
            //.timestamp_usec = monotonic_time + 1000000,  // Set transmission deadline 1 second, optimal for heartbeat.
            .priority = CanardPriorityNominal,
            .transfer_kind = CanardTransferKindMessage,
            .port_id = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
            .remote_node_id = CANARD_NODE_ID_UNSET,
            .transfer_id = (CanardTransferID)(my_message_transfer_id++)
        };


        int payload_size = sizeof(heartbeat);
        (void)canardTxPush(&queue, &canard, 1000000, &transfer, payload_size, &heartbeat[0]);
    }
}



// Transmit pending frames from the prioritized TX queue managed by libcanard.
// You can service an arbitrary number of redundant interfaces here!
int TransmitTxQueue()
{


    for (const CanardTxQueueItem* ti = NULL; (ti = canardTxPeek(&queue)) != NULL;)  // Peek at the top of the queue.
    {
        //if ((0U == ti->tx_deadline_usec) || (ti->tx_deadline_usec > getCurrentMicroseconds()))  // Check the deadline.
        {
            #ifdef DEFINE_DDE_NIOS_NODE
            const int16_t result = nioscanPush(sock, &ti->frame, 0);  // Non-blocking write attempt.
            #else       
            const int16_t result = socketcanPush(sock, &ti->frame, 0);  // Non-blocking write attempt.    
            #endif

            if (result == 0)
            {
                break;  // The queue is full, we will try again on the next iteration.
            }
            if (result < 0)
            {
                return -result;  // SocketCAN interface failure (link down?)
            }
        }
        // After the frame is transmitted or if it has timed out while waiting, pop it from the queue and deallocate:
        canard.memory_free(&canard, canardTxPop(&queue, ti));
    }


}




// Process received frames by feeding them from SocketCAN to libcanard.
// You can service an arbitrary number of redundant interfaces here, libcanard supports that!

const uint16_t  max_frames_to_process_per_iteration = 10;
const CanardMicrosecond loop_resolution = 100;

int ReceiveRxFrames()
{
    CanardFrame frame = { 0 };
    uint8_t     buf[CANARD_MTU_CAN_FD] = { 0 };
    for (uint16_t i = 0; i < max_frames_to_process_per_iteration; ++i)
    {
        #ifdef DEFINE_DDE_NIOS_NODE
        const int16_t socketcan_result = nioscanPop(sock, &frame, sizeof(buf), buf, loop_resolution, NULL);
        #else       
        const int16_t socketcan_result = socketcanPop(sock, &frame, sizeof(buf), buf, loop_resolution, NULL);
        #endif  

        if (socketcan_result == 0)  // The read operation has timed out with no frames, nothing to do here.
        {
            break;
        }
        if (socketcan_result < 0)  // The read operation has failed. This is not a normal condition.
        {
            return -socketcan_result;
        }
        // The SocketCAN adapter uses the wall clock for timestamping, but we need monotonic; override here.
        // Wall clock can only be used for time synchronization.
        //frame.timestamp_usec = getMonotonicMicroseconds();

        CanardMicrosecond rx_timestamp_usec;

        CanardRxTransfer transfer;

        print_frame(frame);

        const int8_t result = canardRxAccept(&canard,
            rx_timestamp_usec,          // When the frame was received, in microseconds.
            &frame,            // The CAN frame received from the bus.
            0,                          // If the transport is not redundant, use 0.
            &transfer,
            NULL);
        if (result < 0)
        {
            // An error has occurred: either an argument is invalid or we've ran out of memory.
            // It is possible to statically prove that an out-of-memory will never occur for a given application if
            // the heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
            // Reception of an invalid frame is NOT an error.
        }
        else if (result == 1)
        {
            DDE_UAVCAN_SLAVE_ReceiveProcess(&transfer);  // A transfer has been received, process it.
            canard.memory_free(&canard, transfer.payload);                  // Deallocate the dynamic memory afterwards.
        }
        else
        {
            // Nothing to do.
            // The received frame is either invalid or it's a non-last frame of a multi-frame transfer.
            // Reception of an invalid frame is NOT reported as an error because it is not an error.
        }

    }
}




static uint8_t resp[64 * 31] = { 0 }; //the largest response possible
void DDE_UAVCAN_SLAVE_ReceiveProcess(CanardRxTransfer* rx)
{

    static uint8_t single_param_resp_transfer_id = 0;  // Must be static or heap-allocated to retain state between calls.
    static uint8_t read_file_resp_transfer_id = 0;  // Must be static or heap-allocated to retain state between calls.

    static uint32_t emul_value = 0;
    emul_value++;


    if (rx->metadata.transfer_kind == CanardTransferKindRequest)
    {
        /**/
        if (rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_REQ)
        {
            int module_id = (((char*)rx->payload)[0]) & 0x3f;
            int param_id = (((char*)rx->payload)[1]) & 0x3f;


            uint16_t addr = (module_id << 6) | param_id;
            resp[0] = module_id;
            resp[1] = param_id;

            dde_node.read_mem32(addr, (uint32_t*)(&resp[2]));

            const CanardTransferMetadata transfer = {
                                .priority = CanardPriorityNominal,
                                .transfer_kind = CanardTransferKindResponse,
                                .port_id = UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_RESP,
                                .remote_node_id = rx->metadata.remote_node_id,
                                .transfer_id = (CanardTransferID)(single_param_resp_transfer_id++),
            };
            int payload_size = 7;
            (void)canardTxPush(&queue, &canard, 1000000, &transfer, payload_size, &resp[0]);
        }


        /**/
        if (rx->metadata.port_id == UAVCAN_SUBJECT_ID_DDE_READ_FILE_REQ)
        {
            uint16_t header_size = 6;
            uint16_t file_id = (((uint8_t*)rx->payload)[0]);
            uint16_t folder_id = (((uint8_t*)rx->payload)[1]); //not used 25.101.2021

            uint16_t addr_lo = (((uint8_t*)rx->payload)[2]);
            uint16_t addr_hi = (((uint8_t*)rx->payload)[3]);
            uint16_t buff_size_lo = (((uint8_t*)rx->payload)[4]);
            uint16_t buff_size_hi = (((uint8_t*)rx->payload)[5]);

            uint16_t buff_addr = (addr_hi << 8) | addr_lo;
            uint16_t buff_size = (buff_size_hi << 8) | buff_size_lo;
            if (buff_size > (queue.mtu_bytes * 31)) return; //ask too much
            //switch (file_id) {
            //case 0: 
            if ((buff_addr + buff_size) > dde_node.params.descr_size) return;//ask out of file
            //default:break;
            //}
            *((uint16_t*)&resp[0]) = file_id;
            *((uint16_t*)&resp[2]) = buff_addr;
            *((uint16_t*)&resp[4]) = buff_size;

            if (buff_addr == 0) //1st pack contains size in ((char*)rx->payload)[6..7] so do special proceed
            {
                *((uint16_t*)&resp[6]) = dde_node.params.descr_size;
                memcpy(&resp[8], &dde_node.params.descr[buff_addr], buff_size);
                header_size = 8;
            }
            else
                memcpy(&resp[6], &dde_node.params.descr[buff_addr], buff_size);

            const CanardTransferMetadata transfer = {
                                .priority = CanardPriorityNominal,
                                .transfer_kind = CanardTransferKindResponse,
                                .port_id = UAVCAN_SUBJECT_ID_DDE_READ_FILE_RESP,
                                .remote_node_id = rx->metadata.remote_node_id,
                                .transfer_id = (CanardTransferID)(read_file_resp_transfer_id++),
            };
            int payload_size = header_size + buff_size;
            (void)canardTxPush(&queue, &canard, 1000000, &transfer, payload_size, &resp[0]);
        }


    }
}



int  do_many_canardRxSubscribe()
{

    // Set up subject subscriptions and RPC-service servers.

    // Message subscriptions:
    if (canard.node_id > CANARD_NODE_ID_MAX)
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&canard,
                CanardTransferKindMessage,
                7509,      // The fixed Subject-ID of the Heartbeat message type (see DSDL definition).
                16,        // The extent (the maximum possible payload size) provided by Nunavut.
                CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                &rx);
        if (res < 0)
        {
            return -res;
        }
    }

    // Service servers:
    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&canard,
                CanardTransferKindRequest,
                UAVCAN_SUBJECT_ID_DDE_READ_SINGLE_PARAM_REQ,       // The Service-ID whose responses we will receive.
                8,      // The extent (see above).
                CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                &rx);
        if (res < 0)
        {
            return -res;
        }
    }

    {
        static CanardRxSubscription rx;
        const int8_t                res =  //
            canardRxSubscribe(&canard,
                CanardTransferKindRequest,
                UAVCAN_SUBJECT_ID_DDE_READ_FILE_REQ,
                8,
                CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
                &rx);
        if (res < 0)
        {
            return -res;
        }
    }

    //{
    //    static CanardRxSubscription rx;
    //    const int8_t                res =  //
    //        canardRxSubscribe(&state.canard,
    //            CanardTransferKindRequest,
    //            uavcan_register_Access_1_0_FIXED_PORT_ID_,
    //            uavcan_register_Access_Request_1_0_EXTENT_BYTES_,
    //            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
    //            &rx);
    //    if (res < 0)
    //    {
    //        return -res;
    //    }
    //}

    //{
    //    static CanardRxSubscription rx;
    //    const int8_t                res =  //
    //        canardRxSubscribe(&state.canard,
    //            CanardTransferKindRequest,
    //            uavcan_register_List_1_0_FIXED_PORT_ID_,
    //            uavcan_register_List_Request_1_0_EXTENT_BYTES_,
    //            CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_USEC,
    //            &rx);
    //    if (res < 0)
    //    {
    //        return -res;
    //    }
    //}
}