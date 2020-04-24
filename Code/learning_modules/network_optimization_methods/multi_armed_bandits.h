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
 */

 /**
 * multi_armed_bandits.h: this file contains functions related to the agents' operation
 *
 *  - This file contains the methods used by the multi-armed bandits (MAB) framework
 */

#include "../../list_of_macros.h"

#ifndef _AUX_MABS_
#define _AUX_MABS_

class MultiArmedBandit {

	// Public items
	public:

        // Print and write logs
        int save_logs;				///> Boolean for saving logs
        int print_logs;				///> Boolean for printing logs

        // General information
		int agent_id;						///> Identified of the agent using MABs
		int num_arms;					///> Number of actions
		int action_selection_strategy;		///> Index of the chosen action-selection strategy

		// Generic variables to all the learning strategies
		int initial_reward;		///> Initial reward
		int num_iterations;		///> Total number of iterations allowed

		// Arms statistics
		double *reward_per_arm;					///> Array containing the reward obtained per each arm
		double *cumulative_reward_per_arm;		///> Array containing the cumulative reward obtained per each arm
		double *average_reward_per_arm;			///> Array containing the average reward obtained per each arm
		double *estimated_reward_per_arm;		///> Array containing the estimated reward obtained per each arm
		int *times_arm_has_been_selected;		///> Array containing the times each arm is selected

		// e-greedy specific variables
		double initial_epsilon;		///> Initial epsilon parameter (exploration coefficient)
		double epsilon;				///> Epsilon parameter (exploration coefficient)

		// Thompsong sampling - Beta distribution
		double *alpha;
		double *beta;
		double kappa_beta;

		// EXP3
		double exp3_gamma;						///> EXP3's exploration rate
		double exp3_eta_original;						///> EXP3's learning rate
		double *exp3_prob_arm;					///> Array containing the prob of picking each arm
		double *exp3_weight_arm;				///> Array containing the weight of each arm

	// Methods
	public:

		/******************/
		/******************/
		/*  MAIN METHODS  */
		/******************/
		/******************/

		/**
		* Update the statistics maintained for each arm
		* @param "action_ix" [type int]: index of the action to be updated
		* @param "reward" [type double]: last reward observed from the action of interest
		*/
		void UpdateArmStatistics(int action_ix, double reward){

			//printf("reward %f\n", reward);

			if(action_ix >= 0) { // Avoid indexing errors
				// Update the reward for the chosen arm
				reward_per_arm[action_ix] = reward;
				// Update the times the chosen arm has been selected
				times_arm_has_been_selected[action_ix] += 1;
				// Update the cumulative reward for the chosen arm
				cumulative_reward_per_arm[action_ix] += reward;
				// Update the average reward for the chosen arm
				average_reward_per_arm[action_ix] = cumulative_reward_per_arm[action_ix] /
					times_arm_has_been_selected[action_ix];

				// Update the estimated reward per arm (Gaussian)
				estimated_reward_per_arm[action_ix] = ((estimated_reward_per_arm[action_ix]
					* times_arm_has_been_selected[action_ix])
					+ reward) / (times_arm_has_been_selected[action_ix] + 2);

				// Beta distribution
				alpha[action_ix] = alpha[action_ix] + kappa_beta * reward;
				beta[action_ix] = beta[action_ix] + kappa_beta * (1 - reward);

				// EXP3
				exp3_weight_arm[action_ix] *= exp(reward/exp3_prob_arm[action_ix] / (double) num_arms);

				double sum_weights = sum_array(num_arms, exp3_weight_arm);

				exp3_prob_arm[action_ix] = (1 - exp3_gamma)
						* exp3_weight_arm[action_ix]/sum_weights + exp3_gamma / (double) num_arms;



			} else {
				printf("[MAB] ERROR: The action ix (%d) is not correct!\n", action_ix);
				exit(EXIT_FAILURE);
			}
		}

		/**
		* Select a new action according to the chosen action selection strategy
		* @return "action_ix" [type int]: index of the selected action
		*/
		int SelectNewAction(int *available_arms, int current_arm) {
			int arm_ix;

			// Select an action according to the chosen strategy: TODO improve this part (now it is hardcoded)
			//action_selection_strategy = STRATEGY_EGREEDY;
			switch(action_selection_strategy) {

				/*
				 * epsilon-greedy strategy:
				 */
				case STRATEGY_EGREEDY:{
					// Update epsilon
					epsilon = initial_epsilon / sqrt( (double) num_iterations);
					// Pick an action according to e-greedy
                    arm_ix = PickArmEgreedy(num_arms, reward_per_arm, epsilon, available_arms);
					break;
				}

				case STRATEGY_EXPLORATION_FIRST:{


					// Pick an action according to e-greedy
					arm_ix = PickArmEgreedySequential(num_arms, reward_per_arm, available_arms);
					break;

				}

				/*
				 * Thompson sampling strategy:
				 */
				case STRATEGY_THOMPSON_SAMPLING:{
					// Pick an action according to Thompson sampling
                    arm_ix = PickArmThompsonSampling(num_arms, estimated_reward_per_arm, times_arm_has_been_selected, available_arms);
					break;
				}

				/*
				 * Thompson sampling strategy:
				 */
				case STRATEGY_THOMPSON_SAMPLING_BETA:{
					// Pick an action according to Thompson sampling
					arm_ix = PickArmThompsonSamplingBeta(num_arms, estimated_reward_per_arm, times_arm_has_been_selected, available_arms);
					break;
				}

				/*
				 * UCB strategy:
				 */
				case STRATEGY_UCB:{
					// Pick an action according to UCB
					arm_ix = PickArmUCB(num_arms, num_iterations, cumulative_reward_per_arm,
							times_arm_has_been_selected, available_arms);
					break;
				}

				/*
				 * EXP3 strategy:
				 */
				case STRATEGY_EXP3:{
					// Pick an action according to UCB
					arm_ix = PickArmEXP3(num_arms, exp3_prob_arm, available_arms);
					break;
				}

                /*
                 * Sequential sampling strategy:
                 */
                case STRATEGY_SEQUENTIAL:{
                    // Pick an action according to Thompson sampling
                    arm_ix = PickArmSequentially(num_arms, available_arms, current_arm);
                    break;
                }

				default:{
					printf("[MAB] ERROR: '%d' is not a correct action-selection strategy!\n", action_selection_strategy);
					PrintAvailableActionSelectionStrategies();
					exit(EXIT_FAILURE);
				}
			}
            // Increase the number of iterations
            ++ num_iterations;

//            printf("\n action #%d: \n", arm_ix);

            // Return the selected action
			return arm_ix;
		}

		/****************************/
		/****************************/
		/*  EPSILON-GREEDY METHODS  */
		/****************************/
		/****************************/

		/**
		 * Select an action according to the epsilon-greedy strategy
		 * @param "num_arms" [type int]: number of possible actions
		 * @param "reward_per_arm" [type double]: array containing the last stored reward for each action
		 * @param "epsilon" [type double]: current exploration coefficient
		 * @return "action_ix" [type int]: index of the selected action
		 */
		int PickArmEgreedy(int num_arms, double *reward_per_arm, double epsilon, int *available_arms) {

			double rand_number = ((double) rand() / (RAND_MAX));
			int action_ix;

			if (rand_number < epsilon) { //EXPLORE
				action_ix = rand() % num_arms;
				int counter(0);
				while (!available_arms[action_ix]) {
					action_ix = rand() % num_arms;
					if(counter > 1000) break; // To avoid getting stuck (none of the actions is available)
				}
//				printf("EXPLORE: Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			} else { //EXPLOIT
				double max = 0;
				for (int i = 0; i < num_arms; i ++) {
					if(available_arms[i] && reward_per_arm[i] >= max) {
						max = reward_per_arm[i];
						action_ix = i;
					}
				}
//				printf("EXPLOIT: Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			}


//			if(num_iterations == 100){
//				printf("\n epsilon-greedy - val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", reward_per_arm[i]);
//
//				}
//				printf("\n");
//			}

			return action_ix;

		}

		int PickArmEgreedySequential(int num_arms, double *reward_per_arm, int *available_arms) {

			int action_ix = -1;

			int unexplored_action_flag = FALSE;

//			printf("epsilon-greedy-sequential: ");
//			for (int i = 0; i < num_arms; i++){
//
//				printf("   %.2f", reward_per_arm[i]);
//
//			}
//			printf("\n");


			for (int i = 0; i < num_arms; i ++) {
				if(available_arms[i] && reward_per_arm[i] == -1) {
					action_ix = i;
					unexplored_action_flag = TRUE;
					break;
				}
			}

			if(!unexplored_action_flag){

				double max = -1;
				for (int i = 0; i < num_arms; i ++) {
					if(available_arms[i] && reward_per_arm[i] > max) {
						max = reward_per_arm[i];
						action_ix = i;
					}
				}
			}


//				printf("EXPLOIT: Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);


//			if(num_iterations == 100){
//				printf("\n epsilon-greedy - val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", reward_per_arm[i]);
//
//				}
//				printf("\n");
//			}

//			printf("U(%d) action: %d\n", unexplored_action_flag, action_ix);

			return action_ix;

		}

		/*******************************/
		/*******************************/
		/*  UCB METHODS  */
		/*******************************/
		/*******************************/
		int PickArmUCB(int num_arms, int num_iterations,
				double *cumulative_reward_per_arm, int *times_arm_has_been_selected, int *available_arms){

			int action_ix;
			double max = 0;
			// Compute argument metric
			double *arg_value = new double[num_arms];

			for (int i = 0; i < num_arms; i ++) {

				arg_value[i] = cumulative_reward_per_arm[i] / (times_arm_has_been_selected[i]+1) +
						sqrt((2*log(num_iterations)/ (times_arm_has_been_selected[i]+1)));

				// argmax of arg_value
				if(available_arms[i] && arg_value[i] >= max) {
					max = arg_value[i];
					action_ix = i;
				}
			}

//			if(num_iterations == 100){
//				printf("\n UCB val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", arg_value[i]);
//
//				}
//				printf("\n");
//			}

//			printf("\n");
//			printf("- max = %.2f - \n", max);
			return action_ix;
		}

		/*******************************/
		/*******************************/
		/*  EXP3 METHODS  */
		/*******************************/
		/*******************************/
		int PickArmEXP3(int num_arms, double *exp3_prob_arm, int* available_arms){


//			printf("\n----------------\n");
//			for (int i = 0; i < num_arms; i++){
//
//				printf("   %.2f", exp3_weight_arm[i]);
//
//			}
//			printf("\n");
//			for (int i = 0; i < num_arms; i++){
//
//				printf("   %.2f", exp3_prob_arm[i]);
//
//			}
//			printf("\n----------------\n");

//			if(num_iterations == 100){
//				printf("\n exp3 val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", exp3_weight_arm[i]);
//
//				}
//				printf("\n");
//				printf("\n val_par_array_2: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", exp3_prob_arm[i]);
//
//				}
//				printf("\n");
//			}

			int action_ix = random_weighted(num_arms, exp3_prob_arm);


			return action_ix;

		}

		double sum_array(int array_size, double* array){

			double sum = 0;
			for(int i = 0; i<array_size; i++){
				sum += array[i];
			}
			return sum;
		}

		/**
		 * Picks a random element according to its weighted probability
		 * @param "size_array" [type int]: size of the array
		 * @param "freq" [type int*]: array with the counts (or weight) of each element
		 * @return [type int]: selected element index
		 * Note: SUCCESSFULLY CHECKED IN MATLAB THROUGH HISTOGRAM
		 */
		int random_weighted(int size_array, double* freq) {

			int total_frequency = 0;
			int selected = -1;

			for(int i = 0; i<size_array; i++){
				total_frequency += (int) (freq[i] * 100000);
			}

			int num = rand() % total_frequency; // int between 0 and total_frequency - 1

			int count_aux = 0;

			for(int i = 0; i<size_array; i++){

				count_aux += (int) (freq[i] * 100000);

				if(count_aux > num){
					selected = i;
					break;
				}

			}

			return selected;

		}


		/*******************************/
		/*******************************/
		/*  THOMPSON SAMPLING METHODS  */
		/*******************************/
		/*******************************/

		double gaussrand(double mean, double std){

			static double V1, V2, S;
			static int phase = 0;
			double X;
			if(phase == 0) {
				do {
					double U1 = (double)rand() /  RAND_MAX;
					double U2 = (double)rand() /  RAND_MAX;
					V1 = 2*U1 - 1;
					V2 = 2*U2 - 1;
					S = V1 * V1 + V2 * V2;
				} while (S >= 1 || S == 0);
				X = (V1 * sqrt(-2 * log(S) / S)) * std + mean;
			} else {
				X = (V1 * sqrt(-2 * log(S) / S)) * std + mean;
			}
			phase = 1 - phase;
			return X;
		}


		/**
		 * Generate random number from normal distribution
		 * @param "mu" [type double]: mean value
		 * @param "sigma" [type double]: standard deviation
		 * @return [type int]: random number following normal (gaussian) distribution
		 * Note: SUCCESSFULLY CHECKED AGAINST MATLAB RANDOM GENERATOR randn()
		 */
		double randn(double mu, double sigma) {

		  double U1, U2, W, mult;
		  static double X1, X2;
		  static int call = 0;

		  if (call == 1)
			{
			  call = !call;
			  return (mu + sigma * (double) X2);
			}

		  do
			{
			  U1 = -1 + ((double) rand () / RAND_MAX) * 2;
			  U2 = -1 + ((double) rand () / RAND_MAX) * 2;
			  W = pow (U1, 2) + pow (U2, 2);
			}
		  while (W >= 1 || W == 0);

		  mult = sqrt ((-2 * log (W)) / W);
		  X1 = U1 * mult;
		  X2 = U2 * mult;

		  call = !call;

		  return (mu + sigma * (double) X1);
		}

		// This is the heart of the generator.
		// It uses George Marsaglia's MWC algorithm to produce an unsigned integer.
		// See http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
		uint GetUint()
		{

			uint m_w = 521288629;
			uint m_z = 362436069;

			m_z = 36969 * (m_z & 65535) + (m_z >> 16);
			m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			return (m_z << 16) + m_w;
		}

		// Produce a uniform random sample from the open interval (0, 1).
		// The method will not return either end point.
		double GetUniform()
		{
			// 0 <= u < 2^32
			uint u = GetUint();
			// The magic number below is 1/(2^32 + 2).
			// The result is strictly between 0 and 1.
			return (u + 1.0) * 2.328306435454494e-10;
		}

		double GetGamma(double shape, double scale)
		{
			// Implementation based on "A Simple Method for Generating Gamma Variables"
			// by George Marsaglia and Wai Wan Tsang.  ACM Transactions on Mathematical Software
			// Vol 26, No 3, September 2000, pages 363-372.

			double d, c, x, xsquared, v, u;

			if (shape >= 1.0)
			{
				d = shape - 1.0/3.0;
				c = 1.0/sqrt(9.0*d);
				for (;;)
				{
					do
					{
						x = randn(0,1);
						v = 1.0 + c*x;
					}
					while (v <= 0.0);
					v = v*v*v;
					u = GetUniform();
					xsquared = x*x;
					if (u < 1.0 -.0331*xsquared*xsquared || log(u) < 0.5*xsquared + d*(1.0 - v + log(v)))
						return scale*d*v;
				}
			}
			else if (shape <= 0.0)
			{
				std::stringstream os;
				os << "Shape parameter must be positive." << "\n"
				   << "Received shape parameter " << shape;
				throw std::invalid_argument( os.str() );
			}
			else
			{
				double g = GetGamma(shape+1.0, 1.0);
				double w = GetUniform();
				return scale*g*pow(w, 1.0/shape);
			}
		}

		double GetBeta(double a, double b)
		{

			// There are more efficient methods for generating beta samples.
			// However such methods are a little more efficient and much more complicated.
			// For an explanation of why the following method works, see
			// http://www.johndcook.com/distribution_chart.html#gamma_beta

			double u = GetGamma(a, 1.0);
			double v = GetGamma(b, 1.0);
			return u / (u + v);
		}

		/**
		 * Select an action according to the Thompson sampling strategy
		 * @param "num_arms" [type int]: number of possible actions
		 * @param "estimated_reward_per_arm" [type double*]: array containing the estimated reward for each action
		 * @param "times_arm_has_been_selected" [type int*]: array containing the times each action has been selected
		 * @return "available_arms" [type int*]: array of the indeces of the permitted arms
		 */
		int PickArmThompsonSampling(int num_arms, double *estimated_reward_per_arm,
			int *times_arm_has_been_selected, int *available_arms) {

			//TODO: validate the behavior of this implementation
			int action_ix = -1;
			double *theta = new double[num_arms];
			for(int i = 0; i < num_arms; ++i){
				theta[i] = 0;
			}

			double *std = new double[num_arms];


			// Compute the posterior probability of each arm
			for (int i = 0; i < num_arms; ++i) {


				if (available_arms[i]) {

					// Normal distribution
					std[i] = 1.0/(1+times_arm_has_been_selected[i]);
					theta[i] = randn(estimated_reward_per_arm[i], std[i]);


				} else {
					theta[i] = -10000;
				}

			}


			// Find the action with the highest likelihood
			double max = theta[0];
			for (int i = 0; i < num_arms; ++i) {
				if(theta[i] >= max) {
					max = theta[i];
					action_ix = i;
				}
				//  TODO: elseif(theta[i] == max) --> Break ties!
			}

			if(action_ix == -1){
				printf("action_ix = %d invalid!\n",
									action_ix);
				exit(-1);
			}


//			if(num_iterations == 100){
//				printf("\n T.S. normal val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", estimated_reward_per_arm[i]);
//
//				}
//				printf("\n");
//
//				printf("\n val_par_array_2: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", std[i]);
//
//				}
//				printf("\n");
//			}


//			printf("Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			return action_ix;
		}

		/**
		 * Select an action according to the Thompson sampling strategy with beta distribution
		 * @param "num_arms" [type int]: number of possible actions
		 * @param "estimated_reward_per_arm" [type double*]: array containing the estimated reward for each action
		 * @param "times_arm_has_been_selected" [type int*]: array containing the times each action has been selected
		 * @return "available_arms" [type int*]: array of the indeces of the permitted arms
		 */
		int PickArmThompsonSamplingBeta(int num_arms, double *estimated_reward_per_arm,
			int *times_arm_has_been_selected, int *available_arms) {

			//TODO: validate the behavior of this implementation
			int action_ix = -1;
			double *theta = new double[num_arms];
			for(int i = 0; i < num_arms; ++i){
				theta[i] = 0;
			}



			// Compute the posterior probability of each arm
			for (int i = 0; i < num_arms; ++i) {


				if (available_arms[i]) {

//					// Beta distribution
					theta[i] = GetBeta(alpha[i], beta[i]);

				} else {
					theta[i] = -10000;
				}

			}


			// Find the action with the highest likelihood
			double max = theta[0];
			for (int i = 0; i < num_arms; ++i) {
				if(theta[i] >= max) {
					max = theta[i];
					action_ix = i;
				}
				//  TODO: elseif(theta[i] == max) --> Break ties!
			}

			if(action_ix == -1){
				printf("action_ix = %d invalid!\n",
									action_ix);
				exit(-1);
			}

//			if(num_iterations == 100){
//				printf("\n T.S. Beta val_par_array: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", alpha[i]);
//
//				}
//				printf("\n");
//
//				printf("\n val_par_array_2: ");
//				for (int i = 0; i < num_arms; i++){
//
//					printf("   %.2f", beta[i]);
//
//				}
//				printf("\n");
//			}



//			printf("Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			return action_ix;
		}

        /*******************/
        /*******************/
        /*  OTHER METHODS  */
        /*******************/
        /*******************/


        /**
         * Select an action according to the Thompson sampling strategy
         * @param "num_arms" [type int]: number of possible actions
         * @param "estimated_reward_per_arm" [type double*]: array containing the estimated reward for each action
         * @param "times_arm_has_been_selected" [type int*]: array containing the times each action has been selected
         * @return "action_ix" [type int]: index of the selected action
         */
        int PickArmSequentially(int num_arms, int *available_arms, int current_arm_ix) {
            int arm_ix(1);
            if (num_iterations == 1) {
                arm_ix = num_iterations;
            } else {
                arm_ix = (current_arm_ix + 1) % num_arms;
            }
            if (available_arms[arm_ix] != 1) {
                arm_ix = PickArmSequentially(num_arms, available_arms, arm_ix);
            }
            return arm_ix;
        }

		/*************************/
		/*************************/
		/*  PRINT/WRITE METHODS  */
		/*************************/
		/*************************/

		/**
		* Print or write the statistics of each arm
		* @param "write_or_print" [type int]: variable to indicate whether to print on the  console or to write on the the output logs file
		* @param "logger" [type Logger]: logger object to write on the output file
		* @param "sim_time" [type double]: simulation time
		*/
		void PrintOrWriteStatistics(int write_or_print, Logger &logger, double sim_time) {
			// Write or print according the input parameter "write_or_print"
			switch(write_or_print){
				// Print logs in console
				case PRINT_LOG:{
					if(print_logs){
						printf("%s Reward per arm: ", LOG_LVL3);
						for(int n = 0; n < num_arms; n++){
							printf("%f  ", reward_per_arm[n]);
						}
						printf("\n%s Cumulative reward per arm: ", LOG_LVL3);
						for(int n = 0; n < num_arms; n++){
							printf("%f  ", cumulative_reward_per_arm[n]);
						}
						printf("\n%s Times each arm has been selected: ", LOG_LVL3);
						for(int n = 0; n < num_arms; n++){
							printf("%d  ", times_arm_has_been_selected[n]);
						}
						printf("\n");
					}
					break;
				}
				// Write logs in agent's output file
				case WRITE_LOG:{
					if(save_logs) fprintf(logger.file, "%.15f;A%d;%s;%s Reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_arms; n++){
						 if(save_logs){
							 fprintf(logger.file, "%f  ", reward_per_arm[n]);
						 }
					}
					if(save_logs) fprintf(logger.file, "\n%.15f;A%d;%s;%s Cumulative reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_arms; n++){
						 if(save_logs){
							 fprintf(logger.file, "%f  ", cumulative_reward_per_arm[n]);
						 }
					}
					fprintf(logger.file, "\n%.15f;A%d;%s;%s Times each arm has been selected: ",
									sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_arms; n++){
						if(save_logs){
							fprintf(logger.file, "%d ", times_arm_has_been_selected[n]);
						}
					}
					if(save_logs) fprintf(logger.file, "\n");
					break;
				}
			}
		}

		/**
		 * Print the available ML mechanisms types
		 */
		void PrintAvailableActionSelectionStrategies(){
			printf("%s Available types of action-selection strategies:\n", LOG_LVL2);
			printf("%s STRATEGY_EGREEDY (%d)\n", LOG_LVL3, STRATEGY_EGREEDY);
			printf("%s STRATEGY_THOMPSON_SAMPLING (%d)\n", LOG_LVL3, STRATEGY_THOMPSON_SAMPLING);
		}

		/***********************/
		/***********************/
		/*  AUXILIARY METHODS  */
		/***********************/
		/***********************/

		/**
		 * Initialize the variables used by the Bandits framework
		 */
		void InitializeVariables(){
			// TODO: generate file that stores algorithm-specific variables
			initial_epsilon = 1;
			epsilon = initial_epsilon;

			if(action_selection_strategy == STRATEGY_EXPLORATION_FIRST){
				initial_reward = -1;
			} else {
				initial_reward = 0;
			}

			num_iterations = 1;

			kappa_beta = 1;
			exp3_gamma = 0.07;				///> EXP3's exploration rate
			// Initialize the rewards assigned to each arm
			reward_per_arm = new double[num_arms];
			cumulative_reward_per_arm = new double[num_arms];
			average_reward_per_arm = new double[num_arms];
			estimated_reward_per_arm = new double[num_arms];
			alpha = new double[num_arms];
			beta = new double[num_arms];
			exp3_prob_arm = new double[num_arms];
			exp3_weight_arm = new double[num_arms];
			// Initialize the array containing the times each arm has been played
			times_arm_has_been_selected = new int[num_arms];
			for(int i = 0; i < num_arms; ++i){
				reward_per_arm[i] = initial_reward;	// Set the initial reward
				cumulative_reward_per_arm[i] = initial_reward;
				average_reward_per_arm[i] = initial_reward;
				estimated_reward_per_arm[i] = initial_reward;
				alpha[i] = 1;
				beta[i] = 1;
				times_arm_has_been_selected[i] = 0;
				exp3_weight_arm[i] = 1;
				exp3_prob_arm[i] = (1 - exp3_gamma)
					* 1 /  (double) num_arms + exp3_gamma / (double) num_arms;
			}

		}

};

#endif
