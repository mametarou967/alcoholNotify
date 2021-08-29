#include "M5Atom.h"
#include <BLEDevice.h> // Bluetooth Low Energy 
#include <BLEServer.h> // Bluetooth Low Energy
#include <BLEUtils.h> // Bluetooth Low Energy
#include <esp_sleep.h>

#define T_PERIOD 10 // �A�h�o�^�C�W���O�p�P�b�g�𑗂�b��

#define DAT_PIN 32
#define SCL_PIN 26

RTC_DATA_ATTR static uint8_t seq; // ���MSEQ

uint16_t alcoholValue; // �A���R�[���l
uint16_t vbat = 0; // �d��

void setAdvData(BLEAdvertising *pAdvertising) { // �A�h�o�^�C�W���O�p�P�b�g�𐮌`����
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

    oAdvertisementData.setFlags(0x06); // BR_EDR_NOT_SUPPORTED | General Discoverable Mode
    // oAdvertisementData.setFlags(0x05); // BR_EDR_NOT_SUPPORTED | Limited Discoverable Mode

    std::string strServiceData = "";
    strServiceData += (char)0x08; // �����i12Byte�j
    strServiceData += (char)0xff; // AD Type 0xFF: Manufacturer specific data
    strServiceData += (char)0xff; // Test manufacture ID low byte
    strServiceData += (char)0xff; // Test manufacture ID high byte
    strServiceData += (char)seq; // �V�[�P���X�ԍ�
    strServiceData += (char)(alcoholValue & 0xff); // �A���R�[������̉��ʃo�C�g
    strServiceData += (char)((alcoholValue >> 8) & 0xff); // �A���R�[������̏�ʃo�C�g
    strServiceData += (char)(vbat & 0xff); // �d�r�d���̉��ʃo�C�g
    strServiceData += (char)((vbat >> 8) & 0xff); // �d�r�d���̏�ʃo�C�g

    oAdvertisementData.addData(strServiceData);
    pAdvertising->setAdvertisementData(oAdvertisementData);
}

void setup() {
    M5.begin();
    // M5.Axp.ScreenBreath(10); // ��ʂ̋P�x��������
    // M5.Lcd.setRotation(1); // LCD�̕�����ς���
    // M5.Lcd.setTextSize(2); // �t�H���g�T�C�Y��2�ɂ���
    // M5.Lcd.setTextColor(WHITE, BLACK); // �����𔒁A�w�i����

    pinMode(SCL_PIN,OUTPUT);
    digitalWrite(SCL_PIN,LOW); // Start to heat the sensor
}

void loop() {
    M5.update();
    
    alcoholValue = analogRead(DAT_PIN);
    // vbat = (uint16_t)(M5.Axp.GetVbatData() * 1.1 / 1000 * 100); // �o�b�e���[�̓d��

    // M5.Lcd.fillScreen(BLACK);
    // M5.Lcd.setCursor(0, 0, 1); // �J�[�\���ʒu
    // ���x�A���x�A�C���A�o�b�e���[�̓d���AMAC�A�h���X���f�B�X�v���C�ɕ\���i���؎��̂ݕ\������j
    // M5.Lcd.printf("alv: %d\r\n", alcoholValue);
    // M5.Lcd.printf("vbat: %4.2fV\r\n", (float)vbat / 100);

    BLEDevice::init("blepub-01"); // �f�o�C�X��������
    BLEServer *pServer = BLEDevice::createServer();  // �T�[�o�[�𐶐�

    BLEAdvertising *pAdvertising = pServer->getAdvertising(); // �A�h�o�^�C�Y�I�u�W�F�N�g���擾
    setAdvData(pAdvertising); // �A�h�o�^�C�W���O�f�[�^�[���Z�b�g

    pAdvertising->start(); // �A�h�o�^�C�Y�N��
    delay(T_PERIOD * 1000); // T_PERIOD�b�A�h�o�^�C�Y����
    pAdvertising->stop(); // �A�h�o�^�C�Y��~

    seq++; // �V�[�P���X�ԍ����X�V
    delay(1000);
  
}
