import serial
import time
import tkinter as tk
from tkinter import ttk

robot = None
time.sleep(2)

window = tk.Tk()
window.title("OmniRoy motor control")
window.resizable(False, False)

main_frame = ttk.Frame(window, padding=16)
main_frame.grid()

ttk.Label(main_frame, text="Motor speed (-255 to 255)").grid(
    row=0, column=0, columnspan=2, pady=(0, 12)
)

sliders = []
value_labels = []
pending_commands = [None] * 4


def send_speed(motor, value):
    speed = int(float(value))
    value_labels[motor].configure(text=str(speed))
    command = f"{motor + 1},{speed}\n"
    print(f"Sending command: {command.strip()}")
    if robot is not None:
        robot.write(command.encode("ascii"))


def schedule_speed(motor, value):
    if pending_commands[motor] is not None:
        window.after_cancel(pending_commands[motor])

    speed = int(float(value))
    value_labels[motor].configure(text=str(speed))
    pending_commands[motor] = window.after(
        80, lambda: send_scheduled_speed(motor, speed)
    )


def send_scheduled_speed(motor, speed):
    pending_commands[motor] = None
    send_speed(motor, speed)


def set_slider_from_event(event, motor):
    slider = sliders[motor]
    start_x = slider.coords(255)[0]
    end_x = slider.coords(-255)[0]
    position = max(start_x, min(end_x, event.x))
    value = round(255 - (position - start_x) * 510 / (end_x - start_x))
    slider.set(value)
    return "break"


def send_slider_value(event, motor):
    if pending_commands[motor] is not None:
        window.after_cancel(pending_commands[motor])
        pending_commands[motor] = None
    send_speed(motor, sliders[motor].get())


def stop_all():
    for motor, slider in enumerate(sliders):
        if pending_commands[motor] is not None:
            window.after_cancel(pending_commands[motor])
            pending_commands[motor] = None
        slider.set(0)
        send_speed(motor, 0)


def close_window():
    stop_all()
    if robot is not None:
        robot.close()
    window.destroy()


for motor in range(4):
    ttk.Label(main_frame, text=f"Motor {motor + 1}").grid(
        row=motor + 1, column=0, sticky="w", padx=(0, 10)
    )

    slider = tk.Scale(
        main_frame,
        from_=255,
        to=-255,
        orient=tk.HORIZONTAL,
        length=280,
        resolution=1,
        showvalue=False,
        command=lambda value, index=motor: schedule_speed(index, value),
    )
    slider.grid(row=motor + 1, column=1)
    slider.bind(
        "<Button-1>",
        lambda event, index=motor: set_slider_from_event(event, index),
    )
    slider.bind(
        "<B1-Motion>",
        lambda event, index=motor: set_slider_from_event(event, index),
    )
    slider.bind(
        "<ButtonRelease-1>",
        lambda event, index=motor: send_slider_value(event, index),
    )
    sliders.append(slider)

    value_label = ttk.Label(main_frame, text="0", width=5, anchor="e")
    value_label.grid(row=motor + 1, column=2, padx=(10, 0))
    value_labels.append(value_label)

ttk.Button(main_frame, text="Stop all", command=stop_all).grid(
    row=5, column=0, columnspan=3, pady=(14, 0), sticky="ew"
)

window.protocol("WM_DELETE_WINDOW", close_window)
window.mainloop()
