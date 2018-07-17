/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author UPF
 */
public class Metrics {

    double num_packets_generated;
    double average_num_packets_generated;
    double throughput;
    double rho;
    double delay;
    double utilization;
    double drop_ratio;

    public void Metrics() {

        this.num_packets_generated = 0;
        this.average_num_packets_generated = 0;
        this.throughput = 0;
        this.rho = 0;
        this.delay = 0;
        this.utilization = 0;
        this.drop_ratio = 0;

    }

    public void Metrics(double num_packets_generated, double average_num_packets_generated,
            double throughput, double rho, double delay, double utilization, double drop_ratio) {

        this.num_packets_generated = num_packets_generated;
        this.average_num_packets_generated = average_num_packets_generated;
        this.throughput = throughput;
        this.rho = rho;
        this.delay = delay;
        this.utilization = utilization;
        this.drop_ratio = drop_ratio;
    }

    public static Metrics sum_metrics(Metrics metrics1, Metrics metrics2) {

        Metrics sum_metrics = new Metrics();

        sum_metrics.num_packets_generated = metrics1.num_packets_generated + metrics2.num_packets_generated;
        sum_metrics.throughput = metrics1.throughput + metrics2.throughput;
        sum_metrics.rho = metrics1.rho + metrics2.rho;
        sum_metrics.delay = metrics1.delay + metrics2.delay;
        sum_metrics.utilization = metrics1.utilization + metrics2.utilization;
        sum_metrics.drop_ratio = metrics1.drop_ratio + metrics2.drop_ratio;

        return sum_metrics;

    }

    public static Metrics get_average_metrics(Metrics metrics,
            int averaging_factor, int delay_averaging_factor) {

        Metrics average_metrics = new Metrics();

        average_metrics.num_packets_generated = metrics.num_packets_generated / averaging_factor;
        average_metrics.throughput = metrics.throughput / averaging_factor;
        average_metrics.rho = metrics.rho / averaging_factor;
        average_metrics.delay = metrics.delay / delay_averaging_factor;
        average_metrics.utilization = metrics.utilization / averaging_factor;
        average_metrics.drop_ratio = metrics.drop_ratio / averaging_factor;

        return average_metrics;

    }

    public static void print_metrics(Metrics metrics) {

        System.out.println("******** METRICS ********");
        System.out.println(" + num_packets_generated = " + metrics.num_packets_generated);
        System.out.println(" + throughput = " + metrics.throughput);
        System.out.println(" + rho = " + metrics.rho);
        System.out.println(" + delay = " + metrics.delay);
        System.out.println(" + utilization = " + metrics.utilization);
        System.out.println(" + drop_ratio = " + metrics.drop_ratio);

    }

}
