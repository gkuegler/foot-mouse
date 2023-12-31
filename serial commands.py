import serial
import serial.tools.list_ports

# command message identifiers
MSG_IDENTIFY = 4
MSG_SET_BUTTONS = 5
MSG_CLEAR_BUTTONS = 6
MSG_ECHO = 7
MSG_SET_TEMP = 8
MSG_KEYBOARD_TYPE_TEMP = 9
MSG_SET_VAULT = 10
MSG_KEYBOARD_TYPE_VAULT = 11

PORT = "COM4"

def send_serial(msg):
    # the 16 & 17 are start and stop markers
    constructed_msg = bytes([16, *msg, 17])
    print(f"[send_serial] constructed_msg: {constructed_msg}")
    with serial.Serial(PORT, 9600, write_timeout=1, timeout=1) as s:
        s.write(constructed_msg)
        s.flush()
        if result := s.readline():
            print(f"result: {result}")


def echo_test():
    msg = "hello world!".encode(encoding="ASCII", errors='strict')
    # Null terminated string.
    send_serial([MSG_ECHO, *msg, 0x00])


def set_temporary(text):
    msg = text.encode(encoding="ASCII", errors='strict')
    # Null terminated string.
    send_serial([MSG_SET_TEMP, *msg, 0x00])


def type_temporary():
    send_serial([MSG_KEYBOARD_TYPE_TEMP])


def set_vault(text):
    msg = text.encode(encoding="ASCII", errors='strict')
    # Null terminated string.
    send_serial([MSG_SET_VAULT, *msg, 0x00])


def type_vault():
    send_serial([MSG_KEYBOARD_TYPE_VAULT])


def main():
    pass


# Run a test enumerating all serial ports.
if __name__ == "__main__":
    # Note that you can't send serial messages
    # when the serial monitor is open in the Arduino IDE.
    # print out list of available serial ports
    print("Available serial ports:")
    print(serial.tools.list_ports.main())
    print("-----------------------")

    echo_test()
    set_temporary("hello world!xxx")
    type_temporary()
