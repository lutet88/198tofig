# requires pySerial 3.4+
import serial
import struct
import threading
import time


class DummyThreadSignal:
    def __init__(self, status):
        self.set_status(status)

    def set_status(self, status):
        self.status = status

    def get_status(self):
        return self.status


class L5CXDataObject:
    def __init__(self):
        # initialize a zeroed 8x8 array
        self.distance_mm = []
        for i in range(8):
            self.distance_mm.append([0, 0, 0, 0, 0, 0, 0, 0])

    def load_data(self, raw_data):
        for i in range(64):
            x = i % 8
            y = i // 8
            self.distance_mm[y][x] = raw_data[i]

    def __repr__(self):
        return f"[\n\t{self.distance_mm[0]}\n\t{self.distance_mm[1]}\n\t{self.distance_mm[2]}\n\t{self.distance_mm[3]}\n\t{self.distance_mm[4]}\n\t{self.distance_mm[5]}\n\t{self.distance_mm[6]}\n\t{self.distance_mm[7]}\n]"


class L5CX:
    def __init__(self, port):
        self.ser = serial.Serial(port=None, baudrate=115200)
        self.ser.port = port
        self.read_thread = threading.Thread(target=self.__read)
        self.enable_signal = DummyThreadSignal(False)
        self.data = L5CXDataObject()

    def start(self):
        self.ser.open()
        self.enable_signal.set_status(True)
        self.read_thread.start()

    def stop(self):
        self.ser.close()
        self.enable_signal.set_status(False)
        self.read_thread.join()

    def __read(self):
        while self.enable_signal.get_status():
            if self.ser.in_waiting >= 130:
                # there may be a valid packet
                # read one byte
                possible_header = self.ser.read()
                if possible_header != b'[':
                    # continue to iterate
                    continue

                raw_data = struct.unpack('<'+'H'*64, self.ser.read(size=128))

                possible_footer = self.ser.read()
                if possible_footer != b']':
                    # oops the packet is bad
                    continue

                self.data.load_data(raw_data)
        time.sleep(1e-4)


if __name__ == "__main__":
    port = input("enter serial port: ")
    sensor = L5CX(port)
    sensor.start()
    for i in range(100):
        print(sensor.data)
        time.sleep(1/15)
    sensor.stop()
