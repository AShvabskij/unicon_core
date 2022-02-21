
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "DDE_UAVCAN_DEVICE.h"
#include "DDE_UAVCAN_defs.h"


// (interface_CAN* interface_CAN, uint8_t node_ID)
static CanardInstance canard;
static CanardTxQueue queue;
//CanardRxQueue queue;


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



int DDE_UAVCAN_DEVICE_init(TDDE_UAVCAN_DEVICE*p)
{
    uint16_t res;

    // Initialize the node with a static node-ID as specified in the command-line arguments.

    canard = canardInit(&canardAllocate, &canardFree);
    canard.node_id = p->node_id;                        // Defaults to anonymous; can be set up later at any point.


    queue = canardTxInit(100, // Limit the size of the queue at 100 frames.
        CANARD_MTU_CAN_FD);  // Set MTU = 64 bytes. There is also CANARD_MTU_CAN_CLASSIC.

    //TODO = add check on queue

    // Initialize a SocketCAN socket. Do not use CAN FD to enhance compatibility.

    res = p->can_init();

    if (res < 0)
    {
        //fprintf(stderr, "Could not initialize interface_CAN->init(node_ID): errno %d %s\n", -sock, strerror(-sock));
        return 1;
    }

}
//------------------------------------------------------------------------------------------------------------------------------



static void publishHeartbeat(CanardInstance* const canard, const uint32_t uptime)
{
    static CanardTransferID transfer_id;
    static CanardMicrosecond tx_deadline_usec;

    const CanardTransferMetadata transfer = {
        //.timestamp_usec = uptime,
        .priority = CanardPriorityNominal,
        .transfer_kind = CanardTransferKindMessage,
        .port_id = 0x1999, // HEART_BEAT_SUBJECT_ID,
        .remote_node_id = CANARD_NODE_ID_UNSET,
        .transfer_id = transfer_id,
    };
    ++transfer_id;
    int32_t result = canardTxPush(  &queue,
                                    &canard,
                                    tx_deadline_usec,
                                    &transfer,
                                    47,
                                    "\x2D\x00" "Sancho, heatrtbets and ai like it nomotrfear.");

    if (result < 0)
    {
        // An error has occurred: either an argument is invalid, the TX queue is full, or we've run out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
    }
}
//------------------------------------------------------------------------------------------------------------------------------




static void PublishMessage(CanardInstance* const canard, const uint32_t uptime) {

    static CanardTransferID message_transfer_id;  // Must be static or heap-allocated to retain state between calls.
    static CanardMicrosecond tx_deadline_usec;
    
    
    const CanardTransferMetadata transfer_metadata = {
        .priority = CanardPriorityNominal,
        .transfer_kind = CanardTransferKindMessage,
        .port_id = 1234,                               // This is the subject-ID.
        .remote_node_id = CANARD_NODE_ID_UNSET,       // Messages cannot be unicast, so use UNSET.
        .transfer_id = message_transfer_id,
    };

    ++message_transfer_id;  // The transfer-ID shall be incremented after every transmission on this subject.
    int32_t result = canardTxPush( &queue,               // Call this once per redundant CAN interface (queue).
                                    &canard,
                                    tx_deadline_usec,     // Zero if transmission deadline is not limited.
                                    &transfer_metadata,
                                    47,                   // Size of the message payload (see Nunavut transpiler).
                                    "\x2D\x00" "Sancho, it strikes me thou art in great fear.");                             //TODO PAYLOAD HERE!!!
    if (result < 0)
    {
        // An error has occurred: either an argument is invalid, the TX queue is full, or we've run out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
    }

}
//------------------------------------------------------------------------------------------------------------------------------

   
static int can_rx(CanardFrame* frame) {}
 
static int can_tx(CanardFrame* frame) {}

//----------------------------------------------------------------------------------------------------



static void processReceivedTransfer(const CanardTransferMetadata* transfer)
{

     if (transfer->transfer_kind == CanardTransferKindMessage)
     {
         //size_t size = transfer->payload_size;
         if (transfer->port_id == 100) // uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_)
         {
             //uavcan_pnp_NodeIDAllocationData_2_0 msg = { 0 };
             //if (uavcan_pnp_NodeIDAllocationData_2_0_deserialize_(&msg, transfer->payload, &size) >= 0)
            // {
             //    processMessagePlugAndPlayNodeIDAllocation(state, &msg);
            // }
         }
         else
         {
             assert(false);  // Seems like we have set up a port subscription without a handler -- bad implementation.
         }
     }
     else if (transfer->transfer_kind == CanardTransferKindRequest)
     {
         if (transfer->port_id == 101) // uavcan_node_GetInfo_1_0_FIXED_PORT_ID_)
         {
             // The request object is empty so we don't bother deserializing it. Just send the response.
             //const uavcan_node_GetInfo_Response_1_0 resp = processRequestNodeGetInfo();
             //uint8_t      serialized[uavcan_node_GetInfo_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = { 0 };
             //size_t       serialized_size = sizeof(serialized);
             const int8_t res = 0;// uavcan_node_GetInfo_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size);
             if (res >= 0)
             {
                 //CanardTransferMetadata rt = *transfer;  // Response transfers are similar to their requests.
                 //rt.timestamp_usec = transfer->timestamp_usec + MEGA;
                 //rt.transfer_kind = CanardTransferKindResponse;
                 //rt.payload_size = serialized_size;
                 //rt.payload = &serialized[0];
                 //(void)canardTxPush(&state->canard, &rt);
             }
             else
             {
                 assert(false);
             }
         }
         //else if (transfer->port_id == uavcan_register_Access_1_0_FIXED_PORT_ID_)
         //{
         //    uavcan_register_Access_Request_1_0 req = { 0 };
         //    size_t                             size = transfer->payload_size;
         //    if (uavcan_register_Access_Request_1_0_deserialize_(&req, transfer->payload, &size) >= 0)
         //    {
         //        const uavcan_register_Access_Response_1_0 resp = processRequestRegisterAccess(&req);
         //        uint8_t serialized[uavcan_register_Access_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = { 0 };
         //        size_t  serialized_size = sizeof(serialized);
         //        if (uavcan_register_Access_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
         //        {
         //            CanardTransfer rt = *transfer;  // Response transfers are similar to their requests.
         //            rt.timestamp_usec = transfer->timestamp_usec + MEGA;
         //            rt.transfer_kind = CanardTransferKindResponse;
         //            rt.payload_size = serialized_size;
         //            rt.payload = &serialized[0];
         //            (void)canardTxPush(&state->canard, &rt);
         //        }
         //    }
         //}
         //else if (transfer->port_id == uavcan_register_List_1_0_FIXED_PORT_ID_)
         //{
         //    uavcan_register_List_Request_1_0 req = { 0 };
         //    size_t                           size = transfer->payload_size;
         //    if (uavcan_register_List_Request_1_0_deserialize_(&req, transfer->payload, &size) >= 0)
         //    {
         //        const uavcan_register_List_Response_1_0 resp = { .name = registerGetNameByIndex(req.index) };
         //        uint8_t serialized[uavcan_register_List_Response_1_0_SERIALIZATION_BUFFER_SIZE_BYTES_] = { 0 };
         //        size_t  serialized_size = sizeof(serialized);
         //        if (uavcan_register_List_Response_1_0_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
         //        {
         //            CanardTransfer rt = *transfer;  // Response transfers are similar to their requests.
         //            rt.timestamp_usec = transfer->timestamp_usec + MEGA;
         //            rt.transfer_kind = CanardTransferKindResponse;
         //            rt.payload_size = serialized_size;
         //            rt.payload = &serialized[0];
         //            (void)canardTxPush(&state->canard, &rt);
         //        }
         //    }
         //}
         //else if (transfer->port_id == uavcan_node_ExecuteCommand_1_1_FIXED_PORT_ID_)
         //{
         //    uavcan_node_ExecuteCommand_Request_1_1 req = { 0 };
         //    size_t                                 size = transfer->payload_size;
         //    if (uavcan_node_ExecuteCommand_Request_1_1_deserialize_(&req, transfer->payload, &size) >= 0)
         //    {
         //        const uavcan_node_ExecuteCommand_Response_1_1 resp = processRequestExecuteCommand(&req);
         //        uint8_t serialized[uavcan_node_ExecuteCommand_Response_1_1_SERIALIZATION_BUFFER_SIZE_BYTES_] = { 0 };
         //        size_t  serialized_size = sizeof(serialized);
         //        if (uavcan_node_ExecuteCommand_Response_1_1_serialize_(&resp, &serialized[0], &serialized_size) >= 0)
         //        {
         //            CanardTransfer rt = *transfer;  // Response transfers are similar to their requests.
         //            rt.timestamp_usec = transfer->timestamp_usec + MEGA;
         //            rt.transfer_kind = CanardTransferKindResponse;
         //            rt.payload_size = serialized_size;
         //            rt.payload = &serialized[0];
         //            (void)canardTxPush(&state->canard, &rt);
         //        }
         //    }
         //}
         //else
         //{
         //    assert(false);  // Seems like we have set up a port subscription without a handler -- bad implementation.
         //}
     }
     else
     {
         assert(false);  // Bad implementation -- check your subscriptions.
     }
 }

 static void process_receive(const CanardFrame* received_frame)
 {




     // We can subscribeand unsubscribe at runtime as many times as we want.Normally, 
     // however, an embedded application would subscribe onceand roll with it.Okay, this is how we receive transfers :

     CanardMicrosecond rx_timestamp_usec;
     CanardRxTransfer transfer;
     const int8_t result = canardRxAccept(&canard,
                                         rx_timestamp_usec,          // When the frame was received, in microseconds.
                                         &received_frame,            // The CAN frame received from the bus.
                                         0,  // If the transport is not redundant, use 0.
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
         processReceivedTransfer(&transfer);             // A transfer has been received, process it.
         canard.memory_free(&canard, transfer.payload);     // Deallocate the dynamic memory afterwards.
     }
     else
     {
         // Nothing to do.
         // The received frame is either invalid or it's a non-last frame of a multi-frame transfer.
         // Reception of an invalid frame is NOT reported as an error because it is not an error.
     }
 }

static void process_transmite()
{
     for (const CanardTxQueueItem* ti = NULL; (ti = canardTxPeek(&queue)) != NULL;)  // Peek at the top of the queue.
     {
         if ((0U == ti->tx_deadline_usec) || (ti->tx_deadline_usec > getCurrentMicroseconds()))  // Check the deadline.
         {
             if (!can_tx(&ti->frame))               // Send the frame over this redundant CAN iface.
             {
                 break;                             // If the driver is busy, break and retry later.
             }
         }
         // After the frame is transmitted or if it has timed out while waiting, pop it from the queue and deallocate:
         canard.memory_free(&canard, canardTxPop(&queue, ti));
     }

 }
 

int DDE_UAVCAN_DEVICE_slow_calc(TDDE_UAVCAN_DEVICE* p)
{
    static CanardFrame received_frame;
    can_rx(&received_frame);

    process_receive(&received_frame);

    process_transmite();

}


//------------------------------------------------------------------
int DDE_UAVCAN_DEVICE_ms_calc(TDDE_UAVCAN_DEVICE*p)
 {
 
 }
 //------------------------------------------------------------------

 
int DDE_UAVCAN_DEVICE_can_tx(CanardFrame*frame) 
{ 
    return -1; 
}

int DDE_UAVCAN_DEVICE_can_rx(CanardFrame*frame)  
{ 
    return -1; 
}


 /// Constructs a response to uavcan.node.GetInfo which contains the basic information about this node.
 //static uavcan_node_GetInfo_Response_1_0 processRequestNodeGetInfo()
 //{
     //uavcan_node_GetInfo_Response_1_0 resp = { 0 };
     //resp.protocol_version.major = CANARD_UAVCAN_SPECIFICATION_VERSION_MAJOR;
     //resp.protocol_version.minor = CANARD_UAVCAN_SPECIFICATION_VERSION_MINOR;

     //// The hardware version is not populated in this demo because it runs on no specific hardware.
     //// An embedded node would usually determine the version by querying the hardware.

     //resp.software_version.major = VERSION_MAJOR;
     //resp.software_version.minor = VERSION_MINOR;
     //resp.software_vcs_revision_id = VCS_REVISION_ID;

     //getUniqueID(resp.unique_id);

     //// The node name is the name of the product like a reversed Internet domain name (or like a Java package).
     //resp.name.count = strlen(NODE_NAME);
     //memcpy(&resp.name.elements, NODE_NAME, resp.name.count);

     //// The software image CRC and the Certificate of Authenticity are optional so not populated in this demo.
     //return resp;
 //}
 //----------------------------------------------------------------------------------------------------
 //EOF
