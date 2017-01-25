#ifndef _AUX_
#define _AUX_

// Notification specific info (may be not checked by the other nodes)
struct TxInfo
{
	int packet_id;			// Packet identifier
	int destination_id;		// Destination node of the transmission
	double tx_duration;		// Duration of the transmission
	double tx_power;		// Transmission power in [dBm]
	int x;					// X position of source node
	int y;					// Y position of source node
	int z;					// Z position of source node

	void printTxInfo(void){
		printf("packet_id = %d - destination_id = %d - tx_duration = %f - tx_power = %f - position = (%d, %d, %d)\n",
				packet_id, destination_id, tx_duration, tx_power, x, y, z);
	}
};

// Notification info
struct Notification
{
	// ALways read in destination
	int source_id;		// Node id of the source
	int left_channel;	// Left channel used in the transmission
	int right_channel;	// Right channel used in the transmission
	int packet_length;	// Size of the packet to transmit

	// Specific transmission info (may not be checked by the others nodes)
	TxInfo tx_info;

	void printNotification(void){
		printf("source_id = %d - left_channel = %d - right_channel = %d - packet_length = %d -",
				source_id, left_channel, right_channel, packet_length);
		printf("tx_info: ");
		tx_info.printTxInfo();
	}
};

// NACK packet
struct NackInfo
{
	int source_id;			// Node sending the NACK
	int packet_id;			// Packet_id
	int reason_id;			// Why the NACK is generated (, etc.)
	/*
	 * - 0: destination was transmitting
	 * - 1: power received < CCA
	 * - 2: interference power >= CCA
	 * - 3: 1 + 2
	 * - 4: pure collision (2 nodes transmitting to same destination)
	 */
	int node_id_a;			// (other uses may be) Hidden node that started the transmission
	int node_id_b;			// Hidden node causing the collision

	void printNackInfo(void){
		printf("source_id = %d - packet_id = %d - reason_id = %d - node_id_a = %d - node_id_b = %d\n",
				source_id, packet_id, reason_id, node_id_a, node_id_b);
	}
};

#endif
