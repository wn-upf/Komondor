#include <math.h>
#include <algorithm>
#include <stddef.h>

#include "../ListOfDefines.h"

/*
 * selectDestination(): pick destination node
 */
int pickRandomElementFromList(int *list, int list_size){

	int element = 0;
	// Pick one of the STAs in the WLAN uniformely
	if(list_size > 0){

		int rand_ix = rand()%(list_size);
		element = list[rand_ix];

	} else {

		element = NODE_ID_NONE;
		printf("The list does not contain elements!\n");

	}

	return element;

}


void printOrWriteArrayInt(int *list, int list_size, int write_or_print, int save_node_logs,
		int print_node_logs, Logger node_logger) {

	switch(write_or_print){

		case PRINT_LOG:{
			if(print_node_logs){
				for(int c = 0; c < list_size; c++){
					printf("%d  ", list[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < list_size; c++){
				 if(save_node_logs)  fprintf(node_logger.file, "%d  ", list[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

void printOrWriteArrayDouble(double *list, int list_size, int write_or_print, int save_node_logs,
		int print_node_logs, Logger node_logger) {

	switch(write_or_print){

		case PRINT_LOG:{
			if(print_node_logs){
				for(int c = 0; c < list_size; c++){
					printf("%f  ", list[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < list_size; c++){
				 if(save_node_logs)  fprintf(node_logger.file, "%f  ", list[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}

}

/*
 * getBoundaryChannel()
 * Input arguments:
 * - position: flag indicating to find the first or last one in the array
 * - channels_available: range to consider
 * - total_channels_number: total number of channels
 * Output:
 * - left_elem or right_elem: first or last one in the array
 */
int getFirstOrLastTrueElemOfArray(int position, int *list, int list_size){

  int left_elem = 0;
  int left_elem_is_set = 0;
  int right_elem = 0;
  for(int c = 0; c < list_size; c++){
    if(list[c]){
      if(!left_elem_is_set){
        left_elem = c;
        left_elem_is_set = TRUE;
      }
      if(right_elem < c){
        right_elem = c;
      }
    }
  }
  if(position) {
    return right_elem; //LAST
  } else {
    return left_elem; //FIRST
  }

}
