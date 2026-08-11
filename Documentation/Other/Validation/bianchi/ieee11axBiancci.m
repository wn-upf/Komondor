%%% File description: this scripts computes the Biancci throughput of fully
%%% overlapping WLANs scenarios.

clc
clear
close all

N = 3;          % Number of overlapping WLANs
CW = 16;        % Min. contention window

L_D = 11728;    % Lenght of data packet [bits]
Na = 43;         % Number of aggregated frames in an A-MPDU
BW = 20;        % Channel bandwidth [MHz]
V_s = 1;        % Number of spatial streams
MCSindex = 9;  % index of the IEEE 802.11ax Modulation Coding Scheme (MCS) (from 0 to 11)
    
[~,~,Te,~,~,~,~,~,~,~] = ieee11axMACParams();
[r,r_leg,T_OFDM,T_OFDM_leg,~,~,~,~,~] = ieee11axPHYParams(BW,MCSindex,V_s);
[T_su,T_c] = ieee11axSUtransmission(L_D,Na,BW,V_s,MCSindex);

fprintf("BW = %d MHz, MCS %d:\n", BW, MCSindex); 

EB = (CW-1)/2 * Te;
tau = 2 / (CW + 1); % Prob. to transmit in a certain slot
p = 1 - (1 - tau) ^ (N - 1);    % Prob. collision

pe = (1 - tau) ^ N; % Prob. slot is empty (no node transmit in that slot)
ps = N * tau * (1 - tau) ^ (N - 1); % Prob. slot contains a successful transmission (only one node transmits)
pc = 1 - pe - ps;   % Prob. slot contains a collision between two or more packets (two or more nodes transmit)

% In Slotted-CSMA the slots are of different duration (not like in slotted-Aloha)
av_T_slot = pe * Te + ps * T_su + pc * T_c; % Average duration of a slot
S = ps / av_T_slot * (L_D * Na);

av_T_su = ps * T_su / av_T_slot / N;
av_T_c = pc * T_c / av_T_slot;
av_T_e = pe * Te / av_T_slot;

t_obs = 1000;

tot_t_su = t_obs * av_T_su;
num_pkts = tot_t_su/T_su;

tot_t_c = t_obs * av_T_c;
num_pkts_c = tot_t_c/T_c;

tot_t_e = t_obs * av_T_e;

fprintf('Biancci input: N = %d, CW = %d\n', N, CW);
fprintf('- Av. throughput = %.3f Mbps\n', S*1E-6 / N);
fprintf('- Prob. col. = %.3f %%\n', p * 100);
fprintf('- pe = %.3f %%\n', pe * 100);
fprintf('- ps = %.3f %%\n', ps * 100);
fprintf('- pc = %.3f %%\n', pc * 100);
fprintf('- Av. share of time successfully transmitting = %.3f %%\n', av_T_su * 100);
fprintf('  + For t_obs = %.2f s: %.3f s\n', t_obs, tot_t_su);
fprintf('  + Num successful packets: %.0f\n', num_pkts);
fprintf('  + Num loss packets: %.0f\n', num_pkts_c);
fprintf('  + Num txed packets: %.0f\n', num_pkts + num_pkts_c);

% fprintf('- Av. throughput Boris 1 WLAN = %.2f Mbps\n', (L_D * Na / (EB + T_su)) * 1E-6);

