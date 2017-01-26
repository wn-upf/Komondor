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

#endif
