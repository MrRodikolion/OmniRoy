package com.omniroy.controller;
import java.io.IOException;
import com.fazecast.jSerialComm.*;

public class SerialPortTransmit {
	private static SerialPort sp;	
	public static void open() throws IOException, InterruptedException {
		SerialPort sp = SerialPort.getCommPort("COM9");
		// default connection settings for Arduino
		sp.setComPortParameters(9600, 8, 1, 0);

		// block until bytes can be written
		sp.setComPortTimeouts(SerialPort.TIMEOUT_WRITE_BLOCKING, 0, 0);

		if (sp.openPort()) {
			System.out.println("Port is open :)");
		} else {
			System.out.println("Failed to open port :(");
			return;
		}
	}


	public static void write(float speed, float angle) throws IOException, InterruptedException {
		while (true)	 {
			String data = speed + "," + angle + "\n";
			byte[] writeBuffer = data.getBytes();
			sp.writeBytes(writeBuffer, writeBuffer.length);
			Thread.sleep(1000); // wait for 1 second before sending the next data
		}	
	}	
	public static void close() throws IOException, InterruptedException {
		if (sp.closePort()) {
			System.out.println("Port is closed :)");
		} else {
			System.out.println("Failed to close port :(");
			return;
		}	
	}
}