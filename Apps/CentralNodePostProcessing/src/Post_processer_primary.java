
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Writer;
import java.sql.*;
import java.util.HashMap;

/*
 * This class is for post processing the logs obtained for the primary channel
 * allocation analysis.
 */
/**
 *
 * @author Sergio Barrachina-Muñoz
 */
public class Post_processer_primary {

    static final String CSV_SEPARATOR = ";";
    static final int NUM_BASIC_CHANNELS = 8;
    static final int CB_OP = 0;
    static final int CB_SCB = 2;
    static final int CB_AM = 4;
    static final int CB_PU = 6;
    static final int[] CHANNEL_BONDINGS = {CB_OP, CB_SCB, CB_AM, CB_PU};
    static final int[] TRAFFIC_LOADS = {0};

    // JDBC driver name and database URL
    static final String JDBC_DRIVER = "com.mysql.jdbc.Driver";
    static final String DB_URL = "jdbc:derby://localhost:1527/paper5";
    static final String TABLE_NAME = "DATATABLE";

    //  Database credentials
    static final String USER = "root";
    static final String PASS = "root";

    static Connection conn = null;
    static Statement stmt = null;

    // Traffic load x primary x policy x metrics
    static HashMap<Integer, HashMap<Integer, HashMap<Integer, Metrics>>> metrics_container = new HashMap<Integer, HashMap<Integer, HashMap<Integer, Metrics>>>();

    public static void logs_to_database(String input_path) {

        String sql = null;
        ResultSet rs = null;
        PreparedStatement pstmt = null;

        try {

            // Open connection
            System.out.println("Connecting to database " + DB_URL + "...");
            conn = DriverManager.getConnection(DB_URL, USER, PASS);
            System.out.println("Connected to database " + DB_URL);

            System.out.println("Delenting all rows in database" + DB_URL + "...");
            stmt = conn.createStatement();
            sql = "DELETE FROM " + TABLE_NAME;
            pstmt = conn.prepareStatement(sql);
            pstmt.executeUpdate();
            System.out.println("All rows deleted!");

            String line;
            int log_ix = 0;
            System.out.println("Reading input file and generating database...");

            try (BufferedReader br = new BufferedReader(new FileReader(input_path))) {
                while ((line = br.readLine()) != null) {

                    // e.g.: KOMONDOR SIMULATION 'sim_input_nodes_n20_s100_p0_cb0_load050.csv' (seed 1992);1215;48.60;48.60;0.5769;66.89;0.7774;0.0000
                    KomondorLog log = new KomondorLog();
                    log.full_log = line;

                    String[] log_attributes = line.split(CSV_SEPARATOR);

                    // Identify attributes in the description
                    log.log_description = log_attributes[0];
                    String[] input_attributes = log.log_description.split("_");
                    log.num_nodes = Integer.parseInt(input_attributes[3].substring(1));
                    log.scenario_id = Integer.parseInt(input_attributes[4].substring(1));
                    log.primary_channel = Integer.parseInt(input_attributes[5].substring(1));
                    log.channel_bonding_model = Integer.parseInt(input_attributes[6].substring(2));
                    String[] separated_load = input_attributes[7].split("\\.");
                    log.traffic_load = Integer.parseInt(separated_load[0].substring(4));

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

                    // Execute a query for inserting new log
                    stmt = conn.createStatement();
                    sql = "INSERT INTO " + TABLE_NAME
                            + " (LOG_ID, NUM_NODES, SCENARIO_ID, PRIMARY_CHANNEL, "
                            + "POLICY_IX, TRAFFIC_LOAD, NUM_PKTS_GENERATED, "
                            + "AVERAGE_NUM_PKTS_GENERATED, THROUGHPUT, RHO, "
                            + "DELAY, UTILIZATION, DROP_RATIO) \n"
                            + "VALUES (" + log_ix + ", " + log.num_nodes + ", "
                            + log.scenario_id + ", " + log.primary_channel + ", "
                            + log.channel_bonding_model + ", " + log.traffic_load + ", "
                            + +log.metrics.num_packets_generated + ", " + log.metrics.average_num_packets_generated + ", "
                            + log.metrics.throughput + ", " + log.metrics.rho + ", "
                            + log.metrics.delay + ", " + log.metrics.utilization + ", "
                            + log.metrics.drop_ratio + ")";

                    pstmt = conn.prepareStatement(sql);
                    pstmt.executeUpdate();
                    log_ix++;
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

            stmt.close();
            conn.close();
            System.out.println("Logs successfully stored in the database!");

        } catch (SQLException se) {
            //Handle errors for JDBC
            se.printStackTrace();
        } catch (Exception e) {
            //Handle errors for Class.forName
            e.printStackTrace();
        } finally {
            //finally block used to close resources
            try {
                if (stmt != null) {
                    stmt.close();
                }
            } catch (SQLException se2) {
            }// nothing we can do
            try {
                if (conn != null) {
                    conn.close();
                }
            } catch (SQLException se) {
                se.printStackTrace();
            }//end finally try
        }//end try

    }

    public static void compute_average() {

        String sql = null;
        ResultSet rs = null;

        try {

            // Open connection
            System.out.println("Connecting to database " + DB_URL + "...");
            conn = DriverManager.getConnection(DB_URL, USER, PASS);
            System.out.println("Connected to database " + DB_URL);

            // Traffic load
            for (int l_ix = 0; l_ix < TRAFFIC_LOADS.length; l_ix++) {

                HashMap<Integer, HashMap<Integer, Metrics>> hash_prim_channel_to_policy = new HashMap<>();

                for (int prim_ix = 0; prim_ix < NUM_BASIC_CHANNELS; prim_ix++) {

                    HashMap<Integer, Metrics> hash_policy_to_metrics = new HashMap<>();

                    for (int pol_ix = 0; pol_ix < CHANNEL_BONDINGS.length; pol_ix++) {

                        Metrics average_metrics = new Metrics();

                        stmt = conn.createStatement();
                        
//                        sql = "select AVG(NUM_PKTS_GENERATED), AVG(AVERAGE_NUM_PKTS_GENERATED), "
//                                + "AVG(THROUGHPUT), AVG(RHO), AVG(DELAY), AVG(UTILIZATION), "
//                                + "AVG(DROP_RATIO) from DATATABLE"
//                                + " WHERE TRAFFIC_LOAD = " + TRAFFIC_LOADS[l_ix]
//                                + " AND PRIMARY_CHANNEL = " + prim_ix
//                                + " AND POLICY_IX = " + CHANNEL_BONDINGS[pol_ix]
//                                + " AND (THROUGHPUT > 0.95 * AVERAGE_NUM_PKTS_GENERATED)";
                        
                        sql = "select AVG(NUM_PKTS_GENERATED), AVG(AVERAGE_NUM_PKTS_GENERATED), "
                                + "AVG(THROUGHPUT), AVG(RHO), AVG(DELAY), AVG(UTILIZATION), "
                                + "AVG(DROP_RATIO) from DATATABLE"
                                + " WHERE TRAFFIC_LOAD = " + TRAFFIC_LOADS[l_ix]
                                + " AND PRIMARY_CHANNEL = " + prim_ix
                                + " AND POLICY_IX = " + CHANNEL_BONDINGS[pol_ix];
                        
                        rs = stmt.executeQuery(sql);

                        if (rs.next()) {
                            average_metrics.num_packets_generated = rs.getFloat(1);
                            average_metrics.average_num_packets_generated = rs.getFloat(2);
                            average_metrics.throughput = rs.getFloat(3);
                            average_metrics.rho = rs.getFloat(4);
                            average_metrics.delay = rs.getFloat(5);
                            average_metrics.utilization = rs.getFloat(6);
                            average_metrics.drop_ratio = rs.getFloat(7);
                        }

                        hash_policy_to_metrics.put(CHANNEL_BONDINGS[pol_ix], average_metrics);

                    }

                    hash_prim_channel_to_policy.put(prim_ix, hash_policy_to_metrics);

                }

                metrics_container.put(TRAFFIC_LOADS[l_ix], hash_prim_channel_to_policy);

            }

            stmt.close();
            conn.close();

        } catch (SQLException se) {
            //Handle errors for JDBC
            se.printStackTrace();
        } catch (Exception e) {
            //Handle errors for Class.forName
            e.printStackTrace();
        } finally {
            //finally block used to close resources
            try {
                if (stmt != null) {
                    stmt.close();
                }
            } catch (SQLException se2) {
            }// nothing we can do
            try {
                if (conn != null) {
                    conn.close();
                }
            } catch (SQLException se) {
                se.printStackTrace();
            }//end finally try
        }//end try
    }

    public static void write_average_info(String output_path) throws IOException {

        File fileTemp = new File(output_path);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;

        String csv_header_line = "traffic_load" + CSV_SEPARATOR
                + "primary" + CSV_SEPARATOR
                + "policy" + CSV_SEPARATOR
                + "num_pkts_generated" + CSV_SEPARATOR
                + "av_num_pkts_generated" + CSV_SEPARATOR
                + "throughput" + CSV_SEPARATOR
                + "rho" + CSV_SEPARATOR
                + "delay" + CSV_SEPARATOR
                + "utilization" + CSV_SEPARATOR
                + "drop_ratio" + CSV_SEPARATOR;

        out.println(csv_header_line);

        for (int l_ix = 0; l_ix < TRAFFIC_LOADS.length; l_ix++) {

            for (int prim_ix = 0; prim_ix < NUM_BASIC_CHANNELS; prim_ix++) {

                for (int pol_ix = 0; pol_ix < CHANNEL_BONDINGS.length; pol_ix++) {

                    line = TRAFFIC_LOADS[l_ix] + CSV_SEPARATOR
                            + prim_ix + CSV_SEPARATOR
                            + CHANNEL_BONDINGS[pol_ix] + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).num_packets_generated + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).average_num_packets_generated + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).throughput + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).rho + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).delay + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).utilization + CSV_SEPARATOR
                            + metrics_container.get(TRAFFIC_LOADS[l_ix]).get(prim_ix).get(CHANNEL_BONDINGS[pol_ix]).drop_ratio + CSV_SEPARATOR;
                    
                    out.println(line);

                }

            }

        }

        out.close();

    }

    public static void main(String args[]) throws IOException {

        String input_path = "script_output.txt";
        System.out.println("input_path: " + input_path);

        logs_to_database(input_path);
        compute_average();
        
        String output_path_delay = "primary_study.csv";
        write_average_info(output_path_delay);

    }//end main
}//end FirstExample
