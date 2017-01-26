#ifndef _AUX1_
#define _AUX1_

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
