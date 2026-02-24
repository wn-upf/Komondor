#ifndef PACKET_EXCHANGE_SEQUENCE_H
#define PACKET_EXCHANGE_SEQUENCE_H

#include "../list_of_macros.h"   // PACKET_TYPE_* constants

/**
 * PacketExchangeSequence — describes the ordered sequence of MAC frames in a
 * single transmission attempt (e.g., RTS/CTS/DATA/ACK or DATA/ACK).
 *
 * The Node member `exchange_sequence` selects the active protocol variant.
 * Set it during initialization or before a transmission to switch modes.
 *
 * Predefined constants:
 *   IEEE_802_11: DATA -> ACK (no RTS/CTS)
 *   IEEE_802_11_RTS_CTS: RTS -> CTS -> DATA -> ACK
 *   IEEE_802_11_COTDMA: 
 *   IEEE_802_11_COBF_COSR: 
 */
struct PacketExchangeSequence {
	int        num_frames;      ///> Total frames in the exchange (e.g., 2 or 4)
	int        frame_types[8];  ///> Ordered PACKET_TYPE_* values; unused slots set to -1
	const char *name;           ///> Human-readable label (for logging)
};

/* IEEE 802.11 2-way exchange: DATA -> ACK (RTS/CTS disabled) */
static const PacketExchangeSequence IEEE_802_11 = {
	2,
	{PACKET_TYPE_DATA, PACKET_TYPE_ACK, -1, -1, -1, -1, -1, -1},
	"IEEE 802.11 (RTS/CTS disabled)"
};

/* IEEE 802.11 4-way handshake with RTS/CTS: RTS -> CTS -> DATA -> ACK */
static const PacketExchangeSequence IEEE_802_11_RTS_CTS = {
	4,
	{PACKET_TYPE_RTS, PACKET_TYPE_CTS, PACKET_TYPE_DATA, PACKET_TYPE_ACK, -1, -1, -1, -1},
	"IEEE 802.11 (RTS/CTS enabled)"
};

/* MAPC exchange (Co-TDMA only) */
static const PacketExchangeSequence IEEE_802_11_COTDMA = {
	7,
	{PACKET_TYPE_ICF, PACKET_TYPE_ICR, PACKET_TYPE_DATA, PACKET_TYPE_ACK, 
		PACKET_TYPE_MU_RTS_TXS, PACKET_TYPE_DATA, PACKET_TYPE_ACK, -1},
	"Co-TDMA packet exchange sequence"
};


/* MAPC exchange (Co-BF & Co-SR) */
static const PacketExchangeSequence IEEE_802_11_COBF_COSR = {
	6,
	{PACKET_TYPE_ICF, PACKET_TYPE_ICR, PACKET_TYPE_TF, 
		PACKET_TYPE_DATA, PACKET_TYPE_TF, PACKET_TYPE_ACK, -1, -1},
	"Co-BF & Co-SR packet exchange sequence"
};

#endif /* PACKET_EXCHANGE_SEQUENCE_H */
