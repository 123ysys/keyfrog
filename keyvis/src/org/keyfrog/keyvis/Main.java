/*
 * Main.java
 *
 * Created on 15 luty 2007, 20:02
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.keyfrog.keyvis;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Minute;
import org.jfree.data.time.Day;
import org.jfree.data.time.Month;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.XYDataset;
import org.jfree.ui.RectangleInsets;
import org.jfree.ui.RefineryUtilities;

import java.sql.*;
import org.sqlite.JDBC;

import java.awt.Color;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.net.URL;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Node;
import org.dom4j.Attribute;
import org.dom4j.io.SAXReader;

/**
 *
 * @author Sebastian Gniazdowski
 */
public class Main {
    Hashtable<Integer, String> m_groupIdToName;
    String userHome;
    
    /**
     * After this function we will have:
     * - m_groupIdToName map filled with id -> groupName
     * - 
     */
    private boolean readConfig() {
        Document document;
        try {
            URL url = new URL("file://" + userHome + "/.keyfrog/config");
            SAXReader reader = new SAXReader();
            document = reader.read(url);
        } catch(Exception e) {
            System.err.println("Bad configuration file path");
            return false;
        }
        prepareGroupNames();
                        
        List<Node> list = document.selectNodes("/keyfrog/application-groups/group");
        for(Iterator<Node> n_it = list.iterator(); n_it.hasNext();) {
            Node node = n_it.next();
            String name = node.valueOf("@name");
            Number id = node.numberValueOf("@id");
            if(name == null || id == null || name.equals("") || id.toString().equals("NaN")) {
                System.err.println("Malformed configuration file (group tag has inproper id or name attribute)");
                return false;
            }
            
            // Finally insert new id -> name mapping into hashtable
            m_groupIdToName.put(id.intValue(), name);
            System.out.println("Added: "+ id + "->" + name );
        }

        return true;
    }

    private void prepareGroupNames() {
        m_groupIdToName = new Hashtable<Integer, String>();        
    }
    
    /** Creates a new instance of Main */
    public Main() {
        userHome = System.getProperty("user.home");
        if(userHome == null) {
            System.err.println("Cant find home directory");
            System.exit(1);
        }
        if(!readConfig()) {
            // FIXME msg
            System.exit(1);
        }

        // Create main window and draw empty chart
        KeyVisWin win = new KeyVisWin();        
        LineChartFactory fac = new LineChartFactory();
        fac.setGroupIdToName(m_groupIdToName);
        win.setLineChartFactory(fac);
        win.drawEmptyChart();
        win.setDatabasePath(userHome+"/.keyfrog/keyfrog.db");
        win.setVisible(true);
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        Main m = new Main();
    }    
}
