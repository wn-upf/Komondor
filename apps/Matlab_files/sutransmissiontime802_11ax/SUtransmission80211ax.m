% Input parameters
% - L: packet size (from network layer)
% - Na: Number of aggregated packets in an A-MPDU
% - W: RU size OFDMA (channel width)
% - SUSS: Single-User Spatial Streams
% - MCSindex: Modulation and Coding Scheme index

% Author: Kostis Dovelos
% Changes: More MCS, and few minor changes in PHY and MAC input parameters (BB, 14-09-2017)

% RTS + SIFS + CTS + DATA + SIFS + ACK + DIFS + Te

function [T,T_c]=SUtransmission80211ax(L,Na,W,SUSS,MCSindex)

  % Load 802.11ax parameters

  %% MAC
  [DIFS,SIFS,Te,L_MACH,L_BACK,L_RTS,L_CTS,L_SF,L_DEL,L_TAIL]=MACParams80211ax();
  
  %% PHY
  [Nsc,Ym,Yc,T_OFDM,Legacy_PHYH,HE_PHYH] = PHYParams80211ax(W,MCSindex,SUSS);
  
  Rate = Nsc * Ym * Yc * SUSS;
  Rate_20MHz = 52 * Ym * Yc; % In legacy mode
  
  disp([Nsc Ym Yc]);
  
  % Duplicate RTS/CTS for bandwidth allocation
  T_RTS  = Legacy_PHYH + ceil((L_SF+L_RTS+L_TAIL)/Rate_20MHz)*T_OFDM;
  T_CTS  = Legacy_PHYH + ceil((L_SF+L_CTS+L_TAIL)/Rate_20MHz)*T_OFDM;
  
  % After successful acquisition of the channel
  T_DATA = (Legacy_PHYH + HE_PHYH) + ceil((L_SF+Na*(L_DEL+L_MACH+L)+L_TAIL)/Rate)*T_OFDM;
  T_BACK = Legacy_PHYH + ceil((L_SF+L_BACK+L_TAIL)/Rate_20MHz)*T_OFDM;
   
  % Successful slot
  T = T_RTS + SIFS + T_CTS + SIFS + T_DATA + SIFS + T_BACK + DIFS + Te; % (Implicit BACK request)
  
  % Collision slot
  T_c = T_RTS + SIFS + T_CTS + DIFS + Te;

end