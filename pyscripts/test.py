import math
import time
import serial
import pygame

SERIAL_PORT = "COM8"
BAUD_RATE = 115200
DEADZONE = 0.15
MIN_SPEED = 0
MAX_SPEED = 0.5
KEYBOARD_SPEED = 0.2
SEND_PERIOD = 0.05


def clamp(value, low, high):
    return max(low, min(high, value))


class RobotController:
    def __init__(self, port_name):
        self.port_name = port_name
        self.serial_port = None
        self.connected = False
        self.last_sent_time = 0.0
        self.last_command = "90.00,0.00"

        try:
            self.serial_port = serial.Serial(port_name, BAUD_RATE, timeout=0.1)
            time.sleep(2)
            self.connected = True
            print(f"Connected to {port_name}")
        except serial.SerialException as exc:
            print(f"Serial port {port_name} not available: {exc}")
            self.connected = False

    def send_command(self, angle_deg, speed_value, force=False):
        angle_deg = float(angle_deg)
        speed_value = clamp(float(speed_value), MIN_SPEED, MAX_SPEED)

        command = f"{angle_deg:.2f},{speed_value:.2f}\n"
        now = time.monotonic()

        if not force and (now - self.last_sent_time) < SEND_PERIOD:
            return

        if self.serial_port is not None and self.connected:
            self.serial_port.write(command.encode("ascii"))
            self.serial_port.flush()

        self.last_sent_time = now
        self.last_command = command
        print(f"Send: {command.strip()}")

    def stop(self):
        self.send_command(90.0, 0.0, force=True)
        if self.serial_port is not None and self.serial_port.is_open:
            self.serial_port.close()


class VirtualJoystick:
    def __init__(self, width=240, height=240, radius=90):
        self.width = width
        self.height = height
        self.radius = radius
        self.center_x = width // 2
        self.center_y = height // 2
        self.joy_x = 0.0
        self.joy_y = 0.0
        self.dragging = False
        self.screen = None

    def setup_window(self):
        pygame.init()
        pygame.display.set_caption("OmniRoy: WASD / arrows, Space to stop")
        self.screen = pygame.display.set_mode((self.width, self.height))

    def update_from_mouse(self, pos):
        dx = pos[0] - self.center_x
        dy = pos[1] - self.center_y
        length = math.hypot(dx, dy)

        if length > self.radius:
            scale = self.radius / length
            dx *= scale
            dy *= scale

        self.joy_x = dx / self.radius
        self.joy_y = -dy / self.radius

        if abs(self.joy_x) < DEADZONE:
            self.joy_x = 0.0
        if abs(self.joy_y) < DEADZONE:
            self.joy_y = 0.0

    def reset(self):
        self.joy_x = 0.0
        self.joy_y = 0.0
        self.dragging = False

    def draw(self):
        if self.screen is None:
            return

        self.screen.fill((20, 20, 25))

        base_rect = pygame.draw.circle(self.screen, (70, 70, 80), (self.center_x, self.center_y), self.radius, 3)
        pygame.draw.circle(self.screen, (40, 180, 255), (self.center_x, self.center_y), 16)

        stick_x = self.center_x + self.joy_x * self.radius
        stick_y = self.center_y - self.joy_y * self.radius
        pygame.draw.circle(self.screen, (255, 140, 60), (int(stick_x), int(stick_y)), 28)

        pygame.display.flip()

    def process_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            x, y = event.pos
            dx = x - self.center_x
            dy = y - self.center_y
            if dx * dx + dy * dy <= self.radius * self.radius:
                self.dragging = True
                self.update_from_mouse(event.pos)

        elif event.type == pygame.MOUSEMOTION and self.dragging:
            self.update_from_mouse(event.pos)

        elif event.type == pygame.MOUSEBUTTONUP:
            self.reset()

        return self.joy_x, self.joy_y


class InputController:
    def __init__(self):
        self.virtual_joystick = VirtualJoystick()
        self.virtual_joystick.setup_window()
        self.clock = pygame.time.Clock()
        self.last_sent_value = None

    def update(self, robot):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    robot.send_command(90.0, 0.0, force=True)
                    self.virtual_joystick.reset()
            self.virtual_joystick.process_event(event)

        keys = pygame.key.get_pressed()
        x = float(keys[pygame.K_d] or keys[pygame.K_RIGHT]) - float(keys[pygame.K_a] or keys[pygame.K_LEFT])
        y = float(keys[pygame.K_w] or keys[pygame.K_UP]) - float(keys[pygame.K_s] or keys[pygame.K_DOWN])

        if keys[pygame.K_SPACE]:
            current_value = (90.0, 0.0)
        elif x != 0.0 or y != 0.0:
            length = math.hypot(x, y)
            angle = math.degrees(math.atan2(y / length, x / length))
            current_value = (angle, KEYBOARD_SPEED)
        else:
            x, y = self.virtual_joystick.joy_x, self.virtual_joystick.joy_y
            if abs(x) < DEADZONE and abs(y) < DEADZONE:
                current_value = (90.0, 0.0)
            else:
                angle = math.degrees(math.atan2(y, x))
                speed = clamp(math.hypot(x, y) * MAX_SPEED, MIN_SPEED, MAX_SPEED)
                current_value = (angle, speed)

        if current_value != self.last_sent_value:
            robot.send_command(current_value[0], current_value[1], force=True)
            self.last_sent_value = current_value

        self.virtual_joystick.draw()
        return True

    def tick(self):
        self.clock.tick(60)


def main():
    robot = RobotController(SERIAL_PORT)
    controller = InputController()

    try:
        while True:
            if not controller.update(robot):
                break
            controller.tick()
    finally:
        robot.stop()
        pygame.quit()


if __name__ == "__main__":
    main()

