#! python3.8-32

import serial
from serial import SerialException
import serial.tools.list_ports

print(serial.tools.list_ports.main())

serial.tools.list_ports

# MIDDLE_DISABLE = b"\x0b"
# MIDDLE_PRESS = b"\x04"
# MIDDLE_MODE_SWITCH = b"\x06"


# try:
#     with serial.Serial('COM6', 9600, timeout=0) as S:
#         S.write(MIDDLE_MODE_SWITCH)
#         print("MIDDLE_PRESS")
# except serial.SerialException:pass
