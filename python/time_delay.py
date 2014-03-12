import serial
import struct
import threading
import time
import matplotlib.pyplot as mp
import numpy as np


class sethread(threading.Thread):
    def __init__(self,se):
        self.se=se
        threading.Thread.__init__(self)#must call father class's init function 
    def run(self):
        msg=self.se.read(5)
        while True:
            data_num=self.se.inWaiting()
            msg+=self.se.read(data_num)
            if not msg:
                break
            pos=msg.find('fe55aa'.decode("hex"))
            if pos!=-1:
                msg=msg[pos+4:]  #include a command byte
                if(len(msg)<10):
                    msg+=self.se.read(10-len(msg))
                global data                
                data=struct.unpack('5H',msg)
                print data
                msg=msg[-5:]
            else:
                msg=msg[data_num:]

if __name__ == "__main__":
    s=serial.Serial('/dev/ttyUSB1',115200)
    st=sethread(s)
    st.start()
    while True:
        raw_input("input:")
        send_data='aa000855fe55aa00010203040506070809'.decode("hex")
        s.write(send_data)
        s.flush()
#        time.sleep(2.0)
    s.close()
