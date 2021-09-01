import java.awt.geom.Point2D;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.text.NumberFormat;
import java.util.Locale;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

/*
 * Program for Toy scenario 1 of the paper "Spatial Reuse in IEEE 802.11ax: 
Analysis, Challenges and Opportunities"
 * - Scenario considering:
 *      + BSS Color for OBSS PD-based SR 
 *      + Tune the OBSS PD threshold
 */

/**
 *
 * @author Sergio Barrachina-Munoz (sergio.barrachina@upf.edu)
 *  - Adapted by Francesc Wilhelmi (francisco.wilhelmi@upf.edu)
 */

public class toy_scenario_2_brute_force {

    static Wlan[] wlan_container = null;

    // USER PARAMETERS (input)
    static double map_width;       // map_width [m]
    static double map_heigth;      // map_heigth [m]
    static int num_wlans;          // number of WLANs (M)
    static int num_sta_min;        // min. number of STAs per WLAN (N_MAX)
    static int num_sta_max;        // max. number of STAs per WLAN (N_MAX)
    static double d_min_AP_AP;     // min. distance between APs
    static double d_min_AP_STA;    // min. distance between AP and STA in same WLAN
    static double d_max_AP_STA;    // max. distance between AP and STA in same WLAN
    static double legacy_ratio;    // proportion of legacy devices (vs. IEEE 802.11 ax)
    static int c_sys_width;         // system channel width
    static int cw_adaptation;
    static int cont_wind;          // contention window (CW)
    static int cont_wind_stage;     // CW stage 
    static int channel_bonding_model;
    static int traffic_model;      
    static double traffic_load;
    static int bss_color_input;
    static int srg_input;
    static int non_srg_obss_pd_input;      // OBSS_PD_INPUT
    static int srg_obss_pd_input;
    static int tpc_default_input;
    static int cca_default_input;
    static double central_freq;
    static int packet_length;
    static int num_packets_aggregated;
    static int capture_effect_model;
    static int capture_effect_thr;
    static double constant_per;
    static int pifs_activated;
    
    // -------------------
    
    static final int MW_TO_DBM = 1;
    static final int DBM_TO_MW = 2;

    static final String DICTIONARY[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
        "M", "N", "O", "P", "Q", "R", "S", "T", "U", "W", "X", "Y", "Z",
        "AA", "BA", "CA", "DA", "EA", "FA", "GA", "HA", "IA", "JA", "KA", "LA",
        "MA", "NA", "OA", "PA", "QA", "RA", "SA", "TA", "UA", "WA", "XA", "YA", "ZA",
        "AB", "BB", "CB", "DB", "EB", "FB", "GB", "HB", "IB", "JB", "KB", "LB",
        "MB", "NB", "OB", "PB", "QB", "RB", "SB", "TB", "UB", "WB", "XB", "YB", "ZB",
        "AC", "BC", "CC", "DC", "EC", "FC", "GC", "HC", "IC", "JC", "KC", "LC",
        "MC", "NC", "OC", "PC", "QC", "RC", "SC", "TC", "UC", "WC", "XC", "YC", "ZC",
        "AD", "BD", "CD", "DD", "ED", "FD", "GD", "HD", "ID", "JD", "KD", "LD",
        "MD", "ND", "OD", "PD", "QD", "RD", "SD", "TD", "UD", "WD", "XD", "YD", "ZD"};

    static final String CSV_SEPARATOR = ";";

    @SuppressWarnings("empty-statement")
    /*
     Read the user input file
     */
    public static void input_attributes(String input_path) throws FileNotFoundException, IOException {

        String line;

        System.out.println("Reading input file...");

        boolean first_line_skipped = false;

        try (BufferedReader br = new BufferedReader(new FileReader(input_path))) {
            while ((line = br.readLine()) != null) {
                if (!first_line_skipped) {
                    // It is a comment --> do nothing
                    first_line_skipped = true;
                } else {

                    String[] node_info = line.split(CSV_SEPARATOR);

                    map_width = Double.parseDouble(node_info[0]);
                    map_heigth = Double.parseDouble(node_info[1]);
                    num_wlans = Integer.parseInt(node_info[2]);
                    wlan_container = new Wlan[num_wlans];
                    num_sta_min = Integer.parseInt(node_info[3]);
                    num_sta_max = Integer.parseInt(node_info[4]);
                    d_min_AP_AP = Double.parseDouble(node_info[5]);
                    d_min_AP_STA = Double.parseDouble(node_info[6]);
                    d_max_AP_STA = Double.parseDouble(node_info[7]);
                    legacy_ratio = Double.parseDouble(node_info[8]);
                    c_sys_width = Integer.parseInt(node_info[9]);
                    cw_adaptation = Integer.parseInt(node_info[10]);
                    cont_wind = Integer.parseInt(node_info[11]);
                    cont_wind_stage = Integer.parseInt(node_info[12]);
                    tpc_default_input = Integer.parseInt(node_info[13]);
                    cca_default_input = Integer.parseInt(node_info[14]);
                    channel_bonding_model = Integer.parseInt(node_info[15]);
                    central_freq = Double.parseDouble(node_info[16]);
                    traffic_model = Integer.parseInt(node_info[17]);
                    traffic_load = Integer.parseInt(node_info[18]);  
                    packet_length = Integer.parseInt(node_info[19]);  
                    num_packets_aggregated = Integer.parseInt(node_info[20]);  
                    capture_effect_model = Integer.parseInt(node_info[21]);  
                    capture_effect_thr = Integer.parseInt(node_info[22]);  
                    constant_per = Double.parseDouble(node_info[23]);  
                    pifs_activated = Integer.parseInt(node_info[24]); 
                    
//                    bss_color_input = Integer.parseInt(node_info[26]);
//                    srg_input = Integer.parseInt(node_info[27]);
//                    non_srg_obss_pd_input = Integer.parseInt(node_info[28]);
//                    srg_obss_pd_input = Integer.parseInt(node_info[29]);                  
                    
                    System.out.println(
                        "Input:"
                        + "\n- map_width: " + map_width
                        + "\n- map_heigth: " + map_heigth
                        + "\n- num_wlans: " + num_wlans
                        + "\n- num_sta_min: " + num_sta_min
                        + "\n- num_sta_max: " + num_sta_max
                        + "\n- d_min_AP_AP: " + d_min_AP_AP
                        + "\n- d_min_AP_STA: " + d_min_AP_STA
                        + "\n- d_max_AP_STA: " + d_max_AP_STA
                        + "\n- legacy_ratio: " + legacy_ratio
                        + "\n- c_sys_width: " + c_sys_width
                        + "\n- cca_default_input: " + cca_default_input                                
                        + "\n- cont_wind: " + cont_wind);
//                        + "\n- bss_color_input: " + bss_color_input
//                        + "\n- srg_input: " + srg_input
//                        + "\n- non_srg_obss_pd_input: " + non_srg_obss_pd_input
//                        + "\n- srg_obss_pd_input: " + srg_obss_pd_input
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void generate_wlans(int[] sensitivity, int[] tx_power) {

        int wlan_counter = 0;

        Wlan wlan_aux;

        int wlan_id;
        String wlan_code;
        int num_stas;
        String ap_code;
        String[] list_sta_code = null;
        int primary_channel;
        int num_channels_allowed_ix;
        int min_ch_allowed = 0;
        int max_ch_allowed = 0;
        boolean wlan_80211ax;   // Legacy or IEEE 802.11 ax
        double x = 0;
        double y = 0;
        double z = 0;
        int channel_bonding_aux;

//        double angle = 0;
        double d_ap_sta = 0;
        double rand_value;

        boolean aps_separated;  // Relocate APs until separated

        for (int w = 0; w < num_wlans; w++) {

            System.out.println("Setting WLAN " + w + "/" + num_wlans + 1);

            wlan_id = wlan_counter;
            wlan_code = DICTIONARY[wlan_counter];
            num_stas = ThreadLocalRandom.current().nextInt(num_sta_min, num_sta_max + 1);
            ap_code = "AP_" + wlan_code;
            list_sta_code = new String[num_stas];

            for (int n = 0; n < num_stas; n++) {
                list_sta_code[n] = "STA_" + wlan_code + "" + (n + 1);
            }

            // Channel allocation
            primary_channel = ThreadLocalRandom.current().nextInt(0, c_sys_width);
            min_ch_allowed = primary_channel;
            max_ch_allowed = primary_channel;
            
            // IEEE 802.11ax or legacy device
            wlan_80211ax = new Random().nextDouble() <= (1 - legacy_ratio);

            // AP position
            while (true) {
                // If WLAN A
                if (w == 0) {
                    x = 6;
                } else {
                    x = 0;
                }
                break;
            }
            
            y = 0;
            z = 0;

            channel_bonding_aux = 0;
                                     
            wlan_aux = new Wlan(wlan_id, wlan_code, num_stas, ap_code,
                list_sta_code, primary_channel, min_ch_allowed,
                max_ch_allowed, wlan_80211ax, x, y, z, 
                channel_bonding_model, traffic_load);                   
            
            // Default sensitivity & transmit power
            wlan_aux.cca_default = sensitivity[w];
            wlan_aux.tpc_default = tx_power[w];
//            // Spatial Reuse Operation
//            wlan_aux.bss_color = -1;
//            wlan_aux.spatial_reuse_group = -1;
//            wlan_aux.non_srg_obss_pd = non_srg_obss_pd_input;
//            wlan_aux.srg_obss_pd = srg_obss_pd_input;
                        
            Point2D.Double[] stas_position_list = new Point2D.Double[wlan_aux.num_stas];

            // Set STAs location
            //  - STAs are at the same x and z axis than their AP
            Point2D.Double point = new Point2D.Double();
            if (w == 0) {
                point.setLocation(9, 0);
            } else if (w == 1) {
                point.setLocation(4, 0);
            }              

            stas_position_list[0] = point;
            wlan_aux.set_stas_positions(stas_position_list);
            
            wlan_container[w] = wlan_aux;
            wlan_counter++;
            
        }
       
    }

    public static void generate_file(String csv_filepath_output)
            throws UnsupportedEncodingException, FileNotFoundException, IOException {

        System.out.println("Generating simulation file...");

        File fileTemp = new File(csv_filepath_output);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(csv_filepath_output, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;
        int line_ix = 0;
        int node_type = 0;
        Wlan wlan;
        double x, y, z = 0;
        double angle = 0;
        double d_ap_sta = 0;
        double rand_value;
        
        // Generate Headers
        String csv_header_line = "node_code" + CSV_SEPARATOR
                + "node_type" + CSV_SEPARATOR
                + "wlan_code" + CSV_SEPARATOR
                + "x(m)" + CSV_SEPARATOR
                + "y(m)" + CSV_SEPARATOR
                + "z(m)" + CSV_SEPARATOR
                + "central_freq (GHz)" + CSV_SEPARATOR
                + "channel_bonding_model" + CSV_SEPARATOR
                + "primary_channel" + CSV_SEPARATOR
                + "min_channel_allowed" + CSV_SEPARATOR
                + "max_channel_allowed" + CSV_SEPARATOR
                + "tpc_default(dBm)" + CSV_SEPARATOR
                + "cca_default(dBm)" + CSV_SEPARATOR
                + "traffic_model" + CSV_SEPARATOR
                + "traffic_load[pkt/s]" + CSV_SEPARATOR
                + "packet_length" + CSV_SEPARATOR
                + "num_packets_aggregated" + CSV_SEPARATOR
                + "capture_effect_model" + CSV_SEPARATOR
                + "capture_effect_thr" + CSV_SEPARATOR
                + "constant_per" + CSV_SEPARATOR
                + "pifs_activated" + CSV_SEPARATOR
                + "cw_adaptation" + CSV_SEPARATOR         
                + "cont_wind" + CSV_SEPARATOR // CW
                + "cont_wind_stage";
//                + "bss_color" + CSV_SEPARATOR
//                + "spatial_reuse_group" + CSV_SEPARATOR
//                + "non_srg_obss_pd" + CSV_SEPARATOR
//                + "srg_obss_pd";

        // System.out.println(csv_header_line);
        out.println(csv_header_line);

        for (int w = 0; w < num_wlans; w++) {

            wlan = wlan_container[w];
            node_type = 0;
            
            line = getCompleteLine(wlan.ap_code, node_type,
                wlan.bss_code, wlan.x, wlan.y, wlan.z, wlan.primary_channel,
                wlan.min_ch_allowed, wlan.max_ch_allowed,
                wlan.channel_bonding_model, wlan.tpc_default, wlan.cca_default);
            
//            line = getCompleteLine(wlan.ap_code, node_type,
//                wlan.wlan_code, wlan.x, wlan.y, wlan.z, wlan.primary_channel,
//                wlan.min_ch_allowed, wlan.max_ch_allowed,
//                wlan.channel_bonding_model, wlan.tpc_default, wlan.cca_default,
//                wlan.bss_color, wlan.spatial_reuse_group, 
//                wlan.non_srg_obss_pd, wlan.srg_obss_pd);

            // System.out.println(line);
            out.println(line);

            node_type = 1;

            // Set STAs location
            for (int n = 0; n < wlan.num_stas; n++) {
                
                line = getCompleteLine(wlan.list_sta_code[n], node_type,
                    wlan.bss_code, wlan.stas_position_list[n].x, 
                    wlan.stas_position_list[n].y, 0, wlan.primary_channel,
                    wlan.min_ch_allowed, wlan.max_ch_allowed,
                    wlan.channel_bonding_model, wlan.tpc_default, wlan.cca_default);
                
//                line = getCompleteLine(wlan.list_sta_code[n], node_type,
//                    wlan.wlan_code, wlan.stas_position_list[n].x, 
//                    wlan.stas_position_list[n].y, 0, wlan.primary_channel,
//                    wlan.min_ch_allowed, wlan.max_ch_allowed,
//                    wlan.channel_bonding_model, wlan.tpc_default, wlan.cca_default,
//                    wlan.bss_color, wlan.spatial_reuse_group, 
//                    wlan.non_srg_obss_pd, wlan.srg_obss_pd);

                // System.out.println(line);
                out.println(line);
            }

        }

        out.close();

        System.out.println("File saved in " + csv_filepath_output);
    }

    static String getCompleteLine(String node_code, int node_type,
            String wlan_code, double x, double y, double z, int primary_channel, 
            int min_channel_allowed, int max_channel_allowed, 
            int channel_bonding_model, int tpc_default, int cca_default) { 
//            int bss_color, int spatial_reuse_group, int non_srg_obss_pd, int srg_obss_pd) {

        // Round position coordinates to limited number of decimals
        NumberFormat nf = NumberFormat.getNumberInstance(Locale.UK);
        nf.setGroupingUsed(true);
        nf.setMaximumFractionDigits(4);
                        
        String line = node_code + CSV_SEPARATOR
                + node_type + CSV_SEPARATOR
                + wlan_code + CSV_SEPARATOR
                + nf.format(x) + CSV_SEPARATOR
                + nf.format(y) + CSV_SEPARATOR
                + nf.format(z) + CSV_SEPARATOR
                + central_freq + CSV_SEPARATOR
                + channel_bonding_model + CSV_SEPARATOR
                + primary_channel + CSV_SEPARATOR
                + min_channel_allowed + CSV_SEPARATOR
                + max_channel_allowed + CSV_SEPARATOR
                + tpc_default + CSV_SEPARATOR
                + cca_default + CSV_SEPARATOR
                + traffic_model + CSV_SEPARATOR
                + traffic_load + CSV_SEPARATOR
                + packet_length + CSV_SEPARATOR
                + num_packets_aggregated + CSV_SEPARATOR
                + capture_effect_model + CSV_SEPARATOR
                + capture_effect_thr + CSV_SEPARATOR
                + constant_per + CSV_SEPARATOR
                + pifs_activated + CSV_SEPARATOR
                + cw_adaptation + CSV_SEPARATOR         
                + cont_wind + CSV_SEPARATOR // CW
                + cont_wind_stage;// + CSV_SEPARATOR // CW's max stage
//                + bss_color + CSV_SEPARATOR  
//                + spatial_reuse_group + CSV_SEPARATOR  
//                + non_srg_obss_pd  + CSV_SEPARATOR  
//                + srg_obss_pd; 
        return line;
    }

    static int log(int x, int base) {
        return (int) (Math.log(x) / Math.log(base));
    }

    static double getDistance(double x1, double x2, double y1, double y2, double z1, double z2) {
        return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
    }
    
    static double ConvertPower(int conversion_type, double power_magnitude_in) {
        
        // MW_TO_DBM
        if (conversion_type == MW_TO_DBM) {
           return 10 * Math.log10(power_magnitude_in);
        // DBM_TO_MW    
        } else if (conversion_type == DBM_TO_MW) {
            return Math.pow(10,(power_magnitude_in/10));
        } else {
            return 0;
        }      
        
    }

    public static void main(String args[]) throws IOException {

        String input_path = "./input_constructor/toy_scenarios/input_template_toy_scenario_1.csv";
        System.out.println("input_path: " + input_path);
        String output_path = "./output/*";

//        // DEFINE THE CCA VALUES TO BE USED
//        int[] non_srg_obss_pd_list = new int[-62 + 82 + 1];  
//        for (int i = 0; i < -62 + 82 + 1; i ++) {
//            non_srg_obss_pd_list[i] = -82 + i;
//        }
        
        int[] sensitvity_list = new int[3];  
        sensitvity_list[0] = -82;
        sensitvity_list[1] = -72;
        sensitvity_list[2] = -65;
        
        int[] tx_power_list = new int[3];  
        tx_power_list[0] = 1;
        tx_power_list[1] = 10;
        tx_power_list[2] = 20;
        
        input_attributes(input_path);
                               
        int sce_id = 0;
        
        for (int i = 0; i < sensitvity_list.length; i++) {                        
            for (int j = 0; j < tx_power_list.length; j++) {                         
//                System.out.println("WLAN1" + "; " + sensitvity_list[i] + "; " + tx_power_list[j]);
               for (int i2 = 0; i2 < sensitvity_list.length; i2++) {                        
                    for (int j2 = 0; j2 < tx_power_list.length; j2++) {  
//                        System.out.println("WLAN2" + "; " + sensitvity_list[i2] + "; " + tx_power_list[j2]);
                        generate_wlans(new int[] {sensitvity_list[i], sensitvity_list[i2]}, 
                            new int[] {tx_power_list[j], tx_power_list[j2]});
                        output_path = "./output/input_nodes_n" + num_wlans + "_s" + String.format("%03d", sce_id) +
                            "_sens1_" + String.format("%03d", (int) sensitvity_list[i]) + 
                            "_txp1_" + String.format("%02d", (int) tx_power_list[j]) +
                            "_sens2_" + String.format("%03d", (int) sensitvity_list[i2]) +
                            "_txp2_" + String.format("%02d", (int) tx_power_list[j2]) + ".csv";
                        System.out.println("output_path: " + output_path);
                        generate_file(output_path);                          
                        sce_id ++;
                    }
                }
            }                   
        }  
        
    }        

}
