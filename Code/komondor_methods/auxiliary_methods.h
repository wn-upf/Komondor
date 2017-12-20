/* Komondor IEEE 802.11ax Simulator
 *
 * Copyright (c) 2017, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007

 * Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Sergio Barrachina-Muñoz and Francesc Wilhelmi
 * Created : 2016-12-05
 * Updated : $Date: 2017/03/20 10:32:36 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 * File description: this is the main Komondor file
 *
 * - This file contains the auxiliary methods to carry out the rest of operations 
 */

#include <math.h>
#include <algorithm>
#include <stddef.h>
#include "../list_of_macros.h"

/*
 * PickRandomElementFromArray(): pick uniformely random an element of an array
 */
int PickRandomElementFromArray(int *array, int array_size){

	int element = 0;
	// Pick one of the STAs in the WLAN uniformely
	if(array_size > 0){

		int rand_ix = rand()%(array_size);
		element = array[rand_ix];

	} else {

		element = NODE_ID_NONE;
		printf("The list does not contain elements!\n");

	}

	return element;

}

/*
 * PrintOrWriteArrayInt(): prints per console or writes to a given file the elements of an "int" array
 */
void PrintOrWriteArrayInt(int *list, int list_size, int write_or_print, int save_node_logs,
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

/*
 * PrintOrWriteArrayDouble(): prints per console or writes to a given file the elements of an "double" array
 */
void PrintOrWriteArrayDouble(double *list, int list_size, int write_or_print, int save_node_logs,
		int print_node_logs, Logger node_logger) {

	switch(write_or_print){

		case PRINT_LOG:{
			if(print_node_logs){
				for(int c = 0; c < list_size; c++){
					printf("%.2f  ", list[c]);
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
 * GetFirstOrLastTrueElemOfArray(): pick the first or last TRUE element of an array
 */
int GetFirstOrLastTrueElemOfArray(int first_or_last, int *list, int list_size){

	int left_ix = 0;
	int left_ix_is_set = 0;
	int right_ix = 0;

	for(int ix = 0; ix < list_size; ix++){
		if(list[ix]){
			if(!left_ix_is_set){
				left_ix = ix;
				left_ix_is_set = TRUE;
			}
			if(right_ix < ix) right_ix = ix;
		}
	}

	switch(first_or_last){
		case LAST_TRUE_IN_ARRAY:{
			return right_ix; //LAST
			break;
		}
		case FIRST_TRUE_IN_ARRAY:{
			return left_ix; //FIRST
			break;
		}
		default:{
			printf("Backoff type not found!\n");
			exit(EXIT_FAILURE);
			break;
		}
	}
}

int GetNumberOfSpecificElementInArray(int value, int* array, int list_size){

	int num = 0;

	for(int i=0; i<list_size; i++){
		if(array[i]==value) num++;
	}

	return num;

}

double RandomDouble(double min, double max)
{
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}
