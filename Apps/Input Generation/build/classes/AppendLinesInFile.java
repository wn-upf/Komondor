
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template origin_file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author UPF
 */
public class AppendLinesInFile {

    static PrintWriter out;

    public static void appendFileText(String origin_path) throws IOException {

        BufferedReader br = null;

        try {

            File origin_file = new File(origin_path);
            br = new BufferedReader(new InputStreamReader(new FileInputStream(origin_file)));

            String line = null;
            int i = 0;
            for (line = br.readLine(); line != null; line = br.readLine(), i++) {
                System.out.println(line);
                out.println(line);
            }

        } catch (Exception e) {
            System.out.println(e);
        } finally {
            if (br != null) {
                br.close();
            }
        }
    }

    public static void main(String args[]) throws IOException {

        String output_path = "appended_file.csv";

        File output_file = new File(output_path);
        output_file.delete();

        output_file = new File(output_path);

        // your directory
        File f = new File("C:\\Users\\UPF\\Google Drive\\Academia\\PhD"
                + "\\Workspace\\Komondor_private\\Komondor\\Apps"
                + "\\Input Generation\\files_to_append");

        File[] matchingFiles = f.listFiles();

        for (int i = 0; i < matchingFiles.length; i++) {
            System.out.println(matchingFiles[i].getName());
        }

        FileWriter fw = new FileWriter(output_path, true);
        BufferedWriter bw = new BufferedWriter(fw);
        out = new PrintWriter(bw);
        
        for (int i = 0; i < matchingFiles.length; i++) {
            appendFileText(matchingFiles[i].getAbsolutePath());
        }

        out.close();
        
    }
}
