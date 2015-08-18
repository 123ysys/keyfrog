/*
 * LineChartFactory.java
 *
 * Created on 23 luty 2007, 18:46
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.keyfrog.keyvis;

// JFreeChart packages
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
import java.awt.BasicStroke;
import java.awt.Stroke;

// Sqlite packages
import java.sql.*;
import org.sqlite.JDBC;

import java.awt.Color;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 *
 * @author Sebastian Gniazdowski
 */
public class LineChartFactory {
    public static final int SIMPLE = 1;
    public static final int ACCUMULATE = 2;
    private int m_chartType = SIMPLE;
    
    public static final long HOUR = 60L*60L*1000L;
    public static final long DAY = 60L*60L*24L*1000L;
    public static final long WEEK = DAY*7L;
    public static final long MONTH = DAY*30L;
    private long m_accumulationRange = DAY;

    private int m_beginTimestamp;
    private int m_endTimestamp;

    private String m_chartTitle;

    private String m_databasePath;

    // Maps group id's to human readable names
    private Hashtable<Integer, String> m_groupIdToName;
    
    /**
     * Creates a new instance of LineChartFactory 
     */
    public LineChartFactory() {
        setupChartTitle();
    }
    
    /**
     * Creates a new instance of LineChartFactory 
     */
    public LineChartFactory(int chartType) {
        setChartType(chartType);
        setupChartTitle();
    }
    
    public JFreeChart produceChart() {
        XYDataset dataset = createDataset();
        JFreeChart chart = createChart(dataset);
        return chart;
    }
    
    /**
     * Setups chart title basing on chart type and other properties
     */
    private void setupChartTitle() {
        if(m_chartType == SIMPLE) {
            m_chartTitle = "Keyboard usage (full resolution)";
        } else if(m_chartType == ACCUMULATE) {
            // FIXME
            if(m_accumulationRange==DAY)
                m_chartTitle = "Keyboard usage (per day)";
            else if(m_accumulationRange==WEEK)
                m_chartTitle = "Keyboard usage (per week)";
            else if(m_accumulationRange==MONTH)
                m_chartTitle = "Keyboard usage (per month)";
            else
                m_chartTitle = "Keyboard usage";                    
        }        
    }
    
    /**
     * Sets chart type
     */
    public void setChartType(int chartType) {
        m_chartType = chartType;
    }
    
    /**
     * Sets range from which average should be calculated in
     * case of ACCUMULATE chart type.
     */
    public void setAccumulationRange(long miliseconds) {
        m_accumulationRange = (long)miliseconds;
        setChartType(ACCUMULATE);
    }

    /**
     * Sets path to SQLite database
     */
    public void setDatabasePath(String databasePath) {
        m_databasePath = databasePath;
    }
    
    /**
     * Sets hashtable that maps grup id to group human readable name
     */
    public void setGroupIdToName(Hashtable<Integer, String> groupIdToName) {
        m_groupIdToName = groupIdToName;
    }
    
    /**
     * Sets time range from which chart should be created
     */
    public void setDataRange(int beginTimestamp, int endTimestamp) {
        m_beginTimestamp = beginTimestamp;
        m_endTimestamp = endTimestamp;        
    }
    
    /**
     * Creates chart basing on dataset
     */     
    public JFreeChart createChart(XYDataset dataset) {
        setupChartTitle();
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
                m_chartTitle,  // title
                "Date",             // x-axis label
                "Keystrokes",   // y-axis label
                dataset,            // data
                true,               // create legend?
                true,               // generate tooltips?
                false               // generate URLs?
                );
        
        chart.setBackgroundPaint(Color.white);

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(Color.lightGray);
        plot.setDomainGridlinePaint(Color.white);
        plot.setRangeGridlinePaint(Color.white);
        plot.setAxisOffset(new RectangleInsets(5.0, 5.0, 5.0, 5.0));
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);
        plot.getRenderer().setSeriesPaint(6, Color.BLACK); 
        //plot.getRenderer().setBaseStroke(new BasicStroke(4f));
        
        XYItemRenderer r = plot.getRenderer();
        if (r instanceof XYLineAndShapeRenderer) {
            XYLineAndShapeRenderer renderer = (XYLineAndShapeRenderer) r;
            renderer.setBaseShapesVisible(true);
            renderer.setBaseShapesFilled(true);
        }
        DateAxis axis = (DateAxis) plot.getDomainAxis();
        //axis.setDateFormatOverride(new SimpleDateFormat("dd-MMM"));  
        axis.setDateFormatOverride(new SimpleDateFormat("dd-MM"));        
        return chart;
    }
    
    private XYDataset createDataset() {
        if(m_databasePath == null) {
            // FIXME
            System.err.println("No database path given to LineChartFactory");
            System.exit(3);
        }
        if(m_groupIdToName == null) {
            System.err.println("No groupIdToName set");
            System.exit(4);
        }
        XYDataset dataset;
        switch(m_chartType) {
            case SIMPLE:
                dataset = createSimpleDataset();
                break;
            case ACCUMULATE:
                dataset = createAverageDataset();
                break;
            default:
                dataset = new TimeSeriesCollection();
        }
        return dataset;
    }
    /**
     * Reads database, creates JFreeChart dataset
     */
    private XYDataset createSimpleDataset() {
        // Can this approach be optimized?
        Hashtable<Integer, TimeSeries> timeSeriesTable = new Hashtable<Integer, TimeSeries>();
        try {   
            // FIXME
            String fileName = m_databasePath;
            Class.forName("org.sqlite.JDBC");
            
            Connection conn = DriverManager.getConnection("jdbc:sqlite:"+fileName);
            
            Statement stmt = conn.createStatement();
            
            // Create a result set object for the statement
            String query = "SELECT cluster_begin, cluster_end, count, app_group " +
                    "FROM keypresses WHERE cluster_begin >= '" + m_beginTimestamp + "' AND " + 
                    "cluster_begin <= '" + m_endTimestamp + "' ORDER BY cluster_begin, app_group";
            System.err.println(query);
            ResultSet rs = stmt.executeQuery(query);
            
            while (rs.next()) {
                long cbeg = rs.getTimestamp(1).getTime()*1000L;
                long cend = rs.getTimestamp(2).getTime()*1000L;
                long cavg = cbeg / 2 + cend / 2;
                int count = rs.getInt(3);
                int appg = rs.getInt(4);
                if(!timeSeriesTable.containsKey(appg)) {
                    if(!m_groupIdToName.containsKey(appg)) {
                        m_groupIdToName.put(appg,"Unknown");
                    }
                    String gname = m_groupIdToName.get(appg);                    
                    timeSeriesTable.put(appg, new TimeSeries(gname, Day.class));
                    //System.out.println("Timeseries for " + gname + " created");
                }             
                TimeSeries timeSeries = timeSeriesTable.get(appg);                
                timeSeries.addOrUpdate(new Minute(new java.util.Date(cavg)), count);               
            }
            
            // Close the connection
            conn.close();            
        }
        catch (Exception e) {
            // Print some generic debug info
            System.out.println(e.getMessage());
            System.out.println(e.toString());
            System.exit(2);
        }
        TimeSeriesCollection dataset = new TimeSeriesCollection();
        for(Enumeration<TimeSeries> e = timeSeriesTable.elements(); e.hasMoreElements();) {
            dataset.addSeries(e.nextElement());
        }
        
        return dataset;
    }

    /**
     * Reads database, creates JFreeChart dataset
     * FIXME: Optimization needed
     */
    private XYDataset createAverageDataset() {
        // Can this approach be optimized?
        Hashtable<Integer, TimeSeries> timeSeriesTable = new Hashtable<Integer, TimeSeries>();
        Hashtable<Integer, Integer> keystrokesMap = new Hashtable<Integer, Integer>();
        TimeSeries allGroupsSeries = new TimeSeries("all", Day.class);
        int allGroupsKeystrokes = 0;

        try {   
            // FIXME
            String fileName = m_databasePath;
            Class.forName("org.sqlite.JDBC");
            
            Connection conn = DriverManager.getConnection("jdbc:sqlite:"+fileName);
            
            Statement stmt = conn.createStatement();
            
            // Create a result set object for the statement
            String query = "SELECT cluster_begin, cluster_end, count, app_group " +
                    "FROM keypresses WHERE cluster_begin >= '" + m_beginTimestamp + "' AND " + 
                    "cluster_begin <= '" + m_endTimestamp + "' ORDER BY cluster_begin, app_group";
            System.err.println(query);
            ResultSet rs = stmt.executeQuery(query);
            
            if(rs.next()) {
                // We got first record
                // Calculate accumulation period first bound
                long rangeEndCursor = rs.getTimestamp(1).getTime()*1000L;
                // Begin
                rangeEndCursor = rangeEndCursor - (rangeEndCursor % m_accumulationRange);
                // End
                rangeEndCursor += m_accumulationRange;

                // Sum accumulator
                boolean unprocessedData;
                do {
                    unprocessedData = true;

                    long cbeg = rs.getTimestamp(1).getTime()*1000L;
                    long cend = rs.getTimestamp(2).getTime()*1000L;
                    long cavg = cbeg / 2 + cend / 2;
                    int count = rs.getInt(3);
                    int appg = rs.getInt(4);
                    
                    // Add entry to group if it's meet for the first time
                    if(!timeSeriesTable.containsKey(appg)) {
                        if(!m_groupIdToName.containsKey(appg)) {
                            m_groupIdToName.put(appg,"Unknown");
                        }
                        String gname = m_groupIdToName.get(appg);
                        timeSeriesTable.put(appg, new TimeSeries(gname, Day.class));
                        
                        // Also prepare keystrokes map entry for that group
                        keystrokesMap.put(appg, 0);
                    }
                    Integer groupKeystrokes = keystrokesMap.get(appg);
                    groupKeystrokes += count;
                    keystrokesMap.put(appg, groupKeystrokes);
                    
                    allGroupsKeystrokes += count;

                    // Before new accumulation period is started
                    // add already collected data to chart
                    if(cavg >= rangeEndCursor) {
                        addAccumulatedTimeSeries(timeSeriesTable, keystrokesMap, rangeEndCursor);
                        allGroupsSeries.addOrUpdate(new Minute(new java.util.Date(rangeEndCursor - m_accumulationRange/2)), allGroupsKeystrokes);
                        allGroupsKeystrokes = 0;
                        rangeEndCursor += m_accumulationRange;
                        unprocessedData = false;

                    }
                } while (rs.next());
                if(unprocessedData) {
                    addAccumulatedTimeSeries(timeSeriesTable, keystrokesMap, rangeEndCursor);
                    allGroupsSeries.addOrUpdate(new Minute(new java.util.Date(rangeEndCursor - m_accumulationRange/2)), allGroupsKeystrokes);
                }
            }
            // Close the connection
            conn.close();            
        }
        catch (Exception e) {
            // Print some generic debug info
            System.out.println(e.getMessage());
            System.out.println(e.toString());
            System.exit(2);
        }
        TimeSeriesCollection dataset = new TimeSeriesCollection();
        for(Enumeration<TimeSeries> e = timeSeriesTable.elements(); e.hasMoreElements();) {
            dataset.addSeries(e.nextElement());
        }
        dataset.addSeries(allGroupsSeries);
        
        return dataset;
    }
    
    private void addAccumulatedTimeSeries(
            Hashtable<Integer, TimeSeries> timeSeriesTable,
            Hashtable<Integer, Integer> keystrokesMap,
            long rangeEndCursor
            ) {
            //System.err.println("Beyond range: " + rangeEndCursor + " = " + new java.util.Date(rangeEndCursor));

            for (Map.Entry<Integer, Integer> e : keystrokesMap.entrySet()) {
                int group = e.getKey();
                int keystrokes = e.getValue();
                e.setValue(0);
                
                TimeSeries timeSeries = timeSeriesTable.get(group);
                // Dirty - FIXME :\
                //timeSeries.addOrUpdate(new Minute(new java.util.Date(rangeEndCursor-m_accumulationRange+1)),
                //        keystrokes);
                //timeSeries.addOrUpdate(new Minute(new java.util.Date(rangeEndCursor-1)), keystrokes);
                timeSeries.addOrUpdate(new Minute(new java.util.Date(rangeEndCursor - m_accumulationRange/2)), keystrokes);
            }
            
    }
}
