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
		double initial_reward;		///> Initial reward
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

		// Regret maching specific variables
		double **regret_matrix;         ///> Matrix Q [current_action][other_action]
		double *estimated_reward_rm;    ///> Estimated rewards for regret matching
		double *action_probs_rm;        ///> Array with probabilities to select each action
		double mu;                		///> Inertia parameter
		double S_CCA_DBM;       		///> Standard preamble detection threshold
		double MAX_THROUGHPUT_MBPS;  	///> Set this to your max expected Mbps

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

			if(action_ix >= 0) { // Avoid indexing errors
				// Update the reward for the chosen arm
				reward_per_arm[action_ix] = reward;
				// Update the times the chosen arm has been selected
				++times_arm_has_been_selected[action_ix];
				// Update the cumulative reward for the chosen arm
				cumulative_reward_per_arm[action_ix] += reward;
				// Update the average reward for the chosen arm
				average_reward_per_arm[action_ix] = cumulative_reward_per_arm[action_ix] /
					times_arm_has_been_selected[action_ix];
				// Update the estimated reward per arm
				//estimated_reward_per_arm[action_ix] = cumulative_reward_per_arm[action_ix] /
				//		(times_arm_has_been_selected[action_ix] + 1);
				estimated_reward_per_arm[action_ix] = ( (estimated_reward_per_arm[action_ix]*(times_arm_has_been_selected[action_ix]-1))
														+ reward ) / ( (times_arm_has_been_selected[action_ix]-1) + 2 );
				//(estimated_reward_per_arm[action_ix] + reward)
				//								/ (double)(times_arm_has_been_selected[action_ix] + 2);
			} else {
				printf("[MAB] ERROR: The action ix (%d) is not correct!\n", action_ix);
				exit(EXIT_FAILURE);
			}
		}

		void UpdateRegretMatching(int selected_action_ix, double reward, double *estimated_performance_per_action) {

			// 1) Update estimated rewards
			for (int k = 0; k < num_arms; k ++) {
				if (k == selected_action_ix) {
					estimated_reward_rm[k] = reward;
				} else {
					estimated_reward_rm[k] = estimated_performance_per_action[k];
				}
			}

			// 2) Update regret matrix (Q)
			for (int k = 0; k < num_arms; k++) {
				regret_matrix[selected_action_ix][k] +=
					action_probs_rm[selected_action_ix]*(estimated_reward_rm[k] - estimated_reward_rm[selected_action_ix]);
				//regret_matrix[selected_action_ix][k] += (estimated_reward_rm[k] - estimated_reward_rm[selected_action_ix]);
			}

			// 3) Update action probabilities

			//   3.1) Calculate sum of the positive part of the regrets for unplayed actions
			double sum_positive_regrets = 0.0;
			for (int k = 0; k < num_arms; k++) {
				if (k != selected_action_ix) {
					double q_val = regret_matrix[selected_action_ix][k];
					if (q_val > 0) sum_positive_regrets += q_val;
				}
			}

			//   3.2) Update the probabilities
			for (int k = 0; k < num_arms; k++) {
				if (k != selected_action_ix) {
					// Switching probability
					double q_val = regret_matrix[selected_action_ix][k];
					action_probs_rm[k] = (q_val > 0) ? (q_val / mu) : 0.0;
				} else {
					// Staying probability (inertia)
					action_probs_rm[k] = 1.0 - (sum_positive_regrets / mu);
				}
			}

		}

		double EstimatePerformanceRegretMatching(int action_ix) {

			//double candidate_power = ;
			//double candidate_sensitivity = ;

			// TO BE DONE
			return 0.0;

		}

		/**
		* Select a new action according to the chosen action selection strategy
		* @return "action_ix" [type int]: index of the selected action
		*/
		int SelectNewAction(int *available_arms, int current_arm) {
			int arm_ix;
			// Select an action according to the chosen strategy
			switch(action_selection_strategy) {
				/*
				 * epsilon-greedy strategy:
				 */
				case STRATEGY_EGREEDY:{
					// Update epsilon
					epsilon = initial_epsilon / sqrt( (double) num_iterations);
					// Pick an action according to e-greedy
                    arm_ix = PickArmEgreedy(num_arms, average_reward_per_arm, epsilon, available_arms);
                    //printf("Action selected = %d\n", arm_ix);
					break;
				}
				/*
				 * Thompson sampling strategy:
				 */
				case STRATEGY_THOMPSON_SAMPLING:{
					// Pick an action according to Thompson sampling
                    arm_ix = PickArmThompsonSampling(num_arms,
						estimated_reward_per_arm, times_arm_has_been_selected, available_arms);
					break;
				}
				/*
				 * Upper Confidence Bound (UCB):
				 */
				case STRATEGY_UCB:{
					// Pick an action according to Thompson sampling
					arm_ix = PickArmUCB(num_arms, average_reward_per_arm,
							times_arm_has_been_selected, available_arms, num_iterations);
					break;
				}
                /*
                 * Sequential selection strategy:
                 */
                case STRATEGY_SEQUENTIAL:{
                    // Pick an action according to Thompson sampling
                    arm_ix = PickArmSequentially(num_arms, available_arms, current_arm);
                    break;
                }
				/*
				 * Regret matching strategy:
				 */
				case STRATEGY_REGRET_MATCHING:{
					// Pick an action according to Regret Matching
					arm_ix = PickArmRegretMatching();
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

			double rand_number = ((double) rand() / (double)RAND_MAX);
			int action_ix;

			if (rand_number < epsilon) { //EXPLORE
				action_ix = rand() % num_arms;
				int counter(0);
				while (!available_arms[action_ix]) {
					action_ix = rand() % num_arms;
					if(counter > 1000) break; // To avoid getting stuck (none of the actions is available)
				}
				//printf("EXPLORE: Selected action %d (available = %d), reward = %f\n", action_ix, available_arms[action_ix], reward_per_arm[action_ix]);
			} else { //EXPLOIT
				double max = 0;
				for (int i = 0; i < num_arms; i ++) {
					//printf("   - reward_per_arm[%d] = %f\n", i, reward_per_arm[i]);
					if(available_arms[i] && reward_per_arm[i] >= max) {
						max = reward_per_arm[i];
						action_ix = i;
					}
				}
				//printf("EXPLOIT: Selected action %d (available = %d), reward = %f\n", action_ix, available_arms[action_ix], reward_per_arm[action_ix]);
			}
			return action_ix;

		}

		/*******************************/
		/*******************************/
		/*  THOMPSON SAMPLING METHODS  */
		/*******************************/
		/*******************************/

		double rand_normal() {
		  double u1 = (double)rand() / RAND_MAX;
		  double u2 = (double)rand() / RAND_MAX;
		  return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
		}

		double sample_rand_normal(double mu, double sigma) {
		  return mu + sigma * rand_normal();
		}

		/**
		 * Select an action according to the Thompson sampling strategy
		 * @param "num_arms" [type int]: number of possible actions
		 * @param "estimated_reward_per_arm" [type double*]: array containing the estimated reward for each action
		 * @param "times_arm_has_been_selected" [type int*]: array containing the times each action has been selected
		 * @return "action_ix" [type int]: index of the selected action
		 */
		int PickArmThompsonSampling(int num_arms, double *estimated_reward_per_arm,
			int *times_arm_has_been_selected, int *available_arms) {
			//TODO: validate the behavior of this implementation
			int action_ix(-1);
			double *theta = new double[num_arms];
			double std;
			int KMAX(1);
			// Compute the posterior probability of each arm
			for (int i = 0; i < num_arms; ++i) {
				if (available_arms[i]) {
					std = 1.0/(double)(1+times_arm_has_been_selected[i]);
					theta[i] = 0;
					for (int k = 0; k < KMAX; ++k){
						theta[i] += sample_rand_normal(estimated_reward_per_arm[i], std);
					}
				}
			}
			// Find the action with the highest likelihood
			double max = -10000;
			for (int i = 0; i < num_arms; ++i) {
				if(theta[i] > max && available_arms[i]) {
					max = theta[i];
					action_ix = i;
				}
				//  TODO: elseif(theta[i] == max) --> Break ties!
			}
//			printf("Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			return action_ix;
		}

		/**
		 * Select an action according to the UCB sampling strategy
		 * @param "num_arms" [type int]: number of possible actions
		 * @param "estimated_reward_per_arm" [type double*]: array containing the estimated reward for each action
		 * @param "times_arm_has_been_selected" [type int*]: array containing the times each action has been selected
		 * @return "action_ix" [type int]: index of the selected action
		 */
		int PickArmUCB(int num_arms, double *average_reward_per_arm,
			int *times_arm_has_been_selected, int *available_arms, int num_iterations) {
			//TODO: validate the behavior of this implementation
			int action_ix(-1);
			double *ucb_estimate = new double[num_arms];
			double max = -10000;
			// Compute the posterior probability of each arm
			for (int i = 0; i < num_arms; ++i) {
				if (available_arms[i]) {
					ucb_estimate[i] = average_reward_per_arm[i] +
							sqrt((2*log(num_iterations))/times_arm_has_been_selected[i]);
					if (ucb_estimate[i] > max) {
						max = ucb_estimate[i];
						action_ix = i;
					}
				}
			}
//			printf("Selected action %d (available = %d)\n", action_ix, available_arms[action_ix]);
			return action_ix;
		}

		/**
		 * Select an action according to the Regret matching strategy
		 * @return "action_ix" [type int]: index of the selected action
		 */
		int PickArmRegretMatching() {

			double r = (double)rand() / RAND_MAX; // Random [0, 1]
			double cumulative = 0.0;
			int action_ix(-1);
			//printf("Random number selected: %f\n", r);
			for (int i = 0; i < num_arms; i++) {
				cumulative += action_probs_rm[i];
				if (r <= cumulative) {
					//printf("Action selected: %d\n", i);
					action_ix = i;
					return action_ix;
				}
			}
			return num_arms - 1; // Fallback

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
						printf("\n%s Estimated reward per arm: ", LOG_LVL3);
						for(int n = 0; n < num_arms; n++){
							printf("%f  ", estimated_reward_per_arm[n]);
						}
						if (action_selection_strategy == STRATEGY_REGRET_MATCHING) {
							printf("\n%s Probabilities per arm: ", LOG_LVL3);
							for(int n = 0; n < num_arms; n++){
								printf("%f  ", action_probs_rm[n]);
							}
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
					if(save_logs) fprintf(logger.file, "\n%.15f;A%d;%s;%s Estimated reward per arm: ",
						sim_time, agent_id, LOG_C00, LOG_LVL3);
					for(int n = 0; n < num_arms; n++){
						 if(save_logs){
							 fprintf(logger.file, "%f  ", estimated_reward_per_arm[n]);
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
			initial_reward = 0; //(double) 1/num_arms;
			num_iterations = 1;
			// Initialize the rewards assigned to each arm
			reward_per_arm = new double[num_arms];
			cumulative_reward_per_arm = new double[num_arms];
			average_reward_per_arm = new double[num_arms];
			estimated_reward_per_arm = new double[num_arms];
			// Initialize the array containing the times each arm has been played
			times_arm_has_been_selected = new int[num_arms];
			for(int i = 0; i < num_arms; ++i){
				reward_per_arm[i] = initial_reward;	// Set the initial reward
				cumulative_reward_per_arm[i] = initial_reward;
				average_reward_per_arm[i] = initial_reward;
				estimated_reward_per_arm[i] = initial_reward;
				times_arm_has_been_selected[i] = 0;
			}

			// REGRET MATCHING INITIALIZATION

			// - Initialize parameters
			S_CCA_DBM = -82.0;
			MAX_THROUGHPUT_MBPS = 400.0;
			mu = 2.0 * MAX_THROUGHPUT_MBPS * (num_arms - 1);

			// - Initialize action probabilities (uniformly) and estimated rewards
			action_probs_rm = new double[num_arms];
			estimated_reward_rm = new double[num_arms];
			for (int i = 0; i < num_arms; i++) {
				action_probs_rm[i] = 1.0 / (double)num_arms;
				estimated_reward_rm[i] = 0.0;
			}

			// - Initialize Q Matrix
			regret_matrix = new double *[num_arms];
			for (int i = 0; i < num_arms; i++) {
				regret_matrix[i] = new double[num_arms];
				for (int j = 0; j < num_arms; j++) {
					regret_matrix[i][j] = 0.0;
				}
			}

		}

};

#endif
