
import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Rectangle2D;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import org.jfree.chart.*;
import org.jfree.chart.annotations.XYTextAnnotation;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/*
 * Program for plotting the nodes in a rectangular map
 */
/**
 *
 * @author Sergio Barrachina-Munoz (sergio.barrachina@upf.edu)
 */
public class Map_viewer extends JFrame {

    private static final int SIZE_X = 720;
    private static final int SIZE_Y = 600;
    private static final String title = "Komondor input Map";
    private final XYSeries series_ap = new XYSeries("AP");
    private final XYSeries series_sta = new XYSeries("STA");

    private static String path = "";

    static final String CSV_SEPARATOR = ";";

    class Node_data {

        public String node_code;
        public double x;
        public double y;
    }

    LinkedList<Node_data> node_code_container = new LinkedList<>();

    static double map_width;
    static double map_heigth;

    public Map_viewer(String title) throws IOException {
        super(title);
        final ChartPanel chartPanel = createPanel();
        this.add(chartPanel, BorderLayout.CENTER);
    }

    private ChartPanel createPanel() throws IOException {

        JFreeChart chart = ChartFactory.createScatterPlot(
                title, "X [m]", "Y [m]", readData(),
                PlotOrientation.VERTICAL, true, true, true);

        XYPlot xyPlot = (XYPlot) chart.getPlot();

        XYItemRenderer renderer = xyPlot.getRenderer();
        renderer.setSeriesPaint(0, Color.blue);
        double size = 8;
        double delta = size / 2.0;
        Shape shape1 = new Rectangle2D.Double(-delta, -delta, size, size);
        Shape shape2 = new Ellipse2D.Double(-delta, -delta, size, size);

        renderer.setSeriesShape(0, shape1);
        renderer.setSeriesPaint(0, Color.YELLOW);
        renderer.setSeriesShape(1, shape2);
        renderer.setSeriesPaint(1, Color.CYAN);

        XYTextAnnotation annotation;

        for (int n = 0; n < node_code_container.size(); n++) {

            System.out.println("n: " + n + "( " + node_code_container.get(n).x
                    + ", " + node_code_container.get(n).y + ")");

            annotation = new XYTextAnnotation(
                    node_code_container.get(n).node_code,
                    node_code_container.get(n).x,
                    node_code_container.get(n).y);

            annotation.setFont(new Font("SansSerif", 0, 6));

            xyPlot.addAnnotation(annotation);
        }

        Stroke stroke = new BasicStroke();
        xyPlot.setDomainGridlineStroke(stroke);
        xyPlot.setRangeGridlineStroke(stroke);

        return new ChartPanel(chart) {
            @Override
            public Dimension getPreferredSize() {
                return new Dimension(SIZE_X, SIZE_Y);
            }
        };
    }

    public XYDataset readData() throws FileNotFoundException, IOException {

        XYSeriesCollection xySeriesCollection = new XYSeriesCollection();

        if (path.equals("")) {
            // Get file path
            JFileChooser fileChooser = new JFileChooser("user.home\\Desktop");
            int result = fileChooser.showOpenDialog(this);
            // If file selected and openned
            if (result == JFileChooser.APPROVE_OPTION) {
                path = "" + fileChooser.getSelectedFile();
            }
        }

        String line;
        System.out.println("Reading input file...");

        boolean first_line_skipped = false;
        String node_code;
        int node_type;
        double x, y;
        Node_data node_data;

        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            while ((line = br.readLine()) != null) {
                if (!first_line_skipped) {
                    // It is a comment --> do nothing
                    first_line_skipped = true;
                } else {

                    String[] node_info = line.split(CSV_SEPARATOR);

                    node_code = node_info[0];
                    node_type = Integer.parseInt(node_info[1]);
                    x = Double.parseDouble(node_info[4]);
                    y = Double.parseDouble(node_info[5]);

                    if (map_width < x / 2) {
                        map_width = x * 2;
                    }

                    if (map_heigth < y / 2) {
                        map_heigth = y * 2;
                    }

                    node_data = new Node_data();
                    node_data.node_code = node_code;
                    node_data.x = x;
                    node_data.y = y;

                    node_code_container.add(node_data);

                    if (node_type == 0) {
                        series_ap.add(x, y);
                    } else {
                        series_sta.add(x, y);
                    }

                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        xySeriesCollection.addSeries(series_ap);
        xySeriesCollection.addSeries(series_sta);

        return xySeriesCollection;

    }

    public static void main(String args[]) {

        if (args.length > 0) {
            path = args[0];
        }

        EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                Map_viewer demo = null;
                try {
                    demo = new Map_viewer(title);
                } catch (IOException ex) {
                    Logger.getLogger(Map_viewer.class.getName()).log(Level.SEVERE, null, ex);
                }
                demo.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
                demo.pack();
                demo.setLocationRelativeTo(null);
                demo.setVisible(true);
            }
        });
    }
}
