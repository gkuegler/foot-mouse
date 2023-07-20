import serial
import serial.tools.list_ports

import threading, time

# command message identifiers
MSG_IDENTIFY = 4
MSG_SET = 5
MSG_RESET_BUTTONS = 6
MSG_SET_SECRET = 7
MSG_KEYBOARD_TYPE_SECRET = 8
MSG_ECHO = 9

PORT = "COM3"


def send_serial(msg):
    # the 16 & 17 are start and stop markers
    constructed_msg = bytes([16, *msg, 17])
    print(f"[send_serial] constructed_msg: {constructed_msg}")
    with serial.Serial(PORT, 9600, write_timeout=1, timeout=2) as s:
        s.write(constructed_msg)
        s.flush()
        # time.sleep(2)
        # print(f" in_waiting: { s.in_waiting}")
        # result = s.read_until("\n")
        result = s.readline()
        print(f"result: {result}")

        # if get_read:
        #     readline_result = s.readline()
        #     print(f"readline_result: {readline_result}")
        #     return readline_result
        # else:
        #     return True


def echo_test():
    msg = "hello world".encode(encoding="UTF-8")
    foot_pedal_message = [MSG_ECHO, *msg, 0]
    print(f"foot pedal message to send: {foot_pedal_message}")
    send_serial(foot_pedal_message)
    # if result:
    #     print(f"result type: {type(result)}")
    #     print(f"echo test result: {result}")
    # else:
    #     print("no result from echo test")


# Run a test enumerating all serial ports.
if __name__ == "__main__":
    # Note that you can't send serial messages
    # when the serial monitor is open in the Arduino IDE.
    # print out list of available serial ports
    print("Available serial ports:")
    print(serial.tools.list_ports.main())
    print("-----------------------")

    # import threading, time
    # t = threading.Thread(target=echo_test)
    # t.start()
    # t.join(timeout=2)
    echo_test()
