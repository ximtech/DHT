#pragma once

#include "main.h"
#include "DWT_Delay.h"

#define DEGREE_SYMBOL 223		// ASCII degree symbol

typedef enum DHTType {
    DHT11,
    DHT22
} DHTType;

typedef enum DHTReadStatus {
    DHT_OK,
    DHT_CHECKSUM_ERROR,
    DHT_TIMEOUT_ERROR,
    DHT_CONNECT_ERROR,
    DHT_ACK_L_ERROR,
    DHT_ACK_H_ERROR
} DHTReadStatus;

typedef struct DHT {
    GPIO_TypeDef *GPIOx;
    uint32_t GPIOPin;
    DHTType type;
    DHTReadStatus readStatus;

    uint8_t integralRH;
    uint8_t decimalRH;
    int16_t integralT;	// temperature can be negative, setting signed int
    uint8_t decimalT;
    uint8_t checkSum;
} DHT;

DHT initDHT(GPIO_TypeDef *GPIOx, uint32_t GPIOPin, DHTType type);
DHTReadStatus readDHT(DHT *DHTPointer);

uint8_t getDHTHumidityIntegralPart(DHT *DHTPointer);
uint8_t getDHTHumidityDecimalPart(DHT *DHTPointer);
int16_t getDHTTemperatureIntegralPart(DHT *DHTPointer);
uint8_t getDHTTemperatureDecimalPart(DHT *DHTPointer);