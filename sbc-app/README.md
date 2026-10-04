# OmniRoy SBC Controller

Java application for the future onboard controller running on a Linux single-board computer.
This module is independent from the Arduino Nano firmware in the parent project.

## Requirements

- JDK 17 or newer
- Maven 3.9 or newer
- Linux on the target board; the development machine may use another OS

## Build and run

Run these commands from this directory:

```powershell
mvn test
mvn package
java -jar target/omniroy-sbc-controller.jar
```

The packaged JAR is a plain executable JAR with no runtime dependencies yet.
Build on, or cross-build for, the same CPU architecture and operating system as the target board.

## Project layout

```text
sbc-app/
|-- deploy/systemd/       Linux service unit example
|-- docs/                 Architecture and hardware notes
|-- src/main/java/        Application source code
|-- src/main/resources/   Runtime configuration
|-- src/test/java/        Automated tests
|-- pom.xml               Maven build and dependency configuration
`-- README.md             Build and run instructions
```

## Current hardware contract

The Nano firmware uses UART at `115200` baud and accepts one line in
`angle,speed` format, for example `90.00,0.00`. The controller reads
`serial.port` and `serial.baud-rate` from `application.properties`; override
them with `-Dserial.port=...` and `-Dserial.baud-rate=...` when needed.

In a graphical desktop session, use **W/S** to drive forward/backward and
**A/D** to strafe left/right at the constant speed of `8.0`. Diagonal movement
is supported; releasing all movement keys stops the robot. Commands are
printed in the console. Serial port open/write/close calls in `Main` are
commented out for console-only testing; uncomment them when connecting hardware.