"""
Python API for interacting with the footmouse using the serial port.
"""
import functools
import inspect
from enum import IntEnum
import struct

import serial
import serial.tools.list_ports

# For future crc verification.
# import zlib

TEENSY_PAYLOAD_BUFFER_SIZE = 512
BAUD_RATE = 115200

NAME = b"footmouse\n"
TESTING_COM_PORT_NAME = "COM3"
TESTING = False


class modes(IntEnum):
    none = 0
    left = 1
    middle = 4
    right = 2
    double = 8
    control_click = 15
    alternate = 32
    anywhere = 64
    orbit = 67
    keycombo = 68


# Command codes.
CMD_IDENTIFY = 4
CMD_SET_BUTTON_FUNCTION = 5
CMD_SET_BUTTON_FUNCTION_EX = 51
CMD_RESET_BUTTONS_TO_DEFAULT = 6
CMD_ECHO = 7
CMD_TYPE_ASCII_STR = 8
CMD_SET_SAVED_ASCII_STR = 10
CMD_TYPE_SAVED_ASCII_STR = 11


def convert_to_zstr_bytes(string: str):
    return string.encode(encoding="ASCII", errors='strict') + b"\x00"


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


def send_serial(port, buf: bytes, block_for_response=False):
    with serial.Serial(port, BAUD_RATE, write_timeout=1, timeout=1) as s:
        s.write(buf)
        s.flush()
        if TESTING:
            block_for_response = True
        if block_for_response:
            from time import sleep
            sleep(0.25)
            lines = s.readlines()
            for l in lines:
                print(l)
            return lines
        else:
            return True


def send_serial_and_get_lines(port, buf: bytes):
    result = send_serial(port, buf, block_for_response=True)
    if isinstance(result, list):
        return result
    else:
        raise Exception()


def get_structured_bytes(cmd: int, payload: bytes = b""):
    SOF = 0xFFFFFFFF
    length = len(payload)
    crc = 0xCAFECAFE

    # Some commands don't have associated payload.
    if length > TEENSY_PAYLOAD_BUFFER_SIZE:
        raise Exception("serial messages is to long." +
                        f"Attempted '{len(payload)}' bytes" +
                        f"expected at max '{TEENSY_PAYLOAD_BUFFER_SIZE}'" +
                        "bytes.")

    msg_bytes = struct.pack("<IIII", SOF, length, crc, cmd) + payload
    print(f"msg_bytes: {msg_bytes}")
    return msg_bytes


@MemoizeCallNoArgs
def find_footmouse_com_port_name() -> str | None:
    """
    Find the COM port of my footmouse device.
    """
    # Iterate through list of available COM port
    # names, e.g. ["COM4", "COM6", "COM9"].
    if TESTING:
        return TESTING_COM_PORT_NAME

    for port in serial.tools.list_ports.comports():
        try:
            lines = send_serial_and_get_lines(
                port.name, get_structured_bytes(CMD_IDENTIFY))
            # There could be several log messages over serial that get in the way.
            if NAME in lines:
                return port.name
            else:
                # print(f"'{port.name}' not the COM port I'm looking for.")
                continue
        except serial.SerialException as ex:
            print(f"{port.name} not available.")
            continue
    print("No Footmouse port found.")
    return None


def send_cmd_to_foot_pedal(cmd: int,
                           payload: bytes = b"",
                           block_for_response=False):
    try:
        if port_name := find_footmouse_com_port_name():
            return send_serial(port_name,
                               get_structured_bytes(cmd, payload),
                               block_for_response=block_for_response)
        else:
            return False
    except serial.SerialException as ex:
        print(ex)
        return False


def echo_test():
    msg = "Hello World!"
    if result := send_cmd_to_foot_pedal(CMD_ECHO,
                                        convert_to_zstr_bytes(msg),
                                        block_for_response=True):
        print(f"echo test result: {result}")
    else:
        print("no result from echo test")


def reset_modes_to_default():
    return send_cmd_to_foot_pedal(CMD_RESET_BUTTONS_TO_DEFAULT)


def change_mode(pedal: int, mode: int, inverted: int):
    return send_cmd_to_foot_pedal(CMD_SET_BUTTON_FUNCTION,
                                  struct.pack("<BBB", pedal, mode, inverted))


def type_char(text: str):
    "Send ASCII characters to be typed out by the hardware."
    return send_cmd_to_foot_pedal(CMD_TYPE_ASCII_STR,
                                  convert_to_zstr_bytes(text))


def set_stored_string(text: str):
    """
    Set the value of the saved string.
    String must be valid ASCII.
    """
    # Note: Null terminated string.
    send_cmd_to_foot_pedal(CMD_SET_SAVED_ASCII_STR,
                           convert_to_zstr_bytes(text))


def type_stored_string():
    send_cmd_to_foot_pedal(CMD_TYPE_SAVED_ASCII_STR)


def generate_keycode_bytes(keycodes: list[int | str]) -> bytes:
    # Create a struct packed with little-endian uint16_t's.
    # See Teensy docs on keycodes being uint16's.
    fmt = '<' + ('H' * len(keycodes))
    # Allow the use of 'c' literals in keycodes.
    san_keycodes = map(lambda k: ord(k) if isinstance(k, str) else k, keycodes)
    return struct.pack(fmt, *san_keycodes)


def set_keycombo(btn: int, keycodes: list[int | str], inverted: int = 0):
    """
    keycodes list need to be a single character or key.
    """
    payload = (btn.to_bytes(1) + inverted.to_bytes(1) +
               len(keycodes).to_bytes(1) + generate_keycode_bytes(keycodes))
    if result := send_cmd_to_foot_pedal(CMD_SET_BUTTON_FUNCTION_EX, payload):
        print(f"result: {result}")


def print_available_serial_ports():
    print("Available serial ports:")
    print(serial.tools.list_ports.main())
    print("----------------------------")


if __name__ == "__main__":
    # Note that you can't send serial messages
    # when the serial monitor is open in the Arduino IDE!

    from time import sleep
    from scan_codes import *

    # TESTING_COM_PORT_NAME = "COM12"
    # TESTING = True

    # print_available_serial_ports()
    # echo_test()
    # type_char("hello\n")
    # set_stored_string("storedstringtest\n")
    # type_stored_string()
    change_mode(2, modes.double, 0)
    # sleep(4)
    # reset_modes_to_default()
    # set_keycombo(2, [MODIFIERKEY_SHIFT, "c"])
