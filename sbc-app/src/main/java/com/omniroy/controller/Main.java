package com.omniroy.controller;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Locale;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;

public final class Main {
    private Main() {
    }

    public static void main(String[] args) {
        if (GraphicsEnvironment.isHeadless()) {
            throw new IllegalStateException("WASD control requires a graphical desktop session");
        }

        // SerialPortTransmit.open(); // Enable when testing with the robot connected.

        SwingUtilities.invokeLater(Main::showController);
    }

    private static void showController() {
        JFrame frame = new JFrame("OmniRoy WASD Controller");
        JPanel panel = new JPanel(new BorderLayout());
        panel.setFocusable(true);
        panel.setPreferredSize(new Dimension(420, 180));
        panel.add(new JLabel(
                "<html><center>W / S: drive forward / backward<br>"
                        + "A / D: strafe left / right<br>"
                        + "Release keys to stop</center></html>",
                SwingConstants.CENTER), BorderLayout.CENTER);

        WasdController controller = new WasdController(command -> {
            System.out.printf(Locale.ROOT, "Command: %.2f,%.2f%n",
                    (float) command.angle(), command.speed());
            // SerialPortTransmit.write(command.angle(), command.speed()); // Enable when testing with the robot connected.
        });
        panel.addKeyListener(controller);
        panel.addFocusListener(controller);

        frame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent event) {
                controller.stop();
                // closeSerialPort(frame); // Enable together with SerialPortTransmit.open().
                frame.dispose();
            }
        });
        frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        frame.add(panel);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
        panel.requestFocusInWindow();
    }

}
