#include <M5StickC.h>
#include <BLEDevice.h> // Bluetooth Low Energy 
#include <BLEServer.h> // Bluetooth Low Energy
#include <BLEUtils.h> // Bluetooth Low Energy
#include <esp_sleep.h>

#define T_PERIOD 10 // アドバタイジングパケットを送る秒数

#define DAT_PIN 33
#define SCL_PIN 32

RTC_DATA_ATTR static uint8_t seq; // 送信SEQ

uint16_t alcoholValue; // アルコール値
uint16_t vbat; // 電圧

void setAdvData(BLEAdvertising *pAdvertising) { // アドバタイジングパケットを整形する
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

    oAdvertisementData.setFlags(0x06); // BR_EDR_NOT_SUPPORTED | General Discoverable Mode
    // oAdvertisementData.setFlags(0x05); // BR_EDR_NOT_SUPPORTED | Limited Discoverable Mode

    std::string strServiceData = "";
    strServiceData += (char)0x08; // 長さ（12Byte）
    strServiceData += (char)0xff; // AD Type 0xFF: Manufacturer specific data
    strServiceData += (char)0xff; // Test manufacture ID low byte
    strServiceData += (char)0xff; // Test manufacture ID high byte
    strServiceData += (char)seq; // シーケンス番号
    strServiceData += (char)(alcoholValue & 0xff); // アルコール測定の下位バイト
    strServiceData += (char)((alcoholValue >> 8) & 0xff); // アルコール測定の上位バイト
    strServiceData += (char)(vbat & 0xff); // 電池電圧の下位バイト
    strServiceData += (char)((vbat >> 8) & 0xff); // 電池電圧の上位バイト

    oAdvertisementData.addData(strServiceData);
    pAdvertising->setAdvertisementData(oAdvertisementData);
}

void setup() {
    M5.begin();
    M5.Axp.ScreenBreath(10); // 画面の輝度を下げる
    M5.Lcd.setRotation(1); // LCDの方向を変える
    M5.Lcd.setTextSize(2); // フォントサイズを2にする
    M5.Lcd.setTextColor(WHITE, BLACK); // 文字を白、背景を黒

    pinMode(SCL_PIN,OUTPUT);
    digitalWrite(SCL_PIN,LOW); // Start to heat the sensor
}

void loop() {
    M5.update();
    
    alcoholValue = analogRead(DAT_PIN);
    vbat = (uint16_t)(M5.Axp.GetVbatData() * 1.1 / 1000 * 100); // バッテリーの電圧

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0, 1); // カーソル位置
    // 温度、湿度、気圧、バッテリーの電圧、MACアドレスをディスプレイに表示（検証時のみ表示する）
    M5.Lcd.printf("alv: %d\r\n", alcoholValue);
    M5.Lcd.printf("vbat: %4.2fV\r\n", (float)vbat / 100);

    BLEDevice::init("blepub-01"); // デバイスを初期化
    BLEServer *pServer = BLEDevice::createServer();  // サーバーを生成

    BLEAdvertising *pAdvertising = pServer->getAdvertising(); // アドバタイズオブジェクトを取得
    setAdvData(pAdvertising); // アドバタイジングデーターをセット

    pAdvertising->start(); // アドバタイズ起動
    delay(T_PERIOD * 1000); // T_PERIOD秒アドバタイズする
    pAdvertising->stop(); // アドバタイズ停止

    seq++; // シーケンス番号を更新
    delay(10000);
  
}