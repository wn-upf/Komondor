/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
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
 * Updated : $Date: 2026/04/08 00:00:00 $
 *           $Revision: 1.0 $
 *
 * -----------------------------------------------------------------
 */

/**
 * logging_methods.h: generic PrintOrWrite helpers for arrays and token lists.
 *
 * Canonical parameter order (matches channel PrintOrWrite helpers):
 *   (write_or_print, save_node_logs, print_node_logs, Logger, ...data...)
 */

#ifndef _LOGGING_METHODS_
#define _LOGGING_METHODS_

/**
 * Print to console or write to log file the elements of an integer array.
 * @param "write_or_print"  [type int]:    PRINT_LOG or WRITE_LOG
 * @param "save_node_logs"  [type int]:    flag: write logs to file
 * @param "print_node_logs" [type int]:    flag: print logs to console
 * @param "node_logger"     [type Logger]: logger object
 * @param "list"            [type int*]:   array of integers
 * @param "list_size"       [type int]:    number of elements
 */
void PrintOrWriteArrayInt(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int *list, int list_size) {

	switch(write_or_print){

		case PRINT_LOG:{
			if(print_node_logs){
				for(int c = 0; c < list_size; ++c){
					printf("%d  ", list[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < list_size; ++c){
				if(save_node_logs)  fprintf(node_logger.file, "%d  ", list[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/**
 * Print to console or write to log file the elements of a double array.
 * @param "write_or_print"  [type int]:    PRINT_LOG or WRITE_LOG
 * @param "save_node_logs"  [type int]:    flag: write logs to file
 * @param "print_node_logs" [type int]:    flag: print logs to console
 * @param "node_logger"     [type Logger]: logger object
 * @param "list"            [type double*]: array of doubles
 * @param "list_size"       [type int]:    number of elements
 */
void PrintOrWriteArrayDouble(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, double *list, int list_size) {

	switch(write_or_print){

		case PRINT_LOG:{
			if(print_node_logs){
				for(int c = 0; c < list_size; ++c){
					printf("%.2f  ", list[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < list_size; ++c){
				if(save_node_logs)  fprintf(node_logger.file, "%f  ", list[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/**
 * Print to console or write to log file the elements of a token order list.
 * @param "write_or_print"     [type int]:    PRINT_LOG or WRITE_LOG
 * @param "save_node_logs"     [type int]:    flag: write logs to file
 * @param "print_node_logs"    [type int]:    flag: print logs to console
 * @param "node_logger"        [type Logger]: logger object
 * @param "token_order_list"   [type int*]:   ordered list of neighboring device identifiers
 * @param "length_order_list"  [type int]:    length of the token list
 */
void PrintOrWriteTokenList(int write_or_print, int save_node_logs, int print_node_logs,
	Logger node_logger, int *token_order_list, int length_order_list){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				for(int i = 0; i < length_order_list; ++i){
					printf("%d  ", token_order_list[i]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int i = 0; i < length_order_list; ++i){
				if(save_node_logs) fprintf(node_logger.file, "%d  ", token_order_list[i]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

#endif /* _LOGGING_METHODS_ */
