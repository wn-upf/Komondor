% Modified by Sergio on 2018-06-20

function [r,r_leg,T_OFDM,T_OFDM_leg,T_PHY_leg,T_PHY_HE_SU,Y_sc,Y_m,Y_c] = ieee11axPHYParams(BW,MCSindex,V_s)
    % ieee11axPHYParams returns the main PHY parameters of the 11ax
    % Input:
    %   - BW: channel bandwidth [MHz]
    %   - MCSindex: index of the Modulation Coding Scheme (MCS) (from 0 to 11)
    %   - V_s: number of spatial streams
    % Output:
    %   - r: number of bits per OFDM symbol in the data packet [bits]
    %   - r_leg: number of bits per OFDM symbol in the control packets [bits]
    %   - T_OFDM_leg: duration of a legacy OFDM symbol (4 us) [s]
    %   - T_OFDM: duration of an OFDM symbol (16 us) [s]
    %   - T_PHY_leg: duration of the legacy preamble [s]
    %   - T_PHY_HE_SU: duration of th HE Single-user preamble [s]
    %   - Y_sc: number of data subcarriers
    %   - Y_m: number of bits per symbol of the constellation used
    %   - Y_c: coding rate
    
    % Header ------
    
    T_OFDM = 16E-6;   % Guard interval (GI) 3.2 us
    T_OFDM_leg = 4E-6;
    T_PHY_leg = 20E-6;
    T_PHY_HE_SU = 164E-6;
    
    switch BW
        case 2.5
            Y_sc = 26;  
        case 5
            Y_sc = 52;
        case 10
            Y_sc = 102;
        case 20
            Y_sc = 234;
        case 40
            Y_sc = 468;
        case 80
            Y_sc = 980;
        case 160
            Y_sc = 1960;
        otherwise
            error(['Specified BW (' num2str(BW) ' MHz) is incorrect!'])
    end
    
    % Modulation and Coding Scheme (MCS) without DCM
    switch MCSindex
        case 0
            Y_m = 1;     
            Y_c = 1/2;  
        case 1
            Y_m = 2;
            Y_c = 1/2;
        case 2
            Y_m = 2;
            Y_c = 3/4;
        case 3
            Y_m = 4;
            Y_c = 1/2;
        case 4
            Y_m = 4;
            Y_c = 3/4;
        case 5
            Y_m = 6;
            Y_c = 1/2;
        case 6
            Y_m = 6;
            Y_c = 2/3;
        case 7
            Y_m = 6;
            Y_c = 3/4;
        case 8
            Y_m = 8;
            Y_c = 3/4;
        case 9
            Y_m = 8;
            Y_c = 5/6;
        case 10
            Y_m = 10;
            Y_c = 3/4;
        case 11
            Y_m = 10;
            Y_c = 5/6;
            
        otherwise
            error(['Specified MCS (' num2str(MCSindex) ') is incorrect!'])
    end
    
    r = V_s * Y_m * Y_c * Y_sc;
    r_leg = 24; % Legacy no. of bits per OFDM symbol is fixed to match MCS 0 (6 Mbps)
    
end