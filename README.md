# OmniRoy

OmniRoy is a four-wheel mecanum robot project. Firmware for the Arduino Nano
and the Java application intended for the onboard single-board computer are
kept as separate buildable projects.

## Repository layout

```text
firmware/       Arduino Nano firmware and PlatformIO configuration
sbc-app/        Java application for the Linux single-board computer
docs/           Architecture, kinematics, communication protocol, references
scripts/        Deployment and serial-monitor helpers
```

The root `.vscode/` folder contains repository-level VS Code recommendations.
PlatformIO-specific generated files and build output belong under `firmware/`.

## Build and test

Build the firmware with PlatformIO Core:

```sh
pio run --project-dir firmware
```

Run the Java tests and package the application with Maven:

```sh
mvn -f sbc-app/pom.xml test package
```

The executable JAR is written to `sbc-app/target/omniroy-sbc-controller.jar`.
The project targets Java 17. On the SBC, use a runtime compatible with its CPU
architecture and Linux distribution.

## Hardware communication

The current Nano firmware accepts one UART line at `115200` baud in
`angle,speed` format. See [docs/api-protocol.md](docs/api-protocol.md) before
changing either side of the interface.

## Deployment helpers

Deploy the JAR over SSH with `SBC_TARGET` set to `user@host`:

```sh
SBC_TARGET=user@robot-sbc bash scripts/deploy-to-sbc.sh
```

Read a Linux serial device at the firmware baud rate:

```sh
bash scripts/monitor-serial.sh /dev/ttyUSB0
```

The deployment script requires SSH and rsync. The serial monitor requires
permission to access the selected device.

The optional `scripts/joystick_controller.py` is a desktop Python controller
that uses pygame and pyserial. Its serial port is currently set to `COM8` in
the script.