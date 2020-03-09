package old;


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

/*
 * Program for generating deterministic node inputs in csv files
 */
/**
 *
 * @author Sergio Barrachina-Munoz (sergio.barrachina@upf.edu)
 */
public class Nodes_deterministic_generator {

    // USER PARAMETERS
    /* Variable
     - congestion_windows
     - number of WLANs
     - number of STAs
     */
    static int[] cong_win;
    static int[] num_wlans;
    static int[] num_stas;

    static double distance_between_aps;  // distance between 2 consecutive APs [m]

    static int primary_channel;
    static int min_channel_allowed;
    static int max_channel_allowed;
    static double tpc_min;
    static double tpc_default;
    static double tpc_max;
    static double cca_min;
    static double cca_default;
    static double cca_max;
    static double tx_antenna_gain;
    static double rx_antenna_gain;
    static int channel_bonding_model;
    static int modulation_default;
    static double central_freq;
    static double lambda;
    
    static String DICTIONARY[] = {"A","B","C","D","E","F","G","H","I","J","K","L",
    "M","N","O","P","Q","R","S","T","U","W","X","Y","Z",
    "AA","BA","CA","DA","EA","FA","GA","HA","IA","JA","KA","LA",
    "MA","NA","OA","PA","QA","RA","SA","TA","UA","WA","XA","YA","ZA",
    "AB","BB","CB","DB","EB","FB","GB","HB","IB","JB","KB","LB",
    "MB","NB","OB","PB","QB","RB","SB","TB","UB","WB","XB","YB","ZB"};

    // FIXED PARAMETERS
    /* Automatically generated
     - node_code	
     - node_type	
     - wlan_code	
     - destination_id (-1)
     */
    static int destination_id = -1;

    static int letter_counter;

    private static final char DEFAULT_SEPARATOR = ',';

    static String csv_output_directory = "output_csv";
    static String csv_filepath_root = "input_nodes";

    @SuppressWarnings("empty-statement")
    public static void input_attributes(String input_path) throws FileNotFoundException, IOException {

        System.out.println("input_path: " + input_path);

        String wlans_str = "";
        String stas_str = "";
        String cw_str = "";
        String distance_str = "";

        BufferedReader br = new BufferedReader(new FileReader(input_path));

        try {

            wlans_str = br.readLine();
            String[] items_wlans = wlans_str.split(",");
            num_wlans = new int[items_wlans.length];
            
            for (int i = 0; i < items_wlans.length; i++) {
                try {
                    num_wlans[i] = Integer.parseInt(items_wlans[i]);
                } catch (NumberFormatException nfe) {
                    //NOTE: write something here if you need to recover from formatting errors
                };
            }

            stas_str = br.readLine();
            String[] items_stas = stas_str.split(",");
            num_stas = new int[items_stas.length];

            for (int i = 0; i < items_stas.length; i++) {
                try {
                    num_stas[i] = Integer.parseInt(items_stas[i]);
                } catch (NumberFormatException nfe) {
                    //NOTE: write something here if you need to recover from formatting errors
                };
            }

            cw_str = br.readLine();

            String[] items_cw = cw_str.split(",");
            cong_win = new int[items_cw.length];

            for (int i = 0; i < items_cw.length; i++) {
                try {
                    cong_win[i] = Integer.parseInt(items_cw[i]);
                } catch (NumberFormatException nfe) {
                    //NOTE: write something here if you need to recover from formatting errors
                };
            }

            distance_between_aps = Double.parseDouble(br.readLine());

            primary_channel = Integer.parseInt(br.readLine());
            min_channel_allowed = Integer.parseInt(br.readLine());
            max_channel_allowed = Integer.parseInt(br.readLine());
            tpc_min = Double.parseDouble(br.readLine());
            tpc_default = Double.parseDouble(br.readLine());
            tpc_max = Double.parseDouble(br.readLine());
            cca_min = Double.parseDouble(br.readLine());
            cca_default = Double.parseDouble(br.readLine());
            cca_max = Double.parseDouble(br.readLine());
            tx_antenna_gain = Double.parseDouble(br.readLine());
            rx_antenna_gain = Double.parseDouble(br.readLine());
            channel_bonding_model = Integer.parseInt(br.readLine());
            modulation_default = Integer.parseInt(br.readLine());
            central_freq = Double.parseDouble(br.readLine());
            lambda = Double.parseDouble(br.readLine());

        } finally {
            br.close();
        }

    }

    public static void generate_csv() throws FileNotFoundException, IOException {

        File output_directory = new File(csv_output_directory);

        // if the directory does not exist, create it
        if (!output_directory.exists()) {
            System.out.println("creating directory: " + output_directory.getName());
            boolean result = false;

            try {
                output_directory.mkdir();
                result = true;
            } catch (SecurityException se) {
                //handle it
            }
            if (result) {
                System.out.println("DIR created");
            }
        } else {
            output_directory.delete();
        }

        int file_ix = 1;

        String csv_filepath;

        String filename_suffix = "";

        // num_wlans x num_stas x congestion_windows files
        for (int wlan_ix = 0; wlan_ix < num_wlans.length; wlan_ix++) {

            for (int stas_ix = 0; stas_ix < num_stas.length; stas_ix++) {

                for (int cw_ix = 0; cw_ix < cong_win.length; cw_ix++) {

                    // If file not open -> open it
                    // Add new line
                    System.out.println("\nfile " + file_ix + ": "
                            + num_wlans[wlan_ix] + DEFAULT_SEPARATOR
                            + num_stas[stas_ix] + DEFAULT_SEPARATOR
                            + cong_win[cw_ix]);

                    filename_suffix = "_"
                            + "w" + num_wlans[wlan_ix] + "_"
                            + "s" + num_stas[stas_ix] + "_"
                            + "cw" + cong_win[cw_ix] + ".csv";

                    csv_filepath = csv_output_directory + "/"
                            + csv_filepath_root
                            + filename_suffix;

                    genearate_file(csv_filepath, num_wlans[wlan_ix],
                            num_stas[stas_ix], cong_win[cw_ix]);

                    file_ix++;

                }
            }

        }

    }

    public static void genearate_file(String csv_filepath, int num_wlans,
            int num_stas, int cong_win) throws UnsupportedEncodingException, FileNotFoundException, IOException {

        File fileTemp = new File(csv_filepath);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        int num_lines = num_wlans * (num_stas + 1);

        String node_code = "";
        int node_type;
        String wlan_code = "";
        int sta_ix = 0;
        String line = "";
        letter_counter = 0;

        double x = 0;
        double y = 0;
        double z = 0;

        Writer fw = new FileWriter(csv_filepath, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String csv_header_line = "node_code" + DEFAULT_SEPARATOR
                + "node_type" + DEFAULT_SEPARATOR
                + "wlan_code" + DEFAULT_SEPARATOR
                + "destination_id" + DEFAULT_SEPARATOR
                + "x(m)" + DEFAULT_SEPARATOR
                + "y(m)" + DEFAULT_SEPARATOR
                + "z(m)" + DEFAULT_SEPARATOR
                + "primary_channel" + DEFAULT_SEPARATOR
                + "min_channel_allowed" + DEFAULT_SEPARATOR
                + "max_channel_allowed" + DEFAULT_SEPARATOR
                + "Cwmin" + DEFAULT_SEPARATOR
                + "Cwmax" + DEFAULT_SEPARATOR
                + "tpc_min(dBm)" + DEFAULT_SEPARATOR
                + "tpc_default(dBm)" + DEFAULT_SEPARATOR
                + "tpc_max(dBm)" + DEFAULT_SEPARATOR
                + "cca_min(dBm)" + DEFAULT_SEPARATOR
                + "cca_default(dBm)" + DEFAULT_SEPARATOR
                + "cca_max(dBm)" + DEFAULT_SEPARATOR
                + "tx_antenna_gain" + DEFAULT_SEPARATOR
                + "rx_antenna_gain" + DEFAULT_SEPARATOR
                + "channel_bonding_model" + DEFAULT_SEPARATOR
                + "modulation_default" + DEFAULT_SEPARATOR
                + "central_freq (GHz)" + DEFAULT_SEPARATOR
                + "lambda";

        out.println(csv_header_line);

        for (int line_ix = 0; line_ix < num_lines; line_ix++) {

            if (line_ix % (num_stas + 1) == 0) {
                node_type = 0;
                wlan_code = DICTIONARY[letter_counter];
                node_code = "AP_";
                x = x + distance_between_aps;
                y = 0;

                letter_counter++;

                node_code = node_code.concat(wlan_code);
            } else {
                node_type = 1;
                y = 0.1;
                node_code = "STA_";
                sta_ix = line_ix % (num_stas + 1);
                node_code = node_code.concat(wlan_code).concat("" + sta_ix);

            }

            // node_code node_type wlan_code destination CWmin CWmax
            line = node_code + DEFAULT_SEPARATOR
                    + node_type + DEFAULT_SEPARATOR
                    + wlan_code + DEFAULT_SEPARATOR
                    + destination_id + DEFAULT_SEPARATOR
                    + x + DEFAULT_SEPARATOR
                    + y + DEFAULT_SEPARATOR
                    + z + DEFAULT_SEPARATOR
                    + primary_channel + DEFAULT_SEPARATOR
                    + min_channel_allowed + DEFAULT_SEPARATOR
                    + max_channel_allowed + DEFAULT_SEPARATOR
                    + cong_win + DEFAULT_SEPARATOR
                    + cong_win + DEFAULT_SEPARATOR
                    + tpc_min + DEFAULT_SEPARATOR
                    + tpc_default + DEFAULT_SEPARATOR
                    + tpc_max + DEFAULT_SEPARATOR
                    + cca_min + DEFAULT_SEPARATOR
                    + cca_default + DEFAULT_SEPARATOR
                    + cca_max + DEFAULT_SEPARATOR
                    + tx_antenna_gain + DEFAULT_SEPARATOR
                    + rx_antenna_gain + DEFAULT_SEPARATOR
                    + channel_bonding_model + DEFAULT_SEPARATOR
                    + modulation_default + DEFAULT_SEPARATOR
                    + central_freq + DEFAULT_SEPARATOR
                    + lambda;

            System.out.println(line);

             out.print(line);
             
            if(line_ix < num_lines - 1){
               out.print("\n");
            }

            // file_in_text = file_in_text.concat(line + "\n");
        }
        out.close();

    }

    public static void main(String args[]) throws IOException {

         // String input_path = args[0];
        String input_path = "input_template_rectangular.csv";
        
        input_attributes(input_path);

        generate_csv();

    }

}
