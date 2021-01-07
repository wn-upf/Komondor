# define execution parameters
SIM_TIME=100.01
SEED=1992
T_IT=1000
# compile KOMONDOR

cd ..
cd main
./build_local
echo ""
echo ""
echo "***********************************************************************"
echo "PAPER #10 - research question 11: rewards"
echo "***********************************************************************"
echo ""
echo ""
echo ""

  
for REWARD in "utilization" "satisfaction" "mix"
do

echo "------------------------------"
echo "REWARD = $(printf %s $REWARD)"

	for ALGORITHM in "explfirst" "epsilon" "tsbeta" "tsnormal" "ucb" "exp3"
	do  

	echo "- ALGORITHM = $(printf %s $ALGORITHM)"

		for MODEL in 1 4
		do  

		echo "  + TRAFFIC MODEL = $(printf %d $MODEL)"

			for LOAD in 20 100
			do  

			echo "    * LOAD = $(printf %03d $LOAD) Mbps"

				for (( SCENARIO=1; SCENARIO<=100; SCENARIO++ ))
				do  
		
#echo "- agent: agents_reward_$(printf %s $REWARD)_$(printf %s $ALGORITHM).csv"	
#echo "- nodes: input_nodes_n5_tm$(printf %01d $MODEL)_lA$(printf %03d $LOAD)mbps_s$(printf %03d $SCENARIO).csv"

echo "          ../output/script_output.txt sim_$(printf %s $REWARD)_$(printf %s $ALGORITHM)_lA$(printf %03d $LOAD)mbps_s$(printf %03d $SCENARIO)"

./komondor_main ../input/input_paper10/nodes_rq11/input_nodes_n5_tm$(printf %01d $MODEL)_lA$(printf %03d $LOAD)mbps_s$(printf %03d $SCENARIO).csv ../input/input_paper10/agents_rq11/action_space_small_4x3/agents_reward_$(printf %s $REWARD)_$(printf %s $ALGORITHM).csv ../output/script_output.txt sim_$(printf %s $REWARD)_$(printf %s $ALGORITHM)_tm$(printf %01d $MODEL)_lA$(printf %03d $LOAD)mbps_s$(printf %03d $SCENARIO) 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx

				done		

			done

		done
	   
	done
done
