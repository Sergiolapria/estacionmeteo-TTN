#include <Arduino.h>
#include "LoRaWan_APP.h"
#include <Wire.h>
#include <SPI.h>
#include <AHT20.h>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
#include <Adafruit_BMP280.h>
/* OTAA para mini3 */
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x6D, 0x82};
uint8_t appEui[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x01, 0x00};
uint8_t appKey[] = {0xD3, 0x35, 0x44, 0xAC, 0xC8, 0xFD, 0xCE, 0xA3, 0xFF, 0x83, 0xB4, 0xC3, 0xA6, 0xCA, 0xA8, 0x0E};
/* ABP para*/
uint8_t nwkSKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t devAddr =  ( uint32_t )0x260BCE52;
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
uint32_t appTxDutyCycle = 300000;//5minutos
bool overTheAirActivation = LORAWAN_NETMODE;
bool loraWanAdr = LORAWAN_ADR;
bool keepNet = LORAWAN_NET_RESERVE;
bool isTxConfirmed = LORAWAN_UPLINKMODE;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;
//CONSTRUCTORES
AHT20 aht;
Adafruit_BMP280 bmp;


static void prepareTxFrame( uint8_t port )
{
	float temperaure=aht.getTemperature();
	float humidity=aht.getHumidity();
	//Serial.println("Temperatura: "+String(temperaure)+" Humedad: "+String(humidity));
	int rawTemp=100*temperaure;
	int rawHum=100*humidity;
	//
	float pressure=bmp.readPressure();
	//Serial.println("Presion: "+String(pressure));//Pa
	int rawPress=100*pressure;
	int Current=analogRead(ADC);//0-4095 sensibilidad 180 mV/A

	unsigned char *puc;
	appDataSize = 16;

	puc=(unsigned char*)&rawTemp;
	appData[0] = puc[0];
	appData[1] = puc[1];
	appData[2] = puc[2];
	appData[3] = puc[3]; 

	puc=(unsigned char*)&rawHum;
	appData[4] = puc[0];
	appData[5] = puc[1];
	appData[6] = puc[2];
	appData[7] = puc[3];

	puc=(unsigned char*)&rawPress;
	appData[8] = puc[0];
	appData[9] = puc[1];
	appData[10] = puc[2];
	appData[11] = puc[3];

	puc=(unsigned char*)&Current;
	appData[12] = puc[0];
	appData[13] = puc[1];
	appData[14] = puc[2];
	appData[15] = puc[3];

	Serial.println("Temperatura: "+String(temperaure));
	Serial.print(puc[0]);
	Serial.print(" ");
	Serial.print(puc[1]);
	Serial.print(" ");
	Serial.print(puc[2]);
	Serial.print(" ");
	Serial.print(puc[3]);
	Serial.println();
}
void setup() {
	boardInitMcu();
	Serial.begin(115200);
	Wire.begin();
	if(aht.begin()==false){
		Serial.println("AHT20 m no encontrado");
		while(true);
	}
	Serial.println(F("BMP280 test"));
  	unsigned status;
  	//status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  	status = bmp.begin();
  	if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
#if(AT_SUPPORT)
	enableAt();
#endif
	deviceState = DEVICE_STATE_INIT;
	LoRaWAN.ifskipjoin();
}

void loop()
{
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(AT_SUPPORT)
			getDevParam();
#endif
			printDevParam();
			LoRaWAN.init(loraWanClass,loraWanRegion);
			deviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			prepareTxFrame( appPort );
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
			LoRaWAN.sleep();
			break;
		}
		default:
		{
			deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}
