package com.omniroy.controller;

import com.fazecast.jSerialComm.SerialPort;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.Properties;

public final class SerialPortTransmit {
    private static final Properties SETTINGS = loadSettings();
    private static SerialPort serialPort;

    private SerialPortTransmit() {
    }

    public static void open() throws IOException {
        String portName = System.getProperty(
                "serial.port",
                SETTINGS.getProperty("serial.port", "/dev/ttyUSB0")).trim();
        String baudRateValue = System.getProperty(
                "serial.baud-rate",
                SETTINGS.getProperty("serial.baud-rate", "115200")).trim();

        if (portName.isEmpty()) {
            throw new IOException("Serial port name must not be empty");
        }

        int baudRate;
        try {
            baudRate = Integer.parseInt(baudRateValue);
        } catch (NumberFormatException exception) {
            throw new IOException("Invalid serial baud rate: " + baudRateValue, exception);
        }

        SerialPort candidate = SerialPort.getCommPort(portName);
        candidate.setComPortParameters(baudRate, 8, SerialPort.ONE_STOP_BIT, SerialPort.NO_PARITY);
        candidate.setComPortTimeouts(SerialPort.TIMEOUT_WRITE_BLOCKING, 0, 0);
        if (!candidate.openPort()) {
            throw new IOException("Failed to open serial port " + portName);
        }
        serialPort = candidate;
        System.out.println("Opened serial port " + portName + " at " + baudRate + " baud");
    }

    public static void write(float angle, float speed) throws IOException {
        String command = String.format(Locale.ROOT, "%.2f,%.2f%n", angle, speed);
        System.out.print("Command: " + command);
        if (serialPort == null || !serialPort.isOpen()) {
            return;
        }

        byte[] bytes = command.getBytes(StandardCharsets.US_ASCII);
        int bytesWritten = serialPort.writeBytes(bytes, bytes.length);
        if (bytesWritten != bytes.length) {
            throw new IOException("Incomplete serial write: " + bytesWritten + " of " + bytes.length + " bytes");
        }
    }

    public static void close() throws IOException {
        if (serialPort == null) {
            return;
        }
        if (serialPort.isOpen() && !serialPort.closePort()) {
            throw new IOException("Failed to close serial port " + serialPort.getSystemPortName());
        }
        serialPort = null;
    }

    private static Properties loadSettings() {
        Properties properties = new Properties();
        try (InputStream input = SerialPortTransmit.class.getClassLoader()
                .getResourceAsStream("application.properties")) {
            if (input != null) {
                properties.load(input);
            }
        } catch (IOException exception) {
            throw new ExceptionInInitializerError(exception);
        }
        return properties;
    }
}
