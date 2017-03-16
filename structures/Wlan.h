#ifndef _AUX_WLAN_
#define _AUX_WLAN_

#include "../ListOfDefines.h"
#include "Logger.h"

// WLAN info
struct Wlan
{
	int wlan_id;		// WLAN ID
	char *wlan_code;	// Code of the WLAN (string)
	int num_stas;		// Number of STAs in the WLAN (AP not included)
	int ap_id;			// Id of the Access Point
	int *list_sta_id;	// List of STAs IDs belonging to the WLAN

	/*
	 * setSizeOfSTAsArray(): sets the size of the array list_sta_id
	 */
	void setSizeOfSTAsArray(int num_stas){

		list_sta_id = (int *) malloc(num_stas * sizeof(*list_sta_id));

		for(int s = 0; s < num_stas; s++){
			list_sta_id[s] = NODE_ID_NONE;
		}
	}

	/*
	 * printStaIds(): prints the list of STAs IDs belonging to the WLAN
	 */
	void printStaIds(){

		for(int s = 0; s < num_stas; s++){
			printf("%d  ", list_sta_id[s]);
		}
		printf("\n");
	}


	/*
	 * writeStaIds(): writes STAs list of IDs in a given file
	 * Input arguments:
	 * - logger: logger containing the file to write on
	 */
	void writeStaIds(Logger logger){

		if (logger.save_logs){
			for(int s = 0; s < num_stas; s++){
				fprintf(logger.file, "%d  ", list_sta_id[s]);
			}
		}
	}

	/*
	 * printWlanInfo(): prints general WLAN info
	 */
	void printWlanInfo(){

		printf("%s WLAN %s:\n", LOG_LVL3, wlan_code);
		printf("%s wlan_id: %d\n", LOG_LVL4, wlan_id);
		printf("%s num_stas: %d\n", LOG_LVL4, num_stas);
		printf("%s ap_id: %d\n", LOG_LVL4, ap_id);
		printf("%s list of STAs IDs: ", LOG_LVL4);
		printStaIds();
	}

	/*
	 * writeWlanInfo(): writes general WLAN info in a given file
	 * Input arguments:
	 * - logger: logger containing the file to write on
	 * - header_string: header string
	 */
	void writeWlanInfo(Logger logger, char *header_string){

		if(header_string == NULL){
			header_string = (char *) malloc(1);
			sprintf(header_string, " ");
		}

		if (logger.save_logs){
			fprintf(logger.file, "%s WLAN %s:\n", header_string, wlan_code);
			fprintf(logger.file, "%s - wlan_id: %d\n", header_string, wlan_id);
			fprintf(logger.file, "%s - num_stas: %d\n", header_string, num_stas);
			fprintf(logger.file, "%s - ap_id: %d\n", header_string, ap_id);
			fprintf(logger.file, "%s - list of STAs IDs: ", header_string);
			writeStaIds(logger);
			fprintf(logger.file, "\n");
		}
	}
};

#endif
