#include <stddef.h>

#include "../ListOfDefines.h"

/***********************/
/***********************/
/*  POWER MANAGEMENT   */
/***********************/
/***********************/

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

/*
 * computeDistance(): returns the distance between 2 points
 * Input arguments:
 * - x1,y1,z1: first point position
 * - x2,y2,z2: second point position
 * Output:
 * - distance: distance [m]
 */
double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
  double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
  return distance;
}

/*
 * computePowerReceived() returns the power received in a given distance from the transmitter depending on the path loss model
 * Input arguments:
 * - distance: distance between transceiver and received [m]
* - tx_power: TX power [dBm]
 * - tx_gain: transmitter gain [dB]
 * - rx_gain: receiver gain [dB]
 * Output arguments:
 * - Power received [dBm]
 */
double computePowerReceived(double distance, double tx_power, int tx_gain, int rx_gain, double central_frequency,
    int path_loss_model) {

  double pw_received = 0;
  double wavelength = 3*pow(10,8)/(central_frequency*pow(10,9));
  double loss;

  switch(path_loss_model){
    // Free space - Calculator: https://www.pasternack.com/t-calculator-fspl.aspx
    case PATH_LOSS_LFS:{
      loss = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);
      pw_received = tx_power + tx_gain + rx_gain - loss;
      break;
    }
    // Okumura-Hata model - Urban areas
    case PATH_LOSS_OKUMURA_HATA:{
      double tx_heigth = 10;    // Transmitter height [m]
      double rx_heigth = 10;    // Receiver height [m]
      double path_loss_A = 69.55 + 26.16 * log10(3*pow(10,8)/wavelength) - 13.82 * log10(tx_heigth);
      double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
      double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97;
      double path_loss = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
      pw_received = tx_power + tx_gain + rx_gain - path_loss;
      break;
    }
    // Indoor model (could suite an apartments building scenario)
    case PATH_LOSS_INDOOR: {
      double path_loss_factor = 5;
      double shadowing = 9.5;
      double obstacles = 30;
      double walls_frequency = 5; //  One wall each 5 meters on average
      double shadowing_at_wlan = (((double) rand())/RAND_MAX)*shadowing;
      double obstacles_at_wlan = (((double) rand())/RAND_MAX)*obstacles;
      double alpha = 4.4; // Propagation model
      double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
          (distance/walls_frequency)*obstacles_at_wlan;
      pw_received = tx_power + tx_gain - path_loss; // Power in dBm
      break;
    }
    // Indoor model without variability
    case PATH_LOSS_INDOOR_2: {
      double path_loss_factor = 5;
      double shadowing = 9.5;
      double obstacles = 30;
      double walls_frequency = 5; //  One wall each 5 meters on average
      double shadowing_at_wlan = 1/2*shadowing;
      double obstacles_at_wlan = 1/2*obstacles;
      double alpha = 4.4; // Propagation model
      double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan +
          (distance/walls_frequency)*obstacles_at_wlan;
      pw_received = tx_power + tx_gain - path_loss; // Power in dBm
      break;
    }

    // Residential - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
    // Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

    // IEEE 802.11ax uses the TGn channel B path loss model for performance evaluation of simulation scenario #1
    // with extra indoor wall and floor penetration loss.
    case PATH_LOSS_SCENARIO_1_TGax: {

      int n_walls = 10;   // Wall frequency (n_walls walls each m)
      int n_floors = 3;   // Floor frequency (n_floors floors each m)
      int L_iw = 5;     // Penetration for a single wall (dB)

      double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

      int d_BP = 5;    // Break-point distance (m)

      if (distance >= d_BP) {
        loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
      } else {
        loss = LFS;
      }

      pw_received = tx_power + tx_gain + rx_gain - loss;
      break;

    }

    // Enterprise - 5 dB/wall and 18.3 dB per floor, and 4 dB shadow
    // Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

    // IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation of simulation scenario #2
    // with extra indoor wall and floor penetration loss.
    case PATH_LOSS_SCENARIO_2_TGax: {

      int n_walls = 2;   // Wall frequency (n_walls walls each m)
      int n_floors = 3;     // Floor frequency (n_floors floors each m)
      int L_iw = 7;   // Penetration for a single wall (dB)

      double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

      int d_BP = 10;    // Break-point distance (m)

      if (distance >= d_BP) {
        loss = LFS + 35*log10(distance/d_BP) + 18.3*pow(n_floors,((n_floors+2)/(n_floors+1)) - 0.46) + L_iw*n_walls;
      } else {
        loss = LFS;
      }

      pw_received = tx_power + tx_gain + rx_gain - loss;
      break;

    }

    // Indoor small BSSs
    // Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx

    // IEEE 802.11ax uses the TGn channel D path loss model for performance evaluation
    // of simulation scenario #3.
    case PATH_LOSS_SCENARIO_3_TGax: {

      double LFS = 32.4 + 20*log10(2.4*pow(10,3))+ 20*log10(distance/1000);

      int d_BP = 10;    // Break-point distance (m)

      if (distance >= d_BP) {
        loss = LFS + 35*log10(distance/d_BP);
      } else {
        loss = LFS;
      }

      pw_received = tx_power + tx_gain + rx_gain - loss;
      break;
    }

    // Outdoor large BSS scenario
    // Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
    case PATH_LOSS_SCENARIO_4_TGax: {

      double h_AP = 10;    // Height of the AP in m
      double h_STA = 1.5;   // Height of the STA in m

      double d_BP = (4 * (h_AP - 1) * (h_STA - 1) * central_frequency * pow(10,9)) / SPEED_LIGHT;


      if (distance < d_BP && distance >= 10) {
        loss = 22 * log10(distance) + 28 + 20  *log10(central_frequency);
      } else if (distance >= d_BP && distance < 5000) {
        loss = 40 * log10(distance) + 7.8 + 18 * log10(h_AP - 1) -
            18 * log10(h_STA - 1)  + 20 * log10(central_frequency);
      }

      pw_received = tx_power + tx_gain - loss; // Power in dBm
      break;
    }

    // Outdoor large BSS scenario + Residential
    // Retrieved from: https://mentor.ieee.org/802.11/dcn/14/11-14-0882-04-00ax-tgax-channel-model-document.docx
    case PATH_LOSS_SCENARIO_4a_TGax: {

//      double h_AP = 10;    // Height of the AP in m
//      double h_STA = 1.5;   // Height of the STA in m

      if (distance < 2000 && distance >= 10) {
        loss = 36.7 * log10(distance) + 22.7 + 26  * log10(central_frequency);
      }

      // Outdoor-to-Indoor building penetration loss
      // TODO: important to consider specifying d_outdoor and d_indoor
      double d_outdoor = 0;
      double d_indoor = 0;
      loss = loss * (d_outdoor + d_indoor) + 20 + 0.5 * d_indoor;

      pw_received = tx_power + tx_gain - loss; // Power in dBm
      break;
    }

    default:{
      printf("Path loss model not found!\n");
      break;
    }

  }
  return pw_received;
}

/*
 * compute_tx_power_per_channel(): computes the transmission time (just link rate) according to the number of channels used and packet lenght
 * Input arguments:
 * - current_tpc: number of channels (OR INDEX) used in the transmission
 * - num_channels_tx: num_channels_tx of bits sent in the transmission
 */
double computeTxPowerPerChannel(double current_tpc, int num_channels_tx){

	double tx_power_per_channel = current_tpc;

	int num_channels_tx_ix = log2(num_channels_tx);

	for (int num_ch_ix = 0; num_ch_ix < num_channels_tx_ix; num_ch_ix ++){

		tx_power_per_channel -= 3;	// Half the power

	}

	return tx_power_per_channel;
}

/***********************/
/***********************/
/* CHANNELS MANAGEMENT */
/***********************/
/***********************/

/*
 * getChannelOccupancyByCCA(): (DEPECATED) indicates the channels occupied and free in a binary way
 */
void getChannelOccupancyByCCA(int *channels_free, int min_channel_allowed, int max_channel_allowed,
    double *channel_power, double current_cca){

  for(int c = min_channel_allowed; c <= max_channel_allowed; c++){

    if(channel_power[c] < convertPower(DBM_TO_PICO, current_cca)){
      channels_free[c] = CHANNEL_OCCUPIED;
    } else {
      channels_free[c] = CHANNEL_FREE;
    }
  }

}

void updatePowerReceivedPerNode(double *power_received_per_node, Notification notification,
    int x, int y, int z, double tx_power, int tx_gain, int rx_gain, int central_frequency,
    int path_loss_model, double SimTime, Logger node_logger, int save_node_logs, int node_id, int node_state) {

  double pw_received_pico;

  double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
  double pw_received = computePowerReceived(distance, tx_power, tx_gain, rx_gain,
      central_frequency, path_loss_model);

  pw_received_pico = convertPower(DBM_TO_PICO, pw_received);
  power_received_per_node[notification.source_id] = pw_received_pico;

  if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Distance to transmitting node N%d: %f m\n",
      SimTime, node_id, node_state, LOG_D04, LOG_LVL4, notification.source_id, distance);

  if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Power received from N%d: = %f dBm (%f pW)\n",
      SimTime, node_id, node_state, LOG_D04, LOG_LVL4, notification.source_id,
      convertPower(PICO_TO_DBM, pw_received_pico), pw_received_pico);

}

/*
 * applyInterferenceModel: applies the interference model
 * Arguments:
 * - notification: type of co-channel model applied (0: no co-channel interference, 1: 20 dB mask, ...)
 * - update_type: type of power update (sum or subtract)
 */
void applyInterferenceModel(double *total_power, Notification notification, Logger node_logger,
    int save_node_logs,  int num_channels_komondor, double *power_received_per_node,
    int node_id, int node_state, double SimTime, int cochannel_model){

  // Direct power (power of the channels used for transmitting)
  for(int i = notification.left_channel; i <= notification.right_channel; i++){
    total_power[i] = power_received_per_node[notification.source_id];
  }

  // Co-channel interference power
  switch(cochannel_model){

    case COCHANNEL_NONE:{
      // Do nothing
      break;
    }

    // (RECOMMENDED) Boundary co-channel interference: only boundary channels (left and right) used in the TX affect the rest of channels
    case COCHANNEL_BOUNDARY:{

      for(int c = 0; c < num_channels_komondor; c++) {

        if(c < notification.left_channel || c > notification.right_channel){

          if(c < notification.left_channel) {

            total_power[c] += convertPower(DBM_TO_PICO,
                convertPower(PICO_TO_DBM, power_received_per_node[notification.source_id])
                - 20*abs(c-notification.left_channel));

          } else if(c > notification.right_channel) {

            total_power[c] += convertPower(DBM_TO_PICO,
                convertPower(PICO_TO_DBM, power_received_per_node[notification.source_id])
                - 20*abs(c-notification.right_channel));

          }

          if(total_power[c] < MIN_VALUE_C_LANGUAGE){

            total_power[c] = 0;

          }

        } else {
          // Inside TX range --> do nothing
        }
      }
      break;
    }
    case COCHANNEL_EXTREME:{

      for(int i = 0; i < num_channels_komondor; i++) {

        for(int j = notification.left_channel; j <= notification.right_channel; j++){

          if(i != j) {

            total_power[i] += convertPower(PICO_TO_DBM, convertPower(DBM_TO_PICO,
                power_received_per_node[notification.source_id]) - 20*abs(i-j));

            if(total_power[i] < MIN_VALUE_C_LANGUAGE) total_power[i] = 0;

          }
        }
      }
      break;
    }
    default:{
      break;
    }
  }

  if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Power to sum or subtract [dBm]: ",
      SimTime, node_id, node_state, LOG_D06, LOG_LVL5);

  for(int i = 0; i < num_channels_komondor; i++) {

    if(save_node_logs) fprintf(node_logger.file,"%f ", convertPower(PICO_TO_DBM, total_power[i]));

  }
  if(save_node_logs) fprintf(node_logger.file,"\n");

}

/*
 * updateChannelsPower: updates the power sensed by the node in every channel array depending on the notifications received
 * (tx start, tx end) and the cochannel model.
 * Arguments:
 * - notification: type of co-channel model applied (no co-channel interference, 20 dB mask, ...)
 * - update_type: type of power update (sum or subtract)
 */
void updateChannelsPower(double *channel_power, double *power_received_per_node, Notification notification,
    int update_type,double SimTime, Logger node_logger, int save_node_logs, int node_id, int node_state,
    double central_frequency, int num_channels_komondor, int path_loss_model, int cochannel_model){

  if(update_type == TX_FINISHED) {
    if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Decreasing channel power sensed\n",
            SimTime, node_id, node_state, LOG_E04, LOG_LVL3);
    if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pre update channel state [dBm]: ",
        SimTime, node_id, node_state, LOG_E04, LOG_LVL4);

  } else if(update_type == TX_INITIATED) {

    if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Increasing channel power sensed\n",
                SimTime, node_id, node_state, LOG_E04, LOG_LVL3);
    if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Pre update channel state [dBm]: ",
        SimTime, node_id, node_state, LOG_D03, LOG_LVL4);

  } else {
    printf("ERROR: update_type %d does not exist!!!", update_type);
  }

  // Total power (of interest and interference) generated only by the incoming or outgoing TX
  double *total_power;
  total_power = (double *) malloc(num_channels_komondor * sizeof(*total_power));
  for(int i = 0; i < num_channels_komondor; i++) {
    total_power[i] = 0;
  }
  applyInterferenceModel(total_power, notification, node_logger,
      save_node_logs, num_channels_komondor, power_received_per_node, node_id, node_state,
      SimTime, cochannel_model);

  // Increase/decrease power sensed if TX started/finished
  for(int c = 0; c < num_channels_komondor; c++){

    if(update_type == TX_FINISHED) {

      channel_power[c] -= total_power[c];

    } else if (update_type == TX_INITIATED) {

      channel_power[c] += total_power[c];

    }
  }

  if(save_node_logs){

    if(update_type == TX_INITIATED)  fprintf(node_logger.file,
        "%f;N%d;S%d;%s;%s Post update channel state [dBm]: ",
        SimTime, node_id, node_state, LOG_D06, LOG_LVL4);

    if(update_type == TX_FINISHED) fprintf(node_logger.file,
        "%f;N%d;S%d;%s;%s Post update channel state [dBm]: ",
        SimTime, node_id, node_state, LOG_E05, LOG_LVL4);

  }

}

/*
 * updateSINR(): Updates the current_sinr parameter
 * Arguments:
 * - pw_received_interest: power received of interest
 * */
double updateSINR(double pw_received_interest, double noise_level, double max_pw_interference,
    int save_node_logs, Logger node_logger, double SimTime, int node_id, int node_state){

  double current_sinr;

  double sinr_db = convertPower(PICO_TO_DBM, pw_received_interest)
            - convertPower(PICO_TO_DBM, (convertPower(DBM_TO_PICO, noise_level) + max_pw_interference));

  current_sinr = convertPower(DB_TO_LINEAR, sinr_db);

  if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s SINR (after update) = %f dBm\n",
      SimTime, node_id, node_state, LOG_D04, LOG_LVL4, convertPower(LINEAR_TO_DB, current_sinr));

  return current_sinr;
}

/*
 * computeMaxInterference(): computes the maximum interference perceived in the channels of interest
 * Arguments:
 * - notification: notification info
 * */
double computeMaxInterference(Notification notification, int current_left_channel, int current_right_channel,
    int node_state, int node_id, int save_node_logs, Logger node_logger, double SimTime,
    double *power_received_per_node, int receiving_from_node_id, double *channel_power) {

  double max_pw_interference = 0;

  for(int c = current_left_channel; c <= current_right_channel; c++){

    if(node_state == STATE_RX_DATA || node_state == STATE_RX_ACK){

      if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Computing Max Interference (power_of_interest = %f dBm / channel_power = %f dBm)\n",
            SimTime, node_id, node_state, LOG_D04, LOG_LVL4,
            convertPower(LINEAR_TO_DB, power_received_per_node[receiving_from_node_id]),
            convertPower(LINEAR_TO_DB, channel_power[c]));


      if(max_pw_interference <= (channel_power[c] - power_received_per_node[receiving_from_node_id])){ // power of interest!!!

        max_pw_interference = channel_power[c] - power_received_per_node[receiving_from_node_id];

      }

      if(save_node_logs) fprintf(node_logger.file, "%f;N%d;S%d;%s;%s Max interference (after update) = %f dBm\n",
            SimTime, node_id, node_state, LOG_D04, LOG_LVL5, convertPower(PICO_TO_DBM,max_pw_interference));

    }
  }

  return max_pw_interference;
}

/*
 * getTxChannelsByChannelBonding: identifies the channels to TX in depending on the channel_bonding scheme
 * and channel_power state.
 */
void getTxChannelsByChannelBonding(int *channels_for_tx, int channel_bonding_model, int *channels_free,
    int min_channel_allowed, int max_channel_allowed, int primary_channel){

  for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
    channels_for_tx[c] = FALSE;
  }

  // I. Get left and right channels available (or free)
  int left_free_ch = 0;
  int left_free_ch_is_set = 0;
  int right_free_ch = 0;
  for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
    if(channels_free[c]){
      if(!left_free_ch_is_set){
        left_free_ch = c;
        left_free_ch_is_set = TRUE;
      }
      if(right_free_ch < c){
        right_free_ch = c;
      }
    }
  }

  int num_free_ch = right_free_ch - left_free_ch + 1;
  int num_available_ch = max_channel_allowed - min_channel_allowed + 1;

  int m;        // Auxiliary variable representing a modulus
  int left_tx_ch;    // Left channel to TX
  int right_tx_ch;  // Right channel to TX

  if(left_free_ch_is_set){
    // Select channels to transmit depending on the sensed power
    switch(channel_bonding_model){

      // Only Primary Channel used if FREE
      case CB_ONLY_PRIMARY:{
        if(primary_channel >= left_free_ch && primary_channel <= right_free_ch){
          channels_for_tx[primary_channel] = CHANNEL_OCCUPIED;
        }
        break;
      }

      // Aggressive SCB: if all channels are FREE, transmit. If not, generate a new backoff.
      case CB_AGGRESIVE_SCB:{
        int tx_possible = TRUE;
        // If all channels are FREE, transmit. If not, generate a new backoff.
        for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
          if(!channels_free[c]){
            tx_possible = FALSE;
          }
        }
        if(tx_possible){
          left_tx_ch = left_free_ch;
          right_tx_ch = right_free_ch;
          for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
            channels_for_tx[c] = CHANNEL_OCCUPIED;
          }
        } else {
          // TX not possible (code it with negative value)
          channels_for_tx[0] = TX_NOT_POSSIBLE;
        }
        break;
      }

      // Log2 SCB:  if all channels accepted by the log2 mapping are FREE, transmit. If not, generate a new backoff.
      case CB_LOG2_SCB:{
        while(1){
          // II. If num_free_ch is power of 2
          if(fmod(log10(num_available_ch)/log10(2), 1) == 0){
            m = primary_channel % num_available_ch;
            left_tx_ch = primary_channel - m;
            right_tx_ch = primary_channel + num_available_ch - m - 1;
            // Check if tx channels are inside the free ones
            if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
              // TX channels found!
              break;
            } else {
              num_available_ch --;
            }
          } else{
            num_available_ch --;
          }
        }

        // If all channels accepted by the log2 mapping, transmit. If not, generate a new backoff.
        int tx_possible = TRUE;
        for(int c = left_tx_ch; c <= right_tx_ch; c++){
          if(!channels_free[c]){
            tx_possible = FALSE;
          }
        }
        if(tx_possible){
          for(int c = left_tx_ch; c <= right_tx_ch; c++){
            channels_for_tx[c] = CHANNEL_OCCUPIED;
          }
        } else {
          // TX not possible (code it with negative value)
          channels_for_tx[0] = TX_NOT_POSSIBLE;
        }
        break;
      }

      // Aggressive DCB: TX in all the free channels contiguous to the primary channel
      case CB_AGGRESIVE_DCB:{
        for(int c = left_free_ch; c <= right_free_ch; c++){
          channels_for_tx[c] = CHANNEL_OCCUPIED;
        }
        break;
      }

      // Log2 DCB: TX in the larger channel range allowed by the log2 mapping
      case CB_LOG2_DCB:{
        while(1){
          // II. If num_free_ch is power of 2
          if(fmod(log10(num_free_ch)/log10(2), 1) == 0){
            m = primary_channel % num_free_ch;
            left_tx_ch = primary_channel - m;
            right_tx_ch = primary_channel + num_free_ch - m - 1;
            // Check if tx channels are inside the free ones
            if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
              // TX channels found!
              for(int c = left_tx_ch; c <= right_tx_ch; c++){
                channels_for_tx[c] = CHANNEL_OCCUPIED;
              }
              break;
            } else {
              num_free_ch --;
            }
          } else{
            num_free_ch --;
          }
        }
        break;
      }
      default:{
        printf("channel_bonding_model %d is NOT VALID!\n", channel_bonding_model);
        break;
      }
    }
  } else{  // No channel is free

    channels_for_tx[0] = TX_NOT_POSSIBLE;

  }

}

/*
 * printOrWriteChannelPower: prints (or writes) the channel_power array representing the power sensed by
 * the node in each subchannel.
 */
void printOrWriteChannelPower(int write_or_print, int save_node_logs,
		Logger node_logger, int print_node_logs,
		int *channels_free, double *channel_power, int num_channels_komondor){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channel_power: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%f  ", channel_power[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(node_logger.file, "%f  ", channel_power[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteChannelsFree: prints (or writes) the channels_free array representing the channels that are free.
 */
void printOrWriteChannelsFree(int write_or_print,
		int save_node_logs, int print_node_logs, Logger node_logger,
		int num_channels_komondor, int *channels_free){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_free: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%d  ", channels_free[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(node_logger.file, "%d ", channels_free[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteNodesTransmitting: prints (or writes) the array representing the transmitting nodes.
 */
void printOrWriteNodesTransmitting(int write_or_print,
		int save_node_logs, int print_node_logs, Logger node_logger, int total_nodes_number,
		int *nodes_transmitting){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("Nodes transmitting: ");
				for(int n = 0; n < total_nodes_number; n++){
					printf("%d  ", nodes_transmitting[n]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int n = 0; n < total_nodes_number; n++){
				 if(save_node_logs){
					 if(nodes_transmitting[n])  fprintf(node_logger.file, "N%d ", n);
				 }
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}

/*
 * printOrWriteChannelForTx: prints (or writes) the channels_for_tx array representing the channels used for TX
 */
void printOrWriteChannelForTx(int write_or_print,
		int save_node_logs, int print_node_logs, Logger node_logger,
		int num_channels_komondor, int *channels_for_tx){

	switch(write_or_print){
		case PRINT_LOG:{
			if(print_node_logs){
				printf("channels_for_tx: ");
				for(int c = 0; c < num_channels_komondor; c++){
					printf("%d  ", channels_for_tx[c]);
					// printf("%d  ", channels_for_tx[c]);
				}
				printf("\n");
			}
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs)  fprintf(node_logger.file, "%d  ", channels_for_tx[c]);
				// printf("%d  ", channels_for_tx[c]);
			}
			if(save_node_logs)  fprintf(node_logger.file, "\n");
			break;
		}
	}
}
