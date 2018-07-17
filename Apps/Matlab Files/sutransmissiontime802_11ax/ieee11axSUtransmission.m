% Modified by Sergio on 2018-06-20

function [T_su,T_c]=ieee11axSUtransmission(L_D,Na,BW,V_s,MCSindex)
    % ieee11axSUtransmission returns the duration of a successful / lost
    % transmission in SU 11ax
    % Input:
    %   - L_D: length of a frame [bits]
    %   - Na: Number of aggregated frames in an A-MPDU
    %   - BW: channel bandwidth [MHz]
    %   - V_s: number of spatial streams
    %   - MCSindex: index of the Modulation Coding Scheme (MCS) (from 0 to 11)
    % Output:
    %   - T_su: duration of single user (SU) transmission [s]
    %   - T_c: duration of a collision [s]
    
    % PHY parameters
    [r,r_leg,T_OFDM,T_OFDM_leg,T_PHY_leg,T_PHY_HE_SU] = ieee11axPHYParams(BW,MCSindex,V_s);
    
    % MAC parameters
    [T_DIFS,T_SIFS,Te,L_MH,L_BACK,L_RTS,L_CTS,L_SF,L_MD,L_TB] = ieee11axMACParams() ;
    
    % Control packets
    T_RTS  = T_PHY_leg + ceil( (L_SF + L_RTS + L_TB) / r_leg ) * T_OFDM_leg;
    T_CTS  = T_PHY_leg + ceil( (L_SF + L_CTS + L_TB) / r_leg ) * T_OFDM_leg;
    T_BACK  = T_PHY_leg + ceil( (L_SF + L_BACK + L_TB) / r_leg ) * T_OFDM_leg;
    
    % Data packet
    if Na == 1
        T_DATA = T_PHY_HE_SU + ceil( ( L_SF + L_MH + L_D + L_TB) / r) * T_OFDM;
    else
        T_DATA = T_PHY_HE_SU + ceil( ( L_SF + Na * (L_MD + L_MH + L_D) + L_TB) / r) * T_OFDM;
    end
        
    % Successful slot
    T_su = T_RTS + T_SIFS + T_CTS + T_SIFS + T_DATA + T_SIFS + T_BACK + T_DIFS + Te; % (Implicit BACK request)
    
    % Collision slot
    T_c = T_RTS + T_SIFS + T_CTS + T_DIFS + Te;
    
%     fprintf('- T_RTS = %f ms\n', T_RTS * 1E3);
%     fprintf('- T_CTS = %f ms\n', T_CTS * 1E3);
    fprintf('- T_DATA = %f ms\n', T_DATA * 1E3);
%     fprintf('- T_BACK = %f ms\n', T_BACK * 1E3);
    fprintf('- T_su = %f ms\n', T_su * 1E3);
%     fprintf('- T_c = %f ms\n', T_c * 1E3);
    
end