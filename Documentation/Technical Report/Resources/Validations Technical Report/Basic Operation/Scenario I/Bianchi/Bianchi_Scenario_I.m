clear
clc

disp('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
disp('% Komondor validations - Bianchi model in Scenario I %')
disp('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')

%%
disp(' ')
disp('PART 1: Calculate the throughput achieved by STA A and STA B when a single packet is transmitted at every attempt for both the basic access and the RTS/CTS mechanisms.')
disp(' ')

Te = 9E-6;
L=12000;
Tack = 0.25E-3;
SIFS = 16E-6;
DIFS = 34E-6;

% ----------- Individual rates

disp(' - STEP 1: compute individual rates')

RA = 8 * 5/6 * 52 / 4E-6;
RB = 2 * 3/4 * 52 / 4E-6; 

disp(['     + Rate A = ' num2str(RA/10e6) ' Mbps'])
disp(['     + Rate B = ' num2str(RB/10e6) ' Mbps'])
disp(' ')

% ----------- Throughput when a single packet is transmitted

% Duration of a transmission using the Basic Access

disp(' - STEP 2: compute packet and collision durations')

TsbaA = 0.5E-3 + (L/RA) + SIFS + Tack + DIFS + Te;
TcbaA = TsbaA;

TsbaB = 0.5E-3 + (L/RB) + SIFS + Tack + DIFS + Te;
TcbaB = TsbaB;

disp('    * Duration of a Transmission and a Collision (Basic):');
disp(['     + T_s(A) = ' num2str(TsbaA) ' s / T_c(A) = ' num2str(TcbaA) ' s'])
disp(['     + T_s(B) = ' num2str(TsbaB) ' s / T_c(B) = ' num2str(TcbaB) ' s'])

% Duration of a transmission using RTS/CTS, and a collision

Trts = 0.3E-3;
Tcts = Tack;

TsrtsA = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (L/RA) + SIFS + Tack + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

TsrtsB = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (L/RB) + SIFS + Tack + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

disp('    * Duration of a Transmission and a Collision (RTS/CTS):');
disp(['     + T_s(A) = ' num2str(TsrtsA) ' s / T_c(A) = ' num2str(Tcrts) ' s'])
disp(['     + T_s(B) = ' num2str(TsrtsB) ' s / T_c(B) = ' num2str(Tcrts) ' s'])

disp(' ')

% Channel state probabilities 

disp(' - STEP 3: compute channel state probabilities ')

CWmin=16;

tau = 2/(CWmin+1);
N_STAs = 2;

pe = (1-tau)^N_STAs;            % Empty slot
psA = tau*(1-tau)^(N_STAs-1);   % A transmitting
psB = tau*(1-tau)^(N_STAs-1);   % B transmitting
pc = 1-pe-psA-psB;            % Collision

disp('    * Channel state probabilities:');
disp(['     + pe = ' num2str(pe)])
disp(['     + pc = ' num2str(pc)])
disp(['     + psA = ' num2str(psA)])
disp(['     + psB = ' num2str(psB)])

disp(' ')

% WLAN throughput

disp(' - STEP 4: compute throughput per STA ')

SbaA = psA * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));
SbaB = psB * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));

disp('    * Throughput single packet per device (Basic):');
disp(['     + STA A = ' num2str(SbaA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SbaB/10e6) ' Mbps'])

SrtsA = psA * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 
SrtsB = psB * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 

disp('    * Throughput single packet per device (RTS/CTS):');
disp(['     + STA A = ' num2str(SrtsA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SrtsB/10e6) ' Mbps'])
  
disp(' ')

%%
% ----------- Throughput when we transmit K = 64 packets
disp(' ')
disp('PART 2: Calculate the throughput achieved by STA A and STA B when 64 packets are transmitted at every attempt.')
disp(' ')

K=64;

Tback = 0.4E-3;

% Duration of a transmission using the Basic Access
disp(' - STEP 1: transmission duration')

TsbaA = 0.5E-3 + (K*(32+L)/RA) + SIFS + Tback + DIFS + Te;
TcbaA = TsbaA;

TsbaB = 0.5E-3 + (K*(32+L)/RB) + SIFS + Tback + DIFS + Te;
TcbaB = TsbaB;

disp('    * Duration of a Transmission and a Collision (Basic):');
disp(['     + T_s(A) = ' num2str(TsbaA) ' s / T_c(A) = ' num2str(TcbaA) ' s'])
disp(['     + T_s(B) = ' num2str(TsbaB) ' s / T_c(B) = ' num2str(TcbaB) ' s'])

% Duration of a transmission using RTS/CTS, and a collision

Trts = 0.3E-3;
Tcts = Tack;

TsrtsA = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (K*(32+L)/RA) + SIFS + Tack + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

TsrtsB = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (K*(32+L)/RB) + SIFS + Tack + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

disp('    * Duration of a Transmission and a Collision (RTS/CTS):');
disp(['     + T_s(A) = ' num2str(TsrtsA) ' s / T_c(A) = ' num2str(Tcrts) ' s'])
disp(['     + T_s(B) = ' num2str(TsrtsB) ' s / T_c(B) = ' num2str(Tcrts) ' s'])

disp(' ')

% Channel state probabilities 

disp(' - STEP 2: compute channel state probabilities ')

CWmin=16;

tau = 2/(CWmin+1);
N_STAs=2;

pe = (1-tau)^N_STAs;
psA = tau*(1-tau)^(N_STAs-1);
psB = tau*(1-tau)^(N_STAs-1);
pc = 1-pe-psA - psB;

disp('    * Channel state probabilities:');
disp(['     + pe = ' num2str(pe)])
disp(['     + pc = ' num2str(pc)])
disp(['     + psA = ' num2str(psA)])
disp(['     + psB = ' num2str(psB)])

disp(' ')

% WLAN throughput

disp(' - STEP 3: compute throughput per STA ')

SbaA = K* psA * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));
SbaB = K* psB * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));

disp('    * Throughput 64 packets per device (Basic):');
disp(['     + STA A = ' num2str(SbaA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SbaB/10e6) ' Mbps'])

SrtsA = K* psA * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 
SrtsB = K* psB * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 

disp('    * Throughput 64 packets per device (RTS/CTS):');
disp(['     + STA A = ' num2str(SrtsA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SrtsB/10e6) ' Mbps'])

disp(' ')

%%
disp(' ')
disp('PART 3: Compare the results obtained in the previous two points, and explain if it is a good idea to use RTS/CTS along with packet aggregation.')
disp(' ')

% Using RTS/CTS with aggregation is positive in order to protect large
% transmissions from collisions.

%%
% ---- Temporal Fairness
disp(' ')
disp(' - PART 4: Calculate the number of packets that can be aggregated by STA A and STA B.')
disp(' ')

Tf = 10E-3;     % Time fairness

for K = 1 : 64

    % Duration of a transmission using the Basic Access

    TsbaA = 0.5E-3 + (K*(32+L)/RA) + SIFS + Tback + DIFS + Te;
    TcbaA = TsbaA;

    TsbaB = 0.5E-3 + (K*(32+L)/RB) + SIFS + Tback + DIFS + Te;
    TcbaB = TsbaB;

    if(TsbaA <= Tf)
      KA = K;
    end

    if(TsbaB <= Tf)
      KB = K;
    end 

    % Duration of a transmission using RTS/CTS, and a collision

    Trts = 0.3E-3;
    Tcts = Tack;

    TsrtsA = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (K*(32+L)/RA) + SIFS + Tback + DIFS + Te;
    Tcrts = Trts + SIFS + Tcts + DIFS;

    TsrtsB = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (K*(32+L)/RB) + SIFS + Tback + DIFS + Te;
    Tcrts = Trts + SIFS + Tcts + DIFS;

    if(TsrtsA <= Tf)
      KArts = K;
    end

    if(TsrtsB <= Tf)
      KBrts = K;
    end

end

disp(['    * Packets that each device can aggregate in ' num2str(Tf) ' s (Basic):']);
disp(['     + STA A = ' num2str(KA) ' packets'])
disp(['     + STA B = ' num2str(KB) ' packets'])

disp(['    * Packets that each device can aggregate in ' num2str(Tf) ' s (RTS/CTS):']);
disp(['     + STA A = ' num2str(KArts) ' packets'])
disp(['     + STA B = ' num2str(KBrts) ' packets'])

disp(' ')

%%
% Duration of a transmission using the Basic Access
disp(' ')
disp('PART 5: Calculate the throughput achieved by STA A and STA B using the proposed mechanism.')
disp(' ')

disp(' - STEP 1: transmission duration')

TsbaA = 0.5E-3 + (KA*(32+L)/RA) + SIFS + Tback + DIFS + Te;
TcbaA = TsbaA;

TsbaB = 0.5E-3 + (KB*(32+L)/RB) + SIFS + Tback + DIFS + Te;
TcbaB = TsbaB;

disp('    * Duration of a Transmission and a Collision (Basic):');
disp(['     + T_s(A) = ' num2str(TsbaA) ' s / T_c(A) = ' num2str(TcbaA) ' s'])
disp(['     + T_s(B) = ' num2str(TsbaB) ' s / T_c(B) = ' num2str(TcbaB) ' s'])

% Duration of a transmission using RTS/CTS, and a collision

Trts = 0.3E-3;
Tcts = Tack;

TsrtsA = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (KArts*(32+L)/RA) + SIFS + Tback + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

TsrtsB = Trts + SIFS + Tcts + SIFS + 0.5E-3 + (KBrts*(32+L)/RB) + SIFS + Tback + DIFS + Te;
Tcrts = Trts + SIFS + Tcts + DIFS;

disp('    * Duration of a Transmission and a Collision (RTS/CTS):');
disp(['     + T_s(A) = ' num2str(TsrtsA) ' s / T_c(A) = ' num2str(Tcrts) ' s'])
disp(['     + T_s(B) = ' num2str(TsrtsB) ' s / T_c(B) = ' num2str(Tcrts) ' s'])

disp('Duration of a Transmission and a Collision: BA and RTS');
disp([TsbaA TcbaA TsbaB TcbaB TsrtsA Tcrts TsrtsB Tcrts]);

disp(' ')

% Channel state probabilities 

disp(' - STEP 2: compute channel state probabilities ')

CWmin=16;

tau = 2/(CWmin+1);
N_STAs=2;

pe = (1-tau)^N_STAs;
psA = tau*(1-tau)^(N_STAs-1);
psB = tau*(1-tau)^(N_STAs-1);
pc = 1-pe-psA - psB;

disp('    * Channel state probabilities:');
disp(['     + pe = ' num2str(pe)])
disp(['     + pc = ' num2str(pc)])
disp(['     + psA = ' num2str(psA)])
disp(['     + psB = ' num2str(psB)])

disp(' ')

% WLAN throughput

disp(' - STEP 3: compute the throughput per STA ')

SbaA = KA * psA * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));
SbaB = KB * psB * L / (pe*Te + psA * TsbaA + psB * TsbaB + pc * max(TsbaA,TsbaB));
SrtsA = KArts * psA * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 
SrtsB = KBrts * psB * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 

disp(['    * Throughput ' num2str(KA) '/' num2str(KB) ' packets per device (Basic):']);
disp(['     + STA A = ' num2str(SbaA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SbaB/10e6) ' Mbps'])

SrtsA = KArts * psA * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 
SrtsB = KBrts * psB * L / (pe*Te + psA * TsrtsA + psB * TsrtsB + pc * Tcrts); 

disp(['    * Throughput ' num2str(KArts) '/' num2str(KBrts) ' packets per device (RTS/CTS):']);
disp(['     + STA A = ' num2str(SrtsA/10e6) ' Mbps'])
disp(['     + STA B = ' num2str(SrtsB/10e6) ' Mbps'])

%% FUNCTIONS
function L=LogNormalPathloss(PL0,d,gamma)

L = PL0 + 10*gamma*log10(d);

end

function [Ym,Yc]=Rates(Pr)

  switch logical(1)
      case Pr >= -82 && Pr < -79; 
          Ym = 1;
          Yc = 1/2;
    case Pr >= -79 && Pr < -77;  
         Ym = 2;
         Yc = 1/2;
    case Pr >= -77 && Pr < -74;  
         Ym = 2;
         Yc = 3/4;
    case Pr >= -74 && Pr < -70;  
         Ym = 4;
         Yc = 1/2;
    case Pr >= -70 && Pr < -66;  
         Ym = 4;
         Yc = 3/4;
    case Pr >= -66 && Pr < -65;  
         Ym = 6;
         Yc = 1/2;   
    case Pr >= -65 && Pr < -64;  
         Ym = 6;
         Yc = 2/3;  
    case Pr >= -64 && Pr < -59;  
         Ym = 6;
         Yc = 3/4;           
    case Pr >= -59 && Pr < -57;  
         Ym = 8;
         Yc = 3/4;   
    case Pr >= -57;  
         Ym = 8;
         Yc = 5/6;         
    otherwise
         Ym = 0;
         Yc = 0;
end
  
disp([Ym Yc]);

end