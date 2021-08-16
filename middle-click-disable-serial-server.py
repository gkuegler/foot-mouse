import serial.tools.list_ports
from serial import SerialException
import serial
import win32api


print(serial.tools.list_ports.main())

serial.tools.list_ports

MIDDLE_DISABLE = b"\x0b"


while True:
    try:
        with serial.Serial('COM6', 9600, timeout=0) as S:
            while True:
                if (msg_bytes := S.read(250)):
                    print(f"msg_bytes: {msg_bytes}")
                    if msg_bytes == MIDDLE_DISABLE: 
                        x, y = win32api.GetCursorPos()
                        if x < 320 or (y < 190 and x < 1920):                    
                            S.write(MIDDLE_DISABLE)
                            print("middle disabled")
    except SerialException:pass