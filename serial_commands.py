"""
Python API for interacting with the footmouse using the serial port.
"""
import functools
import inspect

import serial
import serial.tools.list_ports

TEENSY_SERIAL_MSG_BUFFER_SIZE = 256

modes = {
    "left": 1,
    "middle": 4,
    "right": 2,
    "double": 8,
    "control click": 16,
    "alternate": 32,
    "anywhere": 64,
}

# Command codes.
MSG_IDENTIFY = 4
MSG_SET_BUTTON_FUNCTION = 5
MSG_RESET_BUTTONS_TO_DEFAULT = 6
MSG_ECHO = 7
MSG_TYPE_ASCII_STR = 8
MSG_SET_SAVED_ASCII_STR = 10
MSG_TYPE_SAVED_ASCII_STR = 11


def MemoizeCallNoArgs(function):
    """Decorator to cache function calls with no arguments."""
    sentinel = object()
    value = sentinel

    spec = inspect.getfullargspec(function)

    if spec.args or spec.varargs or spec.varkw or spec.kwonlyargs:
        raise TypeError(
            "Can't apply '@MemoizeNoArgs' decorator" +
            f" to function '{function.__name__}', arguments not allowed.")

    @functools.wraps(function)
    def wrapper():
        nonlocal value
        if value != sentinel:
            return value
        else:
            value = function()
            return value

    return wrapper


def send_serial(port, buf: list[int], block_for_response=False):
    if len(buf) > TEENSY_SERIAL_MSG_BUFFER_SIZE:
        raise Exception("serial messages is to long." +
                        f"Attempted '{len(buf)}' bytes" +
                        f"expected at max '{TEENSY_SERIAL_MSG_BUFFER_SIZE}'" +
                        "bytes.")
    START_MARKER = 16
    STOP_MARKER = 17
    constructed_msg = bytes([START_MARKER, *buf, STOP_MARKER])
    with serial.Serial(port, 9600, write_timeout=1, timeout=2) as s:
        s.write(constructed_msg)
        s.flush()
        if block_for_response:
            return s.readline()
        else:
            return True


@MemoizeCallNoArgs
def find_foot_mouse_com_port() -> str | None:
    """
    Find the COM port of my footmouse device.
    """
    # Iterate through list of available COM port
    # names, e.g. ["COM4", "COM6", "COM9"].
    for port in serial.tools.list_ports.comports():
        try:
            if b"footmouse\n" == send_serial(port.name, [MSG_IDENTIFY],
                                             block_for_response=True):
                return port.name
            else:
                print(f"'{port.name}' not the COM port I'm looking for.")
        except serial.SerialException as ex:
            print(f"{port.name} not available.")
            continue
    return None


def send_to_foot_pedal(buf: list[int], block_for_response=False):
    try:
        if port_name := find_foot_mouse_com_port():
            return send_serial(port_name,
                               buf,
                               block_for_response=block_for_response)
        else:
            return False
    except serial.SerialException as ex:
        print(ex)
        return False


def toggle_mode(pedal: int, mode: int, inverted: int):
    return send_to_foot_pedal([MSG_SET_BUTTON_FUNCTION, pedal, mode, inverted])


def reset_mode():
    return send_to_foot_pedal([MSG_RESET_BUTTONS_TO_DEFAULT, 0, 0, 0])


def type_char(text: str):
    "Send ASCII characters to be typed out by the hardware."
    # Null terminated string.
    return send_to_foot_pedal([
        MSG_TYPE_ASCII_STR, *text.encode(encoding="ASCII", errors='strict'),
        0x00
    ])


def set_stored_string(text: str):
    """
    Set the value of the saved string.
    String must be valid ASCII.
    """
    # Note: Null terminated string.
    send_to_foot_pedal([
        MSG_SET_SAVED_ASCII_STR,
        *text.encode(encoding="ASCII", errors='strict'), 0x00
    ])


def type_stored_string():
    send_to_foot_pedal([MSG_TYPE_SAVED_ASCII_STR])


def echo_test():
    msg = ("Hellow World!" + "\n").encode(encoding="ASCII", errors='strict')
    # Null terminated string.
    buf = [MSG_ECHO, *msg, 0x00]
    print(f"foot pedal message to send: {buf}")
    if result := send_to_foot_pedal(buf, block_for_response=True):
        print(f"result type: {type(result)}")
        print(f"echo test result: {result}")
    else:
        print("no result from echo test")


# Run a test enumerating all serial ports.
if __name__ == "__main__":
    # Note that you can't send serial messages
    # when the serial monitor is open in the Arduino IDE.
    # print out list of available serial ports
    print("Available serial ports:")
    print(serial.tools.list_ports.main())
    print("-----------------------")
    # toggle_mode(1, modes["right"], 0)

    import threading, time
    t = threading.Thread(target=echo_test)
    t.start()
    t.join(timeout=2)
