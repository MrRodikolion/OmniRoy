package com.omniroy.controller;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;

import org.junit.jupiter.api.Test;

class MainTest {
    @Test
    void applicationStarts() {
        assertDoesNotThrow(() -> Main.main(new String[0]));
    }
}