
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/*
 * This program gets logs from the central WLAN scenario and processes them
 */
/**
 *
 * @author sergio barrachina munoz
 */
public class Post_processer {

    static final String CSV_SEPARATOR = ";";
    static final int CB_OP = 0;
    static final int CB_SCB = 2;
    static final int CB_AM = 4;
    static final int CB_PU = 6;
    static final int[] CHANNEL_BONDINGS = {CB_OP, CB_SCB, CB_AM, CB_PU};
    static final int[] TRAFFIC_LOADS = {1, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240};
    // static final int NUM_SCENARIOS = 100;
    static final int NUM_SCENARIOS = 200;
    static final int DELAY_DELTA = 1;   // [ms]
    static final double THROUGHPUT_DELTA = 2;
    static final double THROUGHPUT_DELTA_FACTOR = 0.05; // through >= (1-0.05) * load
    static final double SIM_TIME = 25;

    static HashMap<Integer, Integer> num_zero_delay_measures_op = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> num_zero_delay_measures_scb = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> num_zero_delay_measures_am = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> num_zero_delay_measures_pu = new HashMap<Integer, Integer>();

    static HashMap<Integer, HashMap<Integer, Double>> delay_am = new HashMap<Integer, HashMap<Integer, Double>>();
    static HashMap<Integer, HashMap<Integer, Double>> delay_pu = new HashMap<Integer, HashMap<Integer, Double>>();

    // Number of scenarios where PU has less delay than AM for every load
    static HashMap<Integer, Integer> compare_delays_pu_wins = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> compare_delays_am_wins = new HashMap<Integer, Integer>();

    // Number of scenarios where PU has less delay in scenarios where throug >= 0.95 load than AM for every load
    static HashMap<Integer, Integer> sim_compare_delays_pu_wins = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> sim_compare_delays_am_wins = new HashMap<Integer, Integer>();
    static HashMap<Integer, Integer> sim_compare_delays_draw = new HashMap<Integer, Integer>();

    static List<KomondorLog> list_logs = new ArrayList<KomondorLog>();

    /* This is how to declare HashMap */
    // sum metrics values
    static HashMap<Integer, HashMap<Integer, Metrics>> metrics_op = new HashMap<Integer, HashMap<Integer, Metrics>>();
    static HashMap<Integer, Metrics> sum_metrics_op = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Metrics> average_metrics_op = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Double> std_delay_op = new HashMap<Integer, Double>();
    static HashMap<Integer, HashMap<Integer, Metrics>> metrics_scb = new HashMap<Integer, HashMap<Integer, Metrics>>();
    static HashMap<Integer, Metrics> sum_metrics_scb = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Metrics> average_metrics_scb = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Double> std_delay_scb = new HashMap<Integer, Double>();
    static HashMap<Integer, HashMap<Integer, Metrics>> metrics_am = new HashMap<Integer, HashMap<Integer, Metrics>>();
    static HashMap<Integer, Metrics> sum_metrics_am = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Metrics> average_metrics_am = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Double> std_delay_am = new HashMap<Integer, Double>();
    static HashMap<Integer, HashMap<Integer, Metrics>> metrics_pu = new HashMap<Integer, HashMap<Integer, Metrics>>();
    static HashMap<Integer, Metrics> sum_metrics_pu = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Metrics> average_metrics_pu = new HashMap<Integer, Metrics>();
    static HashMap<Integer, Double> std_delay_pu = new HashMap<Integer, Double>();

    // Prob. throughput is similar to the traffic load
    static HashMap<Integer, Double> op_count_scenarios_load_accomplished = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> scb_count_scenarios_load_accomplished = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> am_count_scenarios_load_accomplished = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> pu_count_scenarios_load_accomplished = new HashMap<Integer, Double>();

    // Average delay of the scenarios where the throughput is similar to the load
    static HashMap<Integer, Double> op_non_sat_delay = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> scb_non_sat_delay = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> am_non_sat_delay = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> pu_non_sat_delay = new HashMap<Integer, Double>();

    static HashMap<Integer, Double> op_non_sat_delay_std = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> scb_non_sat_delay_std = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> am_non_sat_delay_std = new HashMap<Integer, Double>();
    static HashMap<Integer, Double> pu_non_sat_delay_std = new HashMap<Integer, Double>();

    public static void read_logs_file(String input_path) throws FileNotFoundException, IOException {

        String line;

        System.out.println("Reading input file...");

        try (BufferedReader br = new BufferedReader(new FileReader(input_path))) {
            while ((line = br.readLine()) != null) {

                // KOMONDOR SIMULATION 'sim_input_nodes_n20_s10_cb4_load001.csv' (seed 1992)
                KomondorLog log = new KomondorLog();
                log.full_log = line;
//                System.out.println("full_log: " + log.full_log);

                String[] log_attributes = line.split(CSV_SEPARATOR);

                // Identify attributes in the description
                log.log_description = log_attributes[0];
//                System.out.println("- log_description: " + log.log_description);                
                String[] input_attributes = log.log_description.split("_");
                log.num_nodes = Integer.parseInt(input_attributes[3].substring(1));
//                System.out.println("- num_nodes: " + log.num_nodes);
                log.scenario_id = Integer.parseInt(input_attributes[4].substring(1));
//                System.out.println("- scenario_id: " + log.scenario_id);
                log.channel_bonding_model = Integer.parseInt(input_attributes[5].substring(2));
//                System.out.println("- channel_bonding_model: " + log.channel_bonding_model);
                String[] separated_load = input_attributes[6].split("\\.");
                log.traffic_load = Integer.parseInt(separated_load[0].substring(4));
//                System.out.println("- traffic_load: " + log.traffic_load);

                // Identify output values (metrics)
                Metrics metrics = new Metrics();
                metrics.num_packets_generated = Integer.parseInt(log_attributes[1]);
                metrics.average_num_packets_generated = Double.parseDouble(log_attributes[2]);
                metrics.throughput = Double.parseDouble(log_attributes[3]);
                metrics.rho = Double.parseDouble(log_attributes[4]);
                metrics.delay = Double.parseDouble(log_attributes[5]);
                metrics.utilization = Double.parseDouble(log_attributes[6]);
                metrics.drop_ratio = Double.parseDouble(log_attributes[7]);

                log.metrics = metrics;

                // Add log to the list
                list_logs.add(log);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void initialize_sum_metrics() {

        int cb = 0;
        int traffic_load = 0;

        Metrics metrics = new Metrics();

        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {

            traffic_load = TRAFFIC_LOADS[j];

            metrics_op.put(traffic_load, new HashMap<>());
            metrics_scb.put(traffic_load, new HashMap<>());
            metrics_am.put(traffic_load, new HashMap<>());
            metrics_pu.put(traffic_load, new HashMap<>());

            sum_metrics_op.put(traffic_load, metrics);
            sum_metrics_scb.put(traffic_load, metrics);
            sum_metrics_am.put(traffic_load, metrics);
            sum_metrics_pu.put(traffic_load, metrics);

            average_metrics_op.put(traffic_load, metrics);
            average_metrics_scb.put(traffic_load, metrics);
            average_metrics_am.put(traffic_load, metrics);
            average_metrics_pu.put(traffic_load, metrics);

            std_delay_op.put(traffic_load, (double) 0);
            std_delay_scb.put(traffic_load, (double) 0);
            std_delay_am.put(traffic_load, (double) 0);
            std_delay_pu.put(traffic_load, (double) 0);

            op_non_sat_delay.put(traffic_load, (double) 0);
            scb_non_sat_delay.put(traffic_load, (double) 0);
            am_non_sat_delay.put(traffic_load, (double) 0);
            pu_non_sat_delay.put(traffic_load, (double) 0);

            op_non_sat_delay_std.put(traffic_load, (double) 0);
            scb_non_sat_delay_std.put(traffic_load, (double) 0);
            am_non_sat_delay_std.put(traffic_load, (double) 0);
            pu_non_sat_delay_std.put(traffic_load, (double) 0);

            num_zero_delay_measures_op.put(traffic_load, 0);
            num_zero_delay_measures_scb.put(traffic_load, 0);
            num_zero_delay_measures_am.put(traffic_load, 0);
            num_zero_delay_measures_pu.put(traffic_load, 0);

            op_count_scenarios_load_accomplished.put(traffic_load, (double) 0);
            scb_count_scenarios_load_accomplished.put(traffic_load, (double) 0);
            am_count_scenarios_load_accomplished.put(traffic_load, (double) 0);
            pu_count_scenarios_load_accomplished.put(traffic_load, (double) 0);
        }
    }

    // Process logs. Generate processed info
    public static void process_logs() {

        int traffic_load;
        for (KomondorLog log : list_logs) {

            traffic_load = log.traffic_load;

            switch (log.channel_bonding_model) {

                case CB_OP:

                    metrics_op.get(traffic_load).put(log.scenario_id, log.metrics);

                    sum_metrics_op.put(
                            traffic_load, Metrics.sum_metrics(
                                    sum_metrics_op.get(traffic_load), log.metrics));

                    if (log.metrics.delay == 0) {
                        num_zero_delay_measures_op.put(
                                traffic_load,
                                num_zero_delay_measures_op.get(traffic_load) + 1);
                    }

                    // If throughput similar to traffic load
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA) {
                    if (Math.abs(log.metrics.throughput - log.metrics.num_packets_generated / SIM_TIME) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {

                        op_count_scenarios_load_accomplished.put(traffic_load,
                                op_count_scenarios_load_accomplished.get(traffic_load) + 1);

                        op_non_sat_delay.put(traffic_load,
                                op_non_sat_delay.get(traffic_load) + log.metrics.delay);
                    }

                    break;

                case CB_SCB:

                    metrics_scb.get(traffic_load).put(log.scenario_id, log.metrics);

                    sum_metrics_scb.put(
                            traffic_load, Metrics.sum_metrics(
                                    sum_metrics_scb.get(traffic_load), log.metrics));

                    if (log.metrics.delay == 0) {
                        num_zero_delay_measures_scb.put(
                                traffic_load,
                                num_zero_delay_measures_scb.get(traffic_load) + 1);
                    }

                    // If throughput similar to traffic load
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA) {
                    if (Math.abs(log.metrics.throughput - log.metrics.num_packets_generated / SIM_TIME) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                        scb_count_scenarios_load_accomplished.put(traffic_load,
                                scb_count_scenarios_load_accomplished.get(traffic_load) + 1);

                        scb_non_sat_delay.put(traffic_load,
                                scb_non_sat_delay.get(traffic_load) + log.metrics.delay);
                    }

                    break;

                case CB_AM:
                    metrics_am.get(traffic_load).put(log.scenario_id, log.metrics);

                    sum_metrics_am.put(
                            traffic_load, Metrics.sum_metrics(
                                    sum_metrics_am.get(traffic_load), log.metrics));

                    if (log.metrics.delay == 0) {
                        num_zero_delay_measures_am.put(
                                traffic_load,
                                num_zero_delay_measures_am.get(traffic_load) + 1);

                    }

                    // If throughput similar to traffic load
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA) {
                    if (Math.abs(log.metrics.throughput - log.metrics.num_packets_generated / SIM_TIME) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                        am_count_scenarios_load_accomplished.put(traffic_load,
                                am_count_scenarios_load_accomplished.get(traffic_load) + 1);

                        am_non_sat_delay.put(traffic_load,
                                am_non_sat_delay.get(traffic_load) + log.metrics.delay);
                    }

                    break;

                case CB_PU:

                    metrics_pu.get(traffic_load).put(log.scenario_id, log.metrics);
                    sum_metrics_pu.put(
                            traffic_load, Metrics.sum_metrics(
                                    sum_metrics_pu.get(traffic_load), log.metrics));

                    if (log.metrics.delay == 0) {
                        num_zero_delay_measures_pu.put(
                                traffic_load,
                                num_zero_delay_measures_pu.get(traffic_load) + 1);
                    }

                    // If throughput similar to traffic load
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                    // if (Math.abs(log.metrics.throughput - log.traffic_load) < THROUGHPUT_DELTA) {
                    if (Math.abs(log.metrics.throughput - log.metrics.num_packets_generated / SIM_TIME) < THROUGHPUT_DELTA_FACTOR * log.traffic_load) {
                        pu_count_scenarios_load_accomplished.put(traffic_load,
                                pu_count_scenarios_load_accomplished.get(traffic_load) + 1);

                        pu_non_sat_delay.put(traffic_load,
                                pu_non_sat_delay.get(traffic_load) + log.metrics.delay);
                    }

                    break;

                default:
                    break;
            }

        }

    }

    public static void get_std_delay_non_sat() {

        double std_numerator_op = 0;
        double std_numerator_scb = 0;
        double std_numerator_am = 0;
        double std_numerator_pu = 0;

        // **** THROUGH SIMILAR TO LOAD ****
        boolean op_success = false;
        boolean scb_success = false;
        boolean am_success = false;
        boolean pu_success = false;
        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            for (int s = 1; s <= NUM_SCENARIOS; s++) {

                // Check if current log accomplishes with through similar to load
                if (Math.abs(metrics_op.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_op.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {
                    op_success = true;
                }

                if (Math.abs(metrics_scb.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_scb.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {
                    scb_success = true;
                }

                if (Math.abs(metrics_am.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_am.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {
                    am_success = true;
                }

                if (Math.abs(metrics_pu.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_pu.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {
                    pu_success = true;
                }

                if (op_success) {
                    std_numerator_op
                            += Math.pow(metrics_op.get(TRAFFIC_LOADS[l]).get(s).delay
                                    - op_non_sat_delay.get(TRAFFIC_LOADS[l]) / op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]), 2);
                    
                    System.out.println("value = " + metrics_op.get(TRAFFIC_LOADS[l]).get(s).delay);
                    System.out.println("mean = " +  op_non_sat_delay.get(TRAFFIC_LOADS[l]) / op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]));
                    System.out.println("std_numerator_op = " + std_numerator_op);
                }

                if (scb_success) {
                    std_numerator_scb
                            += Math.pow(metrics_scb.get(TRAFFIC_LOADS[l]).get(s).delay
                                    - scb_non_sat_delay.get(TRAFFIC_LOADS[l]) / scb_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]), 2);
                }

                if (am_success) {
                    std_numerator_am
                            += Math.pow(metrics_am.get(TRAFFIC_LOADS[l]).get(s).delay
                                    - am_non_sat_delay.get(TRAFFIC_LOADS[l]) / am_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]), 2);
                }

                if (pu_success) {
                    std_numerator_pu
                            += Math.pow(metrics_pu.get(TRAFFIC_LOADS[l]).get(s).delay
                                    - pu_non_sat_delay.get(TRAFFIC_LOADS[l]) / pu_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]), 2);
                }
                
                
                op_success = false;
                scb_success = false;
                am_success = false;
                pu_success = false;

            }

            System.out.println("STD = " +  Math.sqrt(std_numerator_op
                            / op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])));
            
            op_non_sat_delay_std.put(
                    TRAFFIC_LOADS[l], Math.sqrt(
                            std_numerator_op
                            / op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])));
            
            System.out.println("STD = " + op_non_sat_delay_std.get(TRAFFIC_LOADS[l]));

            scb_non_sat_delay_std.put(
                    TRAFFIC_LOADS[l], Math.sqrt(
                            std_numerator_scb
                            / scb_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])));

            am_non_sat_delay_std.put(
                    TRAFFIC_LOADS[l], Math.sqrt(
                            std_numerator_am
                            / am_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])));

            pu_non_sat_delay_std.put(
                    TRAFFIC_LOADS[l], Math.sqrt(
                            std_numerator_pu
                            / pu_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])));

        }
    }

    public static void average_metrics() {

        int traffic_load;

        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {
            traffic_load = TRAFFIC_LOADS[j];
            average_metrics_op.put(
                    traffic_load, Metrics.get_average_metrics(
                            sum_metrics_op.get(traffic_load), NUM_SCENARIOS,
                            NUM_SCENARIOS - num_zero_delay_measures_op.get(traffic_load)));
            average_metrics_scb.put(
                    traffic_load, Metrics.get_average_metrics(
                            sum_metrics_scb.get(traffic_load), NUM_SCENARIOS,
                            NUM_SCENARIOS - num_zero_delay_measures_scb.get(traffic_load)));
            average_metrics_am.put(
                    traffic_load, Metrics.get_average_metrics(
                            sum_metrics_am.get(traffic_load), NUM_SCENARIOS,
                            NUM_SCENARIOS - num_zero_delay_measures_am.get(traffic_load)));
            average_metrics_pu.put(
                    traffic_load, Metrics.get_average_metrics(
                            sum_metrics_pu.get(traffic_load), NUM_SCENARIOS,
                            NUM_SCENARIOS - num_zero_delay_measures_pu.get(traffic_load)));
        }

        double std_numerator_op = 0;
        double std_numerator_scb = 0;
        double std_numerator_am = 0;
        double std_numerator_pu = 0;

        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {

            traffic_load = TRAFFIC_LOADS[j];

            for (int s = 1; s <= NUM_SCENARIOS; s++) {

                if (metrics_op.get(traffic_load).get(s).delay > 0) {
                    std_numerator_op
                            += +Math.pow(metrics_op.get(traffic_load).get(s).delay
                                    - average_metrics_op.get(traffic_load).delay, 2);
                }

                if (metrics_scb.get(traffic_load).get(s).delay > 0) {
                    std_numerator_scb
                            += +Math.pow(metrics_scb.get(traffic_load).get(s).delay
                                    - average_metrics_scb.get(traffic_load).delay, 2);
                }

                if (metrics_am.get(traffic_load).get(s).delay > 0) {
                    std_numerator_am
                            += +Math.pow(metrics_am.get(traffic_load).get(s).delay
                                    - average_metrics_am.get(traffic_load).delay, 2);
                }

                if (metrics_pu.get(traffic_load).get(s).delay > 0) {
                    std_numerator_pu
                            += +Math.pow(metrics_pu.get(traffic_load).get(s).delay
                                    - average_metrics_pu.get(traffic_load).delay, 2);
                }

            }

            std_delay_op.put(
                    traffic_load, Math.sqrt(
                            std_numerator_op
                            / (NUM_SCENARIOS - num_zero_delay_measures_op.get(traffic_load) - 1)));

            std_delay_scb.put(
                    traffic_load, Math.sqrt(
                            std_numerator_scb
                            / (NUM_SCENARIOS - num_zero_delay_measures_scb.get(traffic_load) - 1)));

            std_delay_am.put(
                    traffic_load, Math.sqrt(
                            std_numerator_am
                            / (NUM_SCENARIOS - num_zero_delay_measures_am.get(traffic_load) - 1)));

            std_delay_pu.put(
                    traffic_load, Math.sqrt(
                            std_numerator_pu
                            / (NUM_SCENARIOS - num_zero_delay_measures_pu.get(traffic_load) - 1)));

        }

    }

    // Write processed info
    public static void save_processed_info(String output_path) throws IOException {

        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "cb_type" + CSV_SEPARATOR
                + "load" + CSV_SEPARATOR
                + "num_pkt_gen" + CSV_SEPARATOR
                + "through" + CSV_SEPARATOR
                + "rho" + CSV_SEPARATOR
                + "delay" + CSV_SEPARATOR
                + "util" + CSV_SEPARATOR
                + "drop" + CSV_SEPARATOR
                + "std_delay" + CSV_SEPARATOR;

        // System.out.println(csv_header_line);
        out.println(csv_header_line);

        int traffic_load;
        Metrics metrics;

        // OP
        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {
            traffic_load = TRAFFIC_LOADS[j];
            metrics = average_metrics_op.get(traffic_load);
            line = CB_OP + CSV_SEPARATOR
                    + traffic_load + CSV_SEPARATOR
                    + metrics.num_packets_generated + CSV_SEPARATOR
                    + metrics.throughput + CSV_SEPARATOR
                    + metrics.rho + CSV_SEPARATOR
                    + metrics.delay + CSV_SEPARATOR
                    + metrics.utilization + CSV_SEPARATOR
                    + metrics.drop_ratio + CSV_SEPARATOR
                    + std_delay_op.get(traffic_load);
            out.println(line);
        }

        // SCB
        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {
            traffic_load = TRAFFIC_LOADS[j];
            metrics = average_metrics_scb.get(traffic_load);
            line = CB_SCB + CSV_SEPARATOR
                    + traffic_load + CSV_SEPARATOR
                    + metrics.num_packets_generated + CSV_SEPARATOR
                    + metrics.throughput + CSV_SEPARATOR
                    + metrics.rho + CSV_SEPARATOR
                    + metrics.delay + CSV_SEPARATOR
                    + metrics.utilization + CSV_SEPARATOR
                    + metrics.drop_ratio + CSV_SEPARATOR
                    + std_delay_scb.get(traffic_load);
            out.println(line);
        }

        // AM
        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {
            traffic_load = TRAFFIC_LOADS[j];
            metrics = average_metrics_am.get(traffic_load);
            line = CB_AM + CSV_SEPARATOR
                    + traffic_load + CSV_SEPARATOR
                    + metrics.num_packets_generated + CSV_SEPARATOR
                    + metrics.throughput + CSV_SEPARATOR
                    + metrics.rho + CSV_SEPARATOR
                    + metrics.delay + CSV_SEPARATOR
                    + metrics.utilization + CSV_SEPARATOR
                    + metrics.drop_ratio + CSV_SEPARATOR
                    + std_delay_am.get(traffic_load);
            out.println(line);
        }

        // PU
        for (int j = 0; j < TRAFFIC_LOADS.length; j++) {
            traffic_load = TRAFFIC_LOADS[j];
            metrics = average_metrics_pu.get(traffic_load);
            line = CB_PU + CSV_SEPARATOR
                    + traffic_load + CSV_SEPARATOR
                    + metrics.num_packets_generated + CSV_SEPARATOR
                    + metrics.throughput + CSV_SEPARATOR
                    + metrics.rho + CSV_SEPARATOR
                    + metrics.delay + CSV_SEPARATOR
                    + metrics.utilization + CSV_SEPARATOR
                    + metrics.drop_ratio + CSV_SEPARATOR
                    + std_delay_pu.get(traffic_load);
            out.println(line);
        }

        out.close();

        System.out.println("File saved in " + output_path);

    }

    public static void compare_delay_pu_am() {

        // For each load, delay 0
        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {
            compare_delays_am_wins.put(TRAFFIC_LOADS[l], 0);
            compare_delays_pu_wins.put(TRAFFIC_LOADS[l], 0);
            sim_compare_delays_am_wins.put(TRAFFIC_LOADS[l], 0);
            sim_compare_delays_pu_wins.put(TRAFFIC_LOADS[l], 0);
            sim_compare_delays_draw.put(TRAFFIC_LOADS[l], 0);
        }

        // For each scenario, Hashmap "for each load" delay 0
        for (int s = 1; s <= NUM_SCENARIOS; s++) {
            HashMap<Integer, Double> hash_map_scenario_am_aux = new HashMap<>();
            HashMap<Integer, Double> hash_map_scenario_pu_aux = new HashMap<>();
            for (int l = 0; l < TRAFFIC_LOADS.length; l++) {
                hash_map_scenario_am_aux.put(TRAFFIC_LOADS[l], (double) 0);
                hash_map_scenario_pu_aux.put(TRAFFIC_LOADS[l], (double) 0);
            }
            delay_am.put(s, hash_map_scenario_am_aux);
            delay_pu.put(s, hash_map_scenario_pu_aux);
        }

        for (KomondorLog log : list_logs) {

            if (log.channel_bonding_model == CB_AM) {

                delay_am.get(log.scenario_id).put(log.traffic_load, log.metrics.delay);

            } else if (log.channel_bonding_model == CB_PU) {

                delay_pu.get(log.scenario_id).put(log.traffic_load, log.metrics.delay);

            }

        }

        double delay_dif;

        double ratio_best_pu_worst_am = 0;
        double load_best_pu_worst_am = -1;
        double delay_am_best_pu_worst_am = 0;
        double delay_pu_best_pu_worst_am = 0;

        for (int s = 1; s <= NUM_SCENARIOS; s++) {

//            System.out.println("- Scenario " + s);
            for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

//                System.out.println("  + Traffic load = " + TRAFFIC_LOADS[l]);
                delay_dif = delay_pu.get(s).get(TRAFFIC_LOADS[l]) - delay_am.get(s).get(TRAFFIC_LOADS[l]);
//                System.out.println("    * Delays AM / PU: "
//                        + delay_am.get(s).get(TRAFFIC_LOADS[l])
//                        + " / " + delay_pu.get(s).get(TRAFFIC_LOADS[l])
//                        + " - (delay_dif) = " + delay_dif);

                if (delay_dif > DELAY_DELTA) {   // d_PU > d_AM

                    compare_delays_am_wins.replace(TRAFFIC_LOADS[l], compare_delays_am_wins.get(TRAFFIC_LOADS[l]) + 1);

                } else if (delay_dif < -DELAY_DELTA) {

                    compare_delays_pu_wins.replace(TRAFFIC_LOADS[l], compare_delays_pu_wins.get(TRAFFIC_LOADS[l]) + 1);

                    if (ratio_best_pu_worst_am
                            < delay_am.get(s).get(TRAFFIC_LOADS[l]) / delay_pu.get(s).get(TRAFFIC_LOADS[l])) {

                        if (!Double.isInfinite(delay_am.get(s).get(TRAFFIC_LOADS[l]) / delay_pu.get(s).get(TRAFFIC_LOADS[l]))) {
                            ratio_best_pu_worst_am = delay_am.get(s).get(TRAFFIC_LOADS[l]) / delay_pu.get(s).get(TRAFFIC_LOADS[l]);
                            load_best_pu_worst_am = TRAFFIC_LOADS[l];
                            delay_am_best_pu_worst_am = delay_am.get(s).get(TRAFFIC_LOADS[l]);
                            delay_pu_best_pu_worst_am = delay_pu.get(s).get(TRAFFIC_LOADS[l]);
                        }
                    }

                } else {

                    // do nothing
                }

            }

        }

        System.out.println("RATIO SALVAJE: " + ratio_best_pu_worst_am + " with load "
                + load_best_pu_worst_am + ". d_am = " + delay_am_best_pu_worst_am
                + " d_pu = " + delay_pu_best_pu_worst_am);

        // **** THROUGH SIMILAR TO LOAD ****
        boolean am_success = false;
        boolean pu_success = false;
        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            for (int s = 1; s <= NUM_SCENARIOS; s++) {

                // Check if current log accomplishes with through similar to load
                if (Math.abs(metrics_am.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_am.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {

                    am_success = true;
                }

                if (Math.abs(metrics_pu.get(TRAFFIC_LOADS[l]).get(s).throughput - metrics_pu.get(TRAFFIC_LOADS[l]).get(s).num_packets_generated / SIM_TIME)
                        < THROUGHPUT_DELTA_FACTOR * TRAFFIC_LOADS[l]) {

                    pu_success = true;
                }

                if (am_success && pu_success) {

                    delay_dif = delay_pu.get(s).get(TRAFFIC_LOADS[l]) - delay_am.get(s).get(TRAFFIC_LOADS[l]);

                    if (delay_dif > DELAY_DELTA) {   // d_PU > d_AM

                        sim_compare_delays_am_wins.replace(TRAFFIC_LOADS[l], sim_compare_delays_am_wins.get(TRAFFIC_LOADS[l]) + 1);

                    } else if (delay_dif < -DELAY_DELTA) {

                        sim_compare_delays_pu_wins.replace(TRAFFIC_LOADS[l], sim_compare_delays_pu_wins.get(TRAFFIC_LOADS[l]) + 1);

                    } else {
                        sim_compare_delays_draw.replace(TRAFFIC_LOADS[l], sim_compare_delays_draw.get(TRAFFIC_LOADS[l]) + 1);
                    }

                } else if (am_success && !pu_success) {
                    sim_compare_delays_am_wins.replace(TRAFFIC_LOADS[l], sim_compare_delays_am_wins.get(TRAFFIC_LOADS[l]) + 1);
                } else if (!am_success && pu_success) {
                    sim_compare_delays_pu_wins.replace(TRAFFIC_LOADS[l], sim_compare_delays_pu_wins.get(TRAFFIC_LOADS[l]) + 1);
                }

                am_success = false;
                pu_success = false;
            }
        }
    }

    public static void write_delay_comparison(String output_path) throws IOException {

        // Write results
        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "load (delta_delay = " + DELAY_DELTA + ")" + CSV_SEPARATOR
                + "AM" + CSV_SEPARATOR
                + "PU" + CSV_SEPARATOR
                + "Draw" + CSV_SEPARATOR;

        out.println(csv_header_line);

        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            line = TRAFFIC_LOADS[l] + CSV_SEPARATOR
                    + (compare_delays_am_wins.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR
                    + (compare_delays_pu_wins.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR
                    + ((NUM_SCENARIOS - compare_delays_am_wins.get(TRAFFIC_LOADS[l]) - compare_delays_pu_wins.get(TRAFFIC_LOADS[l])) * 100 / NUM_SCENARIOS);

            out.println(line);

        }

        out.close();

        System.out.println("File saved in " + output_path);

    }

    public static void write_sim_delay_comparison(String output_path) throws IOException {

        // Write results
        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "load (delta_delay = " + DELAY_DELTA + ")" + CSV_SEPARATOR
                + "AM" + CSV_SEPARATOR
                + "PU" + CSV_SEPARATOR
                + "DRAW";

        out.println(csv_header_line);

        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            line = TRAFFIC_LOADS[l] + CSV_SEPARATOR
                    + (sim_compare_delays_am_wins.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + (sim_compare_delays_pu_wins.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + (sim_compare_delays_draw.get(TRAFFIC_LOADS[l]));

            out.println(line);

        }

        out.close();

        System.out.println("File saved in " + output_path);

    }

    public static void write_prob_throughput_similar_load(String output_path) throws IOException {

        // Write results
        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "load (THROUGHPUT_DELTA_FACTOR = " + THROUGHPUT_DELTA_FACTOR + ")" + CSV_SEPARATOR
                + "OP" + CSV_SEPARATOR
                + "SCB" + CSV_SEPARATOR
                + "AM" + CSV_SEPARATOR
                + "PU" + CSV_SEPARATOR;

        out.println(csv_header_line);

        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            line = TRAFFIC_LOADS[l] + CSV_SEPARATOR
                    + (op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR
                    + (scb_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR
                    + (am_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR
                    + (pu_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l]) * 100 / NUM_SCENARIOS) + CSV_SEPARATOR;

            out.println(line);

        }

        out.close();

        System.out.println("File saved in " + output_path);

    }

    public static void write_average_delay_achieved_load(String output_path) throws IOException {

        // Write results
        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "load (THROUGHPUT_DELTA_FACTOR = " + THROUGHPUT_DELTA_FACTOR + ")" + CSV_SEPARATOR
                + "OP" + CSV_SEPARATOR
                + "SCB" + CSV_SEPARATOR
                + "AM" + CSV_SEPARATOR
                + "PU" + CSV_SEPARATOR
                + "std_OP" + CSV_SEPARATOR
                + "std_SCB" + CSV_SEPARATOR
                + "std_AM" + CSV_SEPARATOR
                + "std_PU" + CSV_SEPARATOR;

        out.println(csv_header_line);

        for (int l = 0; l < TRAFFIC_LOADS.length; l++) {

            line = TRAFFIC_LOADS[l] + CSV_SEPARATOR
                    + (op_non_sat_delay.get(TRAFFIC_LOADS[l]) / op_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + (scb_non_sat_delay.get(TRAFFIC_LOADS[l]) / scb_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + (am_non_sat_delay.get(TRAFFIC_LOADS[l]) / am_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + (pu_non_sat_delay.get(TRAFFIC_LOADS[l]) / pu_count_scenarios_load_accomplished.get(TRAFFIC_LOADS[l])) + CSV_SEPARATOR
                    + op_non_sat_delay_std.get(TRAFFIC_LOADS[l]) + CSV_SEPARATOR
                    + scb_non_sat_delay_std.get(TRAFFIC_LOADS[l]) + CSV_SEPARATOR
                    + am_non_sat_delay_std.get(TRAFFIC_LOADS[l]) + CSV_SEPARATOR
                    + pu_non_sat_delay_std.get(TRAFFIC_LOADS[l]) + CSV_SEPARATOR;

            out.println(line);

        }

        out.close();

        System.out.println("File saved in " + output_path);

    }

    public static void main(String args[]) throws IOException {

        String input_path = "script_output.txt";
        System.out.println("input_path: " + input_path);

        read_logs_file(input_path);

        initialize_sum_metrics();

        process_logs();

        average_metrics();
        
        get_std_delay_non_sat();

        String output_path = "generated_info.csv";

        save_processed_info(output_path);

        String output_path_delay = "delay_comparison.csv";
        compare_delay_pu_am();
        write_delay_comparison(output_path_delay);

        String output_path_probability = "probability_throughput_load_similar.csv";
        write_prob_throughput_similar_load(output_path_probability);

        String output_path_ref_delay = "delay_load_similar.csv";
        write_average_delay_achieved_load(output_path_ref_delay);

        String output_path_delay_comp_sim = "delay_comparison_sim.csv";
        write_sim_delay_comparison(output_path_delay_comp_sim);

        

        System.out.println("num_zero_delay_measures_op: " + num_zero_delay_measures_op);
        System.out.println("num_zero_delay_measures_scb: " + num_zero_delay_measures_scb);
        System.out.println("num_zero_delay_measures_am: " + num_zero_delay_measures_am);
        System.out.println("num_zero_delay_measures_pu: " + num_zero_delay_measures_pu);

    }

}
