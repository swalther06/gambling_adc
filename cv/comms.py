
# from gpiozero import DigitalInputDevice, DigitalOutputDevice
import serial

"""
# input pins
PIN_DIRECTION = 3
PIN_RESET = 4

# output pins
PIN_LEFT = 17
PIN_RIGHT = 27
PIN_CENTERED = 22
PIN_PLAYER = 23
PIN_SKIP = 24

inp_direction = DigitalInputDevice(PIN_DIRECTION) # 0 is clockwise, 1 is counterclockwise
inp_reset = DigitalInputDevice(PIN_RESET)

out_right = DigitalOutputDevice(PIN_RIGHT)
out_left = DigitalOutputDevice(PIN_LEFT)
out_center = DigitalOutputDevice(PIN_CENTERED)
out_player = DigitalOutputDevice(PIN_PLAYER)
out_skip = DigitalOutputDevice(PIN_SKIP)
"""


#uart
class xmc_link():
    def __init__(self):
        self.ser = serial.Serial(
            "/dev/ttyAMA0", 
            baudrate=115200, 
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_EVEN,
            stopbits=serial.STOPBITS_ONE,
            timeout=0)   # timeout=0: never blocks

        self.SYNC = 0b101 << 5


    def uart_read_inputs(self):
        # read input
        data = self.ser.read(self.ser.in_waiting)

        direction, reset = 0
        for byte in data:
            if byte & 0xE0 == self.SYNC:
                direction = byte & 1
                reset = (byte >> 1) & 1

        return direction, reset
        

    def uart_send_status(self, player : bool, skip : bool, left : bool, right : bool, center : bool):
        # send
        byte = self.SYNC | player << 4 | skip << 3 | left << 2 | right << 1 | center
        self.ser.write(bytes[byte])



