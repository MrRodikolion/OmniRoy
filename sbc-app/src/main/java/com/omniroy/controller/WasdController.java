package com.omniroy.controller;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;
import java.util.function.Consumer;

public final class WasdController implements KeyListener, FocusListener {
    public static final float DRIVE_SPEED = 8.0f;

    private final Consumer<DriveCommand> commandSink;
    private final Set<Character> pressedKeys = new HashSet<>();

    public WasdController(Consumer<DriveCommand> commandSink) {
        this.commandSink = Objects.requireNonNull(commandSink, "commandSink");
    }

    @Override
    public void keyPressed(KeyEvent event) {
        char key = movementKey(event.getKeyCode());
        if (isMovementKey(key) && pressedKeys.add(key)) {
            event.consume();
            publish();
        }
    }

    @Override
    public void keyReleased(KeyEvent event) {
        char key = movementKey(event.getKeyCode());
        if (pressedKeys.remove(key)) {
            event.consume();
            publish();
        }
    }

    @Override
    public void keyTyped(KeyEvent event) {
    }

    @Override
    public void focusGained(FocusEvent event) {
    }

    @Override
    public void focusLost(FocusEvent event) {
        stop();
    }

    public void stop() {
        pressedKeys.clear();
        publish();
    }

    private void publish() {
        int horizontal = (pressedKeys.contains('d') ? 1 : 0) - (pressedKeys.contains('a') ? 1 : 0);
        int vertical = (pressedKeys.contains('w') ? 1 : 0) - (pressedKeys.contains('s') ? 1 : 0);
        if (horizontal == 0 && vertical == 0) {
            commandSink.accept(new DriveCommand(90, 0.0f));
            return;
        }

        int angle = (int) Math.round(Math.toDegrees(Math.atan2(vertical, horizontal)));
        if (angle < 0) {
            angle += 360;
        }
        commandSink.accept(new DriveCommand(angle, DRIVE_SPEED));
    }

    private boolean isMovementKey(char key) {
        return key == 'w' || key == 'a' || key == 's' || key == 'd';
    }

    private char movementKey(int keyCode) {
        return switch (keyCode) {
            case KeyEvent.VK_W -> 'w';
            case KeyEvent.VK_A -> 'a';
            case KeyEvent.VK_S -> 's';
            case KeyEvent.VK_D -> 'd';
            default -> '\0';
        };
    }

    public record DriveCommand(int angle, float speed) {
    }
}
