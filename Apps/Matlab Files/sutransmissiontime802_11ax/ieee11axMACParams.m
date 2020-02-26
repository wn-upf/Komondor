% Modified by Sergio on 2018-06-20

function [T_DIFS,T_SIFS,Te,L_MH,L_BACK,L_RTS,L_CTS,L_SF,L_MD,L_TB] = ieee11axMACParams()
    % MACParams80211ax returns the main MAC parameters of the 11ax
    % Input:
    %   - void
    % Output:
    %   - DIFS: duration of the DCF Interframe Space (DIFS) [s]
    %   - SIFS: duratio of the Short Interframe Space (SIFS) [s]
    %   - Te: duration of an empty backoff slot [s]
    %   - L_MH: length of MAC header [bits]
    %   - L_BACK: length of Block ACK (BACK) [bits]
    %   - L_RTS: length of Request to Send (RTS) control packet [bits]
    %   - L_CTS: length of Clear to Send (CTS) control packet [bits]
    %   - L_SF: length of service field [bits]
    %   - L_MD: length of MPDU delimiter [bits]
    %   - L_TB: length of tail bits [bits]
    
    T_DIFS = 34E-6;
    T_SIFS = 16E-6;
    Te = 9E-6;
    L_RTS = 160;
    L_CTS = 112;
    L_SF = 16;
    L_MD = 32;
    L_MH = 320;
    L_TB = 18;
    L_BACK = 432;
    
end
