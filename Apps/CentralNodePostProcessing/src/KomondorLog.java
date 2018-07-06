
import java.util.ArrayList;
import java.util.List;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author UPF
 */
public class KomondorLog {
    
    String full_log;
    String log_description;
    int num_nodes;
    int scenario_id;
    int channel_bonding_model;
    int primary_channel;
    int traffic_load;
    int seed;
    Metrics metrics;
    
    public void print_log(){
        
        System.out.println(" + full_log = " + full_log);
        System.out.println("   - log_description = " + log_description);
        System.out.println("   - num_nodes = " + num_nodes);
        System.out.println("   - scenario_id = " + scenario_id);
        System.out.println("   - channel_bonding_model = " + channel_bonding_model);
        System.out.println("   - traffic_load = " + traffic_load);

    }
 
}
