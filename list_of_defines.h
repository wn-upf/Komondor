// Fix parameters
#define T_SLOT 			0.000009

// Log hierarchy level to make output log file more readable
#define LOG_LVL1	""
#define LOG_LVL2	"  -"
#define LOG_LVL3	"    ·"
#define LOG_LVL4	"       +"
#define LOG_LVL5	"          *"

// Node states
#define STATE_UNKNOWN	-1	// Unkwnown state (e.g. at the beginning of the simulation)
#define STATE_SENSING	0	// Sensing the channel and decreasing backoff counter when possible according CCA requirements
#define STATE_TX		1	// Transmitting. Transmission are finished after the transmission duration established.
#define STATE_RX		2	// Receiving a packet from other node.

// Logs destination
#define PRINT_LOG		0	// Print logs per console
#define WRITE_LOG		1	// Write log in file

// Transmission initiated or finished
#define TX_INITIATED	0	// Transmission is initiated ('inportSomeNodeStartTX()')
#define TX_FINISHED		1	// Transmission is finished ('inportSomeNodeFinishTX()')
#define TX_NOT_POSSIBLE -1	// Transmission is not possible

// Logical Nack reasons
#define PACKET_NOT_LOST					-1	// Packet is not lost
#define PACKET_LOST_DESTINATION_TX		0	// Destination was already transmitting when the transmission was attempted
#define PACKET_LOST_LOW_SIGNAL			1	// Signal strenght is not enough (< CCA) to be decoded
#define PACKET_LOST_INTERFERENCE		2	// There are interference signals greater than cca (collision)
#define PACKET_LOST_PURE_COLLISION		3	// Two nodes transmitting to same destination with signal strengths enough to be decoded
#define PACKET_LOST_LOW_SIGNAL_AND_RX	4	// Destination already receiving and new signal strength was not enough to be decoded

// Destination and source node IDs
#define NO_NODE_ID	-1

// Packet ID
#define NO_PACKET_ID	-1
#define MAX_PACKET_ID	645210	// Maybe some day it is needed :D

// Timers
#define PAUSE_TIMER		0	// Try to pause a timer (e.g. backoff)
#define RESUME_TIMER	1	// Try to resume timer

// CCA
#define CCA_NOT_EXCEEDED	0	// CCA is not exceeded (primary channel is free)
#define CCA_EXCEEDED		1	// CCA is exceeded

// Power units conversion
#define PICO_TO_DBM		0	// Convert pW to dBm
#define DBM_TO_PICO		1	// Convert dBm to pW
#define MILLI_TO_DBM	2	// Convert mW to dBm
#define DBM_TO_MILLI	3	// Convert dBm to mW

// Boundary channels
#define FIRST_ONE_IN_ARRAY 	0	// Search first element '1' in an array
#define LAST_ONE_IN_ARRAY	1	// Search last element '1' in an array

// Channel free - occupied
#define CHANNEL_FREE		0
#define CHANNEL_OCCUPIED	1

// Progress bar
#define PROGRESS_BAR_DELTA	5	// Amount of percentage between two progress bar indicators

// Time variables
#define MIN_TIME_VALUE	0.000001

// Channel bonding model
#define CB_ONLY_PRIMARY		0	// Only Primary Channel used if FREE
#define CB_AGGRESIVE_SCB	1	// Aggresive SCB: if all channels are FREE, transmit. If not, generate a new backoff.
#define CB_LOG2_SCB			2	// Log2 SCB:  if all channels accepted by the log2 mapping are FREE, transmit. If not, generate a new backoff.
#define CB_AGGRESIVE_DCB	3	// Aggresive DCB: TX in all the free channels contiguous to the primary channel
#define CB_LOG2_DCB			4	// Log2 DCB: TX in the larger channel range allowed by the log2 mapping

/* LOG TYPE ENCODING: TODO */

// Setup() - A

// Start() - B
#define LOG_B00 "B00" 	// other option: #define LOG_B00 "B00: Start() starts"
#define LOG_B01 "B01"	// Start() ends
#define LOG_B02 "B02"
#define LOG_B03 "B03"
#define LOG_B04 "B04"
#define LOG_B05 "B05"
#define LOG_B06 "B06"
#define LOG_B07 "B07"
#define LOG_B08 "B08"
#define LOG_B09 "B09"
#define LOG_B10 "B10"
#define LOG_B11 "B11"
#define LOG_B12 "B12"
#define LOG_B13 "B13"
#define LOG_B14 "B14"
#define LOG_B15 "B15"
#define LOG_B16 "B16"

// Stop() - C
#define LOG_C00 "C00"	// Stop() starts
#define LOG_C01 "C01"	// Stop() ends
#define LOG_C02 "C02"	// Statistics: Time...
#define LOG_C03 "C03"
#define LOG_C04 "C04"
#define LOG_C05 "C05"
#define LOG_C06 "C06"
#define LOG_C07 "C07"
#define LOG_C08 "C08"
#define LOG_C09 "C09"
#define LOG_C10 "C10"
#define LOG_C11 "C11"
#define LOG_C12 "C12"
#define LOG_C13 "C13"
#define LOG_C14 "C14"
#define LOG_C15 "C15"
#define LOG_C16 "C16"

// InportSomeNodeStartTX() - D
#define LOG_D00 "D00"
#define LOG_D01 "D01"
#define LOG_D02 "D02"
#define LOG_D03 "D03"
#define LOG_D04 "D04"
#define LOG_D05 "D05"
#define LOG_D06 "D06"
#define LOG_D07 "D07"
#define LOG_D08 "D08"
#define LOG_D09 "D09"
#define LOG_D10 "D10"
#define LOG_D11 "D11"
#define LOG_D12 "D12"
#define LOG_D13 "D13"
#define LOG_D14 "D14"
#define LOG_D15 "D15"
#define LOG_D16 "D16"
#define LOG_D17 "D17"
#define LOG_D18 "D18"
#define LOG_D19 "D19"
#define LOG_D20 "D20"
#define LOG_D21 "D21"
#define LOG_D22 "D22"

// InportSomeNodeFinishTX() - E
#define LOG_E00 "E00"
#define LOG_E01 "E01"
#define LOG_E02 "E02"
#define LOG_E03 "E03"
#define LOG_E04 "E04"
#define LOG_E05 "E05"
#define LOG_E06 "E06"
#define LOG_E07 "E07"
#define LOG_E08 "E08"
#define LOG_E09 "E09"
#define LOG_E10 "E10"
#define LOG_E11 "E11"
#define LOG_E12 "E12"
#define LOG_E13 "E13"
#define LOG_E14 "E14"
#define LOG_E15 "E15"
#define LOG_E16 "E16"
#define LOG_E17 "E17"
#define LOG_E18 "E18"
#define LOG_E19 "E19"
#define LOG_E20 "E20"
#define LOG_E21 "E21"
#define LOG_E22 "E22"
#define LOG_E23 "E23"

// endBackoff() - F
#define LOG_F00 "F00"
#define LOG_F01 "F01"
#define LOG_F02 "F02"
#define LOG_F03 "F03"
#define LOG_F04 "F04"
#define LOG_F05 "F05"
#define LOG_F06 "F06"
#define LOG_F07 "F07"
#define LOG_F08 "F08"
#define LOG_F09 "F09"
#define LOG_F10 "F10"
#define LOG_F11 "F11"
#define LOG_F12 "F12"
#define LOG_F13 "F13"
#define LOG_F14 "F14"
#define LOG_F15 "F15"
#define LOG_F16 "F16"

// myTXFinished() - G
#define LOG_G00 "G00"
#define LOG_G01 "G01"
#define LOG_G02 "G02"
#define LOG_G03 "G03"
#define LOG_G04 "G04"
#define LOG_G05 "G05"
#define LOG_G06 "G06"
#define LOG_G07 "G07"
#define LOG_G08 "G08"
#define LOG_G09 "G09"
#define LOG_G10 "G10"
#define LOG_G11 "G11"
#define LOG_G12 "G12"
#define LOG_G13 "G13"
#define LOG_G14 "G14"
#define LOG_G15 "G15"
#define LOG_G16 "G16"

// inportNackReceived() - H
#define LOG_H00 "H00"
#define LOG_H01 "H01"
#define LOG_H02 "H02"
#define LOG_H03 "H03"
#define LOG_H04 "H04"
#define LOG_H05 "H05"
#define LOG_H06 "H06"
#define LOG_H07 "H07"
#define LOG_H08 "H08"
#define LOG_H09 "H09"
#define LOG_H10 "H10"
#define LOG_H11 "H11"
#define LOG_H12 "H12"
#define LOG_H13 "H13"
#define LOG_H14 "H14"
#define LOG_H15 "H15"
#define LOG_H16 "H16"

#define LOG_I00 "I00"
#define LOG_I01 "I01"
#define LOG_I02 "I02"
#define LOG_I03 "I03"
#define LOG_I04 "I04"
#define LOG_I05 "I05"
#define LOG_I06 "I06"
#define LOG_I07 "I07"
#define LOG_I08 "I08"
#define LOG_I09 "I09"
#define LOG_I10 "I10"
#define LOG_I11 "I11"
#define LOG_I12 "I12"
#define LOG_I13 "I13"
#define LOG_I14 "I14"
#define LOG_I15 "I15"
#define LOG_I16 "I16"

// Informative logs - Z
#define LOG_Z00 "Z00"	// Node info
#define LOG_Z01 "Z01"
#define LOG_Z02 "Z02"
#define LOG_Z03 "Z03"
#define LOG_Z04 "Z04"
#define LOG_Z05 "Z05"
#define LOG_Z06 "Z06"
#define LOG_Z07 "Z07"
#define LOG_Z08 "Z08"
#define LOG_Z09 "Z09"
#define LOG_Z10 "Z10"
#define LOG_Z11 "Z11"
#define LOG_Z12 "Z12"
#define LOG_Z13 "Z13"
#define LOG_Z14 "Z14"
#define LOG_Z15 "Z15"
#define LOG_Z16 "Z16"

