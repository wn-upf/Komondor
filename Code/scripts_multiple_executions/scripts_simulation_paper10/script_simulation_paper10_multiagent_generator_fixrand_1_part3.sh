# define execution parameters
SIM_TIME=6.5
#NUM_SCENARIOS=100
# compile KOMONDOR

cd ../../
cd main
./build_local
echo ""
echo ""
echo "***********************************************************************"
echo "PAPER #10 - Multiagent generator v1 - part 3"
echo "***********************************************************************"
echo ""

for NUM_AP in "6" "10" "16"
do

	echo "- NUM_AP: $(printf %s $NUM_AP)"

	for REWARD in "thrsat" "mix"
	do

		echo "  * reward: $REWARD"

		for MAB_ALG in "fixed"
			do

			echo "      - MAB: $(printf %s $MAB_ALG)"

			for (( SCENARIO=51; SCENARIO<=75; SCENARIO++ ))
			do

				echo "          * scenario: $SCENARIO/75"

				for SEED in "1990" "1991"
				do

					echo "              - seed: $SEED"	

./komondor_main ../input/input_paper10/input_nodes/nodes_multiagent_generator_1/input_nodes_n$(printf %d $NUM_AP)_s$(printf %03d $SCENARIO).csv ../input/input_paper10/input_agents/agents_multiagent_generator_1/action_space1/agents_n$(printf %d $NUM_AP)_$(printf %s $REWARD)_$(printf %s $MAB_ALG).csv ../output_part3/script_output.txt sim_n$(printf %d $NUM_AP)_$(printf %s $REWARD)_$(printf %s $MAB_ALG)_s$(printf %03d $SCENARIO)_seed$(printf %d $SEED) 0 1 1 1 1 $SIM_TIME $(printf %d $SEED) >> ../output_part3/logs_console.tx		

				done
			done
		done
	done
done
