
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author UPF
 */
public class AddNaiveColumn {

    public static void addColumn(String path, String fileName) throws IOException {
        BufferedReader br = null;
        BufferedWriter bw = null;
        final String lineSep = System.getProperty("line.separator");

        try {
            File file = new File(path);
            File file2 = new File(fileName);//so the
            //names don't conflict or just use different folders

            br = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
            bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file2)));
            String line = null;
            int i = 0;
            for (line = br.readLine(); line != null; line = br.readLine(), i++) {
                String addedColumn = ";aux";
                bw.write(line + addedColumn + lineSep);
            }

        } catch (Exception e) {
            System.out.println(e);
        } finally {
            if (br != null) {
                br.close();
            }
            if (bw != null) {
                bw.close();
            }
        }
    }

    public static void main(String args[]) throws IOException {

        System.out.println("FINDING FILES TO MODIFY...");
        // your directory
        File f = new File("/home/dtic/Documents/Komondor/Apps/Input Generation/files_to_append");
        File[] matchingFiles = f.listFiles(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                // return name.startsWith("input_nodes_n") && name.endsWith("4.csv");
                return name.startsWith("input_nodes");
            }
        });
        
        System.out.println("THERE ARE " + matchingFiles.length + " FILES TO MODIFY:");
        // Files to modify
        for (int i = 0; i < matchingFiles.length; i++) {
             System.out.println("  " + i + ": " + matchingFiles[i].getName());
        }
        
        String output_path_directory = 
                "/home/dtic/Documents/Komondor/Apps/Input Generation/appended_files";
        System.out.println("DELETING OLD APPENDED FILES...");
        // Delete files
        File f_delete = new File(output_path_directory);
        File[] delFiles = f_delete.listFiles(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                return (name.startsWith("input_nodes") && !name.endsWith("csv2.csv"));
            }
        });

        for (int i = 0; i < delFiles.length; i++) {
             delFiles[i].delete();
        }
        System.out.println("OLD APPENDED FILES DELETED!");
        
       
                
        System.out.println("ADDING NAIVE COLUMN TO FILES...");
        for (int i = 0; i < matchingFiles.length; i++) {
            addColumn(matchingFiles[i].getAbsolutePath(), output_path_directory + "\\" + matchingFiles[i].getName() + "2.csv");
            System.out.println("  " + i + ": " + matchingFiles[i].getName());
        }
        
        System.out.println("COLUMNS ADDED!");
        
        

    }

}
