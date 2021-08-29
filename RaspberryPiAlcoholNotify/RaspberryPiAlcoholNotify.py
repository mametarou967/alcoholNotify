# -*- coding: utf-8 -*-
# M5StickCにBME280を接続
# 温度、湿度、気圧、電圧を取得してBLEでアドバタイズ（ブロードキャスト）
# M5StickCは10秒：アドバタイズ、20秒：Deep Sleep
# ラズパイ側は常時スキャンし、データーを取得したらprintする

# bluetooth
from bluepy.btle import DefaultDelegate, Scanner, BTLEException
import sys
import struct
from datetime import datetime
# GPS
import serial
import micropyGPS
import threading
import time
# http
import requests

# GPS setup
gpsLatitude = 0.0
gpsLongitude = 0.0
gpsValid = False

# http request setup
headers = {
    'Content-Type': 'application/json',
}

gps = micropyGPS.MicropyGPS(9, 'dd') # MicroGPSオブジェクトを生成する。
                                     # 引数はタイムゾーンの時差と出力フォーマット

def rungps(): # GPSモジュールを読み、GPSオブジェクトを更新する
    s = serial.Serial('/dev/serial0', 9600, timeout=10)
    s.readline() # 最初の1行は中途半端なデーターが読めることがあるので、捨てる
    while True:
        sentence = s.readline().decode('utf-8') # GPSデーターを読み、文字列に変換する
        if sentence[0] != '$': # 先頭が'$'でなければ捨てる
            continue
        for x in sentence: # 読んだ文字列を解析してGPSオブジェクトにデーターを追加、更新する
            gps.update(x)

gpsthread = threading.Thread(target=rungps, args=()) # 上の関数を実行するスレッドを生成
gpsthread.daemon = True
gpsthread.start() # スレッドを起動


class ScanDelegate(DefaultDelegate):
    def __init__(self,sendValid): # コンストラクタ
        DefaultDelegate.__init__(self)
        self.lastseq = None
        self.lasttime = datetime.fromtimestamp(0)
        self.sendValid = sendValid

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev or isNewData: # 新しいデバイスまたは新しいデータ
            for (adtype, desc, value) in dev.getScanData(): # データの数だけ繰り返す
                if desc == 'Manufacturer' and value[0:4] == 'ffff': # テスト用companyID
                    __delta = datetime.now() - self.lasttime
                    # アドバタイズする10秒の間に複数回測定されseqが加算されたものは捨てる（最初に取得された１個のみを使用する）
                    if value[4:6] != self.lastseq and __delta.total_seconds() > 11:
                        self.lastseq = value[4:6] # Seqと時刻を保存
                        self.lasttime = datetime.now()
                        (alcoholValue, volt) = struct.unpack('<hh', bytes.fromhex(value[6:])) # hは2Byte整数（2つ取り出す）
                        print('alcoholValue= {0}、 電圧 = {1} V'.format( alcoholValue, volt/100))
                        print('緯度経度: %2.8f, %2.8f' % (gpsLatitude, gpsLongitude))
                        data = '{"alcohol" : %d, "latitude" : %f, "longitude" : %f }' %(alcoholValue,gpsLatitude,gpsLongitude)
                        
                        if alcoholValue < 3000 and self.sendValid == False:
                            print('send')
                            response = requests.post('http://funnel.soracom.io', headers=headers, data=data)
                            self.sendValid = True

if __name__ == "__main__":
    sendValid = False
    scanner = Scanner().withDelegate(ScanDelegate(sendValid))
    while True:
        # for gps module
        if gps.clean_sentences > 20: # ちゃんとしたデーターがある程度たまったら出力する
            h = gps.timestamp[0] if gps.timestamp[0] < 24 else gps.timestamp[0] - 24
            gpsLatitude = gps.latitude[0]
            gpsLongitude = gps.longitude[0]
            if gpsLatitude > 0 and gpsLongitude > 0:
                gpsValid = True
        # for bluetooth
        if sendValid == True:
            timeDelta = currentTime - datetime.now()
            if timeDelta.minute > 30 :
                sendValid = False
        else:
            currentTime = datetime.now()
        
        try:
            scanner.scan(5.0) # スキャンする。デバイスを見つけた後の処理はScanDelegateに任せる
        except BTLEException:
            ex, ms, tb = sys.exc_info()
            print('BLE exception '+str(type(ms)) + ' at ' + sys._getframe().f_code.co_name)
