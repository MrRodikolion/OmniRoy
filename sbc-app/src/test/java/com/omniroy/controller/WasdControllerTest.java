package com.omniroy.controller;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.awt.Canvas;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import org.junit.jupiter.api.Test;

class WasdControllerTest {
    private final Canvas source = new Canvas();

    @Test
    void mapsWasdToForwardBackwardAndLateralMovementAtConstantSpeed() {
        List<WasdController.DriveCommand> commands = new ArrayList<>();
        WasdController controller = new WasdController(commands::add);

        press(controller, KeyEvent.VK_W, 'w');
        release(controller, KeyEvent.VK_W, 'w');
        press(controller, KeyEvent.VK_S, 's');
        release(controller, KeyEvent.VK_S, 's');
        press(controller, KeyEvent.VK_A, 'a');
        release(controller, KeyEvent.VK_A, 'a');
        press(controller, KeyEvent.VK_D, 'd');
        release(controller, KeyEvent.VK_D, 'd');

        assertCommand(commands.get(0), 90, WasdController.DRIVE_SPEED);
        assertCommand(commands.get(1), 90, 0.0f);
        assertCommand(commands.get(2), 270, WasdController.DRIVE_SPEED);
        assertCommand(commands.get(3), 90, 0.0f);
        assertCommand(commands.get(4), 180, WasdController.DRIVE_SPEED);
        assertCommand(commands.get(5), 90, 0.0f);
        assertCommand(commands.get(6), 0, WasdController.DRIVE_SPEED);
        assertCommand(commands.get(7), 90, 0.0f);
    }

    @Test
    void combinesForwardAndLateralKeysForDiagonalMovement() {
        List<WasdController.DriveCommand> commands = new ArrayList<>();
        WasdController controller = new WasdController(commands::add);

        press(controller, KeyEvent.VK_W, 'w');
        press(controller, KeyEvent.VK_D, 'd');
        press(controller, KeyEvent.VK_S, 's');
        release(controller, KeyEvent.VK_S, 's');

        assertCommand(commands.get(1), 45, WasdController.DRIVE_SPEED);
        assertCommand(commands.get(3), 45, WasdController.DRIVE_SPEED);
    }

    @Test
    void stopsWhenMovementKeysAreReleasedOrFocusIsLost() {
        List<WasdController.DriveCommand> commands = new ArrayList<>();
        WasdController controller = new WasdController(commands::add);

        press(controller, KeyEvent.VK_W, 'w');
        controller.focusLost(new FocusEvent(source, FocusEvent.FOCUS_LOST));

        assertCommand(commands.get(1), 90, 0.0f);
    }

    @Test
    void printsCommandWithoutAnOpenSerialPort() throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        PrintStream originalOut = System.out;

        try (PrintStream capturedOut = new PrintStream(output, true, StandardCharsets.UTF_8)) {
            System.setOut(capturedOut);
            SerialPortTransmit.write(95.0f, WasdController.DRIVE_SPEED);
        } finally {
            System.setOut(originalOut);
        }

        assertEquals("Command: 95.00,8.00" + System.lineSeparator(),
                output.toString(StandardCharsets.UTF_8));
    }

    private void press(WasdController controller, int keyCode, char keyChar) {
        controller.keyPressed(new KeyEvent(source, KeyEvent.KEY_PRESSED, 1L, 0, keyCode, keyChar));
    }

    private void release(WasdController controller, int keyCode, char keyChar) {
        controller.keyReleased(new KeyEvent(source, KeyEvent.KEY_RELEASED, 2L, 0, keyCode, keyChar));
    }

    private void assertCommand(WasdController.DriveCommand actual, int expectedAngle, float expectedSpeed) {
        assertEquals(expectedAngle, actual.angle());
        assertEquals(expectedSpeed, actual.speed(), 0.01f);
    }
}
