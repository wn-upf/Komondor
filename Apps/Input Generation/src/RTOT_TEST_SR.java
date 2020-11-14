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
 * Program for generating the enterprise-like scenarios used in the ITU AI Challenge (UPF)"
 * - Scenario considering:
 *      + Random channel allocation 
 *      + Random location of STAs wrt to their AP */

/**
 *
 * @author Francesc Wilhelmi (francisco.wilhelmi@upf.edu)
 */

public class RTOT_TEST_SR {

    static Wlan[] bss_container = null;

    // USER PARAMETERS (input)
    static double map_width;       // map_width [m]
    static double map_heigth;      // map_heigth [m]
    static int num_bss;          // number of WLANs (M)
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
    static int traffic_model = 99;      
    static double traffic_load;
    static int tpc_default_input;
    static int cca_default_input;
    static double central_freq;
    static int packet_length = 12000;
    static int num_packets_aggregated = 64;
    static int capture_effect_model = 0;
    static int capture_effect_thr = 10;
    static double constant_per = 0;
    static int pifs_activated = 0;
    static int bss_color_input;
    static int srg_input = 0;
    static int non_srg_obss_pd_input;      // OBSS_PD_INPUT
    static int srg_obss_pd_input;
    
    static final int MW_TO_DBM = 1;
    static final int DBM_TO_MW = 2;
    
    static int list_of_ch_width[] = new int[]{2,4,8};
    static int list_of_primary_ch2[] = new int[]{0,2,4,6};
    static int list_of_primary_ch4[] = new int[]{0,4};
    
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
                    num_bss = Integer.parseInt(node_info[2]);
                    bss_container = new Wlan[num_bss];
                    num_sta_min = Integer.parseInt(node_info[3]);
                    num_sta_max = Integer.parseInt(node_info[4]);
                    d_min_AP_AP = Double.parseDouble(node_info[5]);
                    d_min_AP_STA = Double.parseDouble(node_info[6]);
                    d_max_AP_STA = Double.parseDouble(node_info[7]);
                    legacy_ratio = Double.parseDouble(node_info[8]);
                    c_sys_width = Integer.parseInt(node_info[9]);
                    cont_wind = Integer.parseInt(node_info[10]);
                    cont_wind_stage = Integer.parseInt(node_info[11]);
                    tpc_default_input = Integer.parseInt(node_info[13]);
                    cca_default_input = Integer.parseInt(node_info[16]);
                    channel_bonding_model = Integer.parseInt(node_info[20]);
                    central_freq = Double.parseDouble(node_info[22]);
                    traffic_load = Integer.parseInt(node_info[25]);                    
                    bss_color_input = Integer.parseInt(node_info[26]);
                    srg_input = Integer.parseInt(node_info[27]);
                    non_srg_obss_pd_input = Integer.parseInt(node_info[28]);
                    srg_obss_pd_input = Integer.parseInt(node_info[29]);
                    
                    System.out.println("Input:"
                        + "\n- map_width: " + map_width
                        + "\n- map_heigth: " + map_heigth
                        + "\n- num_wlans: " + num_bss
                        + "\n- num_sta_min: " + num_sta_min
                        + "\n- num_sta_max: " + num_sta_max
                        + "\n- d_min_AP_AP: " + d_min_AP_AP
                        + "\n- d_min_AP_STA: " + d_min_AP_STA
                        + "\n- d_max_AP_STA: " + d_max_AP_STA
                        + "\n- legacy_ratio: " + legacy_ratio
                        + "\n- c_sys_width: " + c_sys_width
                        + "\n- cca_default_input: " + cca_default_input                                
                        + "\n- cont_wind: " + cont_wind
                        + "\n- bss_color_input: " + bss_color_input
                        + "\n- srg_input: " + srg_input
                        + "\n- non_srg_obss_pd_input: " + non_srg_obss_pd_input
                        + "\n- srg_obss_pd_input: " + srg_obss_pd_input);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void generate_bss(Point2D.Double[] aps_position_list) {
        
        int bss_counter = 0;
        Wlan bss_aux;
        int bss_id;
        String bss_code;
        int num_stas;
        String ap_code;
        String[] list_sta_code = null;
        int primary_channel;
        int min_ch_allowed = 0;
        int max_ch_allowed = 0;
        boolean wlan_80211ax;   // Legacy or IEEE 802.11 ax
        double x = 0;
        double y = 0;
        double z = 0;
        
        // Random number of STA per BSS
        Random r = new Random();
        double x_sta = 0;
        double y_sta = 0;
        double angle = 0;
        double rand_value = 0;
        double d_ap_sta = 0;

        for (int i = 0; i < num_bss; i++) {

            System.out.println("  - Setting BSS " + (i+1) + "/" + num_bss);
            
            bss_id = bss_counter;
            bss_code = DICTIONARY[bss_counter];
            num_stas = ThreadLocalRandom.current().nextInt(num_sta_min, num_sta_max + 1);
            ap_code = "AP_" + bss_code;
            
            // Generate the number of STAs randomly
            Random r_stas = new Random();
            num_stas = r_stas.nextInt((num_sta_max - num_sta_min) + 1) + num_sta_min;
            list_sta_code = new String[num_stas];
            for (int j = 0; j < num_stas; ++j) {
                list_sta_code[j] = "STA_" + bss_code + "" + (j + 1);
            }

            primary_channel = 0;
            min_ch_allowed = 0;
            max_ch_allowed = 0;//primary_channel + ch_width - 1;
            
            //primary_channel = 0;
            //min_ch_allowed = 0;
            //max_ch_allowed = 7;      
            
            // IEEE 802.11ax or legacy device
            wlan_80211ax = new Random().nextDouble() <= (1 - legacy_ratio);
             
            // Create an auxiliary BSS object
            bss_aux = new Wlan(bss_id, bss_code, num_stas, ap_code,
                list_sta_code, primary_channel, min_ch_allowed,
                max_ch_allowed, wlan_80211ax, x, y, z, channel_bonding_model, traffic_load);

            // Set APs location       
            bss_aux.x = aps_position_list[i].x;
            bss_aux.y = aps_position_list[i].y;
            bss_aux.z = 0;
            
            // Generate STAs' location randomly            
            bss_aux.num_stas = num_stas;
            Point2D.Double[] stas_position_list = new Point2D.Double[num_stas];   
            for (int j = 0; j < num_stas; j++) {
                angle = 360 * new Random().nextDouble();
                rand_value = new Random().nextDouble();
                d_ap_sta = d_min_AP_STA + Math.sqrt(rand_value) * (d_max_AP_STA - d_min_AP_STA);
                x_sta = aps_position_list[i].x + Math.cos(Math.toRadians(angle)) * d_ap_sta;
                y_sta = aps_position_list[i].y + Math.sin(Math.toRadians(angle)) * d_ap_sta;
                Point2D.Double point_sta = new Point2D.Double();
                point_sta.setLocation(x_sta, y_sta);
                stas_position_list[j] = point_sta;     
//                double distance = getDistance(aps_position_list[i].x, 
//                    stas_position_list[j].x, aps_position_list[i].y, stas_position_list[j].y, z, z);
//                System.out.println("Distance = " + distance);                
            }
            bss_aux.set_stas_positions(stas_position_list);
                        
            // Default sensitivity & transmit power
            bss_aux.cca_default = cca_default_input;
            bss_aux.tpc_default = tpc_default_input;
            
            bss_aux.bss_color = bss_id + 1;
            bss_aux.spatial_reuse_group = 0;
            bss_aux.non_srg_obss_pd = -82;
            bss_aux.srg_obss_pd = -82;            
                       
            // Add the BSS to the container
            bss_container[i] = bss_aux;
            bss_counter++;
                        
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
        Wlan bss;
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
                + "central_freq(GHz)" + CSV_SEPARATOR
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
                + "cont_wind_stage" + CSV_SEPARATOR
                + "bss_color" + CSV_SEPARATOR
                + "spatial_reuse_group" + CSV_SEPARATOR
                + "non_srg_obss_pd" + CSV_SEPARATOR
                + "srg_obss_pd";

        // System.out.println(csv_header_line);
        out.println(csv_header_line);

        for (int w = 0; w < num_bss; w++) {

            bss = bss_container[w];
            node_type = 0;
            
            line = getCompleteLine(bss.ap_code, node_type,
                bss.bss_code, bss.x, bss.y, bss.z, bss.primary_channel,
                bss.min_ch_allowed, bss.max_ch_allowed,
                bss.channel_bonding_model, bss.tpc_default, bss.cca_default,
                bss.bss_color, bss.spatial_reuse_group, bss.non_srg_obss_pd, bss.srg_obss_pd);

            // System.out.println(line);
            out.println(line);

            node_type = 1;

            // Set STAs location
            for (int n = 0; n < bss.num_stas; n++) {    
//                System.out.println("   + STA: " + bss.stas_position_list[n]);
                line = getCompleteLine(bss.list_sta_code[n], node_type,
                    bss.bss_code, bss.stas_position_list[n].x, 
                    bss.stas_position_list[n].y, 0, bss.primary_channel,
                    bss.min_ch_allowed, bss.max_ch_allowed,
                    bss.channel_bonding_model, bss.tpc_default, bss.cca_default,
                    bss.bss_color, bss.spatial_reuse_group, 
                    bss.non_srg_obss_pd, bss.srg_obss_pd);    
                // System.out.println(line);
                out.println(line);
            }

        }

        out.close();

        System.out.println("File saved in " + csv_filepath_output);
    }

    static String getCompleteLine(String node_code, int node_type,
            String wlan_code, double x, double y, double z, int primary_channel,
            int min_channel_allowed, int max_channel_allowed, int channel_bonding_model,
            int tpc_default, int cca_default, int bss_color, int spatial_reuse_group,
            int non_srg_obss_pd, int srg_obss_pd) {

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
                + cont_wind_stage + CSV_SEPARATOR
                + bss_color + CSV_SEPARATOR
                + spatial_reuse_group + CSV_SEPARATOR
                + non_srg_obss_pd + CSV_SEPARATOR                
                + srg_obss_pd;;// + CSV_SEPARATOR // CW's max stage
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

        // Complete path building
        String input_path = "./input_constructor/template_ai_challenge_sce1a_sr.csv";
        System.out.println("input_path: " + input_path);
        String output_path = "./output/*";

        input_attributes(input_path);
        
        int num_random_deployments = 50;
        
        double x = 0;
        double y = 0;
                
        // Fix APs' location
        Point2D.Double[] array_ap_locations = new Point2D.Double[num_bss];
        int num_rows = 3;
        int num_cols = 4;
        int aux_row_counter = 0;
        for (int i = 0; i < num_bss; ++i) {
            Point2D.Double ap_location = new Point2D.Double();
            x = map_width/(num_cols*2) + (i%num_cols)*(map_width/num_cols);
            y = map_heigth/(num_rows*2) + aux_row_counter*(map_heigth/num_rows);
            if (i%num_cols == num_cols-1) aux_row_counter ++ ;
            ap_location.setLocation(x, y);
            array_ap_locations[i] = ap_location;  
            //System.out.println(ap_location);
        }
        
        // Generate the random .csv files of "input_nodes"
        for (int i = 0; i < num_random_deployments; ++i) {   
            // Generate BSSs
            generate_bss(array_ap_locations);
            // Specify the output path (file's name)
            output_path = "./output/input_nodes_sce1a_deployment_" + String.format("%03d", i) + ".csv";
            System.out.println("output_path: " + output_path);
            // Generate the .csv file
            generate_file(output_path);  
        }
        
    }
        
}