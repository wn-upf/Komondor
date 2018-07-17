
import java.awt.Color;
import java.awt.Point;
import java.awt.geom.Ellipse2D;
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author Sergio.Barrachina
 */

/*
 * Class packing node shape (ellipse) and its id
 */
public class Node {

    Ellipse2D shape;        // All nodes are painted as circles
    Color background_color; // Depends on node type
    Color color = Color.BLUE;

    String node_code;
    int node_type;                  // Node type 0: GW, 1: STA
    String wlan_code;                  // Parent ID

    Point position;

    public Node() {
        // void constructor
    }

    public Node(String node_code, String wlan_code, int node_type,
            Point position) {

        // Items read by CSV
        this.node_code = node_code;
        this.wlan_code = wlan_code;
        this.node_type = node_type;
        if (this.node_type == 1) {
            color = Color.CYAN;
        }
        this.position = new Point();
        this.shape = new Ellipse2D.Double(0, 0, 0, 0);
    }

    public void setPosition(Point p) {
        this.position = p;
    }

    public Point getPosition() {
        return this.position;
    }

    public void setShape(Ellipse2D shape) {
        this.shape = shape;
    }

    public String nodeToString() {

        return ("- node_code: " + node_code
                + "\n- type: " + node_type
                + "\n- wlan_code: " + wlan_code
                + "\n- position: (" + position.getX() + ", " + position.getY() + ")");
    }

}
