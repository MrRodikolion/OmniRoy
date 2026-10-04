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

The Nano firmware uses UART at `115200` baud. Its current input is one line in
`angle,speed` format, for example `90.00,0.00`. The Java application does not
open the serial port or send motion commands yet. The board model, Linux image,
and serial-port library should be confirmed before that integration is added.