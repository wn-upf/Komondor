for reward in "satisfaction" "utilization" "mix"
do

	mkdir ../output/reward_$(printf %s $reward)

	echo "- mv ../output/logs_output_sim_$(printf %s $reward)_* ../output/reward_$(printf %s $reward)"
	mv ../output/logs_output_sim_$(printf %s $reward)_* ../output/reward_$(printf %s $reward)

	for algorithm in "explfirst" "epsilon" "tsnormal" "tsbeta" "ucb" "exp3"
	do

		mkdir ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)

		echo "   + mv ../output/reward_$(printf %s $reward)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)"
		mv ../output/reward_$(printf %s $reward)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)

		for tmodel in 1 4
		do

		mkdir ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)

		echo "      * mv ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)_tm$(printf %d $tmodel)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)"
		mv ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)_tm$(printf %d $tmodel)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)

			for load in 20 100
			do

			mkdir ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)/$(printf %d $load)mbps
		
				echo "          X mv ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)_tm$(printf %d $tmodel)_lA$(printf %03d $load)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)/$(printf %d $load)mbps"
				mv ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)/logs_output_sim_$(printf %s $reward)_$(printf %s $algorithm)_tm$(printf %d $tmodel)_lA$(printf %03d $load)* ../output/reward_$(printf %s $reward)/$(printf %s $algorithm)/tm$(printf %d $tmodel)/$(printf %d $load)mbps

			done

		done
		
	done

done
