# -*- coding: utf-8 -*-

from bluepy.btle import DefaultDelegate, Scanner, BTLEException
import sys
import struct
from datetime import datetime

class ScanDelegate(DefaultDelegate):
    def __init__(self): # ???????
        DefaultDelegate.__init__(self)
        self.lastseq = None
        self.lasttime = datetime.fromtimestamp(0)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev or isNewData: # ????????????????
            for (adtype, desc, value) in dev.getScanData(): # ???????????
                if desc == 'Manufacturer' and value[0:4] == 'ffff': # ????companyID
                    __delta = datetime.now() - self.lasttime
                    # ????????10???????????seq???????????????????????????????
                    if value[4:6] != self.lastseq and __delta.total_seconds() > 11:
                        self.lastseq = value[4:6] # Seq??????
                        self.lasttime = datetime.now()
                        (alcoholValue, volt) = struct.unpack('<hh', bytes.fromhex(value[6:])) # h?2Byte???2??????
                        print('alcoholValue= {0}? ?? = {1} V'.format( alcoholValue, volt/100))

if __name__ == "__main__":
    scanner = Scanner().withDelegate(ScanDelegate())
    while True:
        try:
            scanner.scan(5.0) # ?????????????????????ScanDelegate????
        except BTLEException:
            ex, ms, tb = sys.exc_info()
            print('BLE exception '+str(type(ms)) + ' at ' + sys._getframe().f_code.co_name)

