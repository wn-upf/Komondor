#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
* convertPower(): convert power units
* Input arguments:
* - conversion_type: unit conversion type
* - power: power value
* Output:
* - converted_power: power converted to required unit
*/
double convertPower(int conversion_type, double power){

	double converted_power;

	switch(conversion_type){
		// pW to dBm
		case PICO_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-9));
			break;
		}
		// dBm to pW
		case DBM_TO_PICO:{
			converted_power = pow(10,(power + 90)/10);
			break;
		}
		// mW to dBm
		case MILLI_TO_DBM:{
			converted_power = 10 * log10(power * pow(10,-6));
			break;
		}
		// dBm to mW (dB to linear)
		case DBM_TO_MILLI:
		case DB_TO_LINEAR:
		case DB_TO_W: {
			converted_power = pow(10,power/10);
			break;
		}
		// W to dB
		case W_TO_DB:
		case LINEAR_TO_DB: {
			converted_power = 10 * log10(power);
			break;
		}
		default:{
			printf("Power conversion type not found!\n");
			break;
		}
	}

	return converted_power;

}
