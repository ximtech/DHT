#include "DHT.h"

#define DHT11_WAKEUP_MS 20
#define DHT22_WAKEUP_MS 1
#define DHT_RESPONSE_TIMEOUT_US 200      // 200uSec max
#define DHT_RESPONSE_DATA_SIZE_BITS 40  // total data size 5x8 = 40 bits

static void sendSensorRequestForDataRead(DHT *DHTPointer);
static void waitSensorResponse(DHT *DHTPointer);
static void readDHTSensorData(DHT *DHTPointer);
static void storeReceivedData(DHT *DHTPointer, uint8_t bitIndex, uint8_t dataBit);
static DHTReadStatus validateCheckSum(DHT *DHTPointer);
void convertDataForDHT22(DHT *DHTPointer);

static inline uint8_t setBitToDataStore(uint8_t dataStore, uint8_t dataBit) {
    return (dataBit == 0x01) ? ((dataStore << 1) | 0x01) : (dataStore << 1);
}

static inline uint16_t combineTwo8BitValuesToOne(uint8_t integralValue, uint8_t decimalValue) {
    return (integralValue << 8) | decimalValue;
}

DHT initDHT(GPIO_TypeDef *GPIOx, uint32_t GPIOPin, DHTType type) {
    DHT dht = {0};
    dht.GPIOx = GPIOx;
    dht.GPIOPin = GPIOPin;
    dht.type = type;
    dht.integralRH = 0;
    dht.decimalRH = 0;
    dht.integralT = 0;
    dht.decimalT = 0;
    dht.checkSum = 0;
    dwtDelayInit();
    return dht;
}

DHTReadStatus readDHT(DHT *DHTPointer) {
    DHTPointer->readStatus = DHT_OK;
    sendSensorRequestForDataRead(DHTPointer);
    waitSensorResponse(DHTPointer);

    if (DHTPointer->readStatus != DHT_OK) {
        return DHTPointer->readStatus;
    }

    readDHTSensorData(DHTPointer);
    if (DHTPointer->readStatus == DHT_OK) {
        DHTPointer->readStatus = validateCheckSum(DHTPointer);
        if (DHTPointer->readStatus != DHT_OK) {
            return DHTPointer->readStatus;
        }
    }

    if (DHTPointer->type == DHT22) {
        convertDataForDHT22(DHTPointer);
    }

    return DHTPointer->readStatus;
}

uint8_t getDHTHumidityIntegralPart(DHT *DHTPointer) {
    return DHTPointer->integralRH;
}

uint8_t getDHTHumidityDecimalPart(DHT *DHTPointer) {
    return DHTPointer->decimalRH;
}

int16_t getDHTTemperatureIntegralPart(DHT *DHTPointer) {
    return DHTPointer->integralT;
}

uint8_t getDHTTemperatureDecimalPart(DHT *DHTPointer) {
    return DHTPointer->decimalT;
}

static void sendSensorRequestForDataRead(DHT *DHTPointer) { // send 20ms pulse to DHT
    LL_GPIO_SetPinMode(DHTPointer->GPIOx, DHTPointer->GPIOPin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(DHTPointer->GPIOx, DHTPointer->GPIOPin);

    if (DHTPointer->type == DHT11) {
        delay_ms(DHT11_WAKEUP_MS);
    } else {
        delay_ms(DHT22_WAKEUP_MS);
    }
    LL_GPIO_SetOutputPin(DHTPointer->GPIOx, DHTPointer->GPIOPin);
}

static void waitSensorResponse(DHT *DHTPointer) {
    LL_GPIO_SetPinMode(DHTPointer->GPIOx, DHTPointer->GPIOPin, LL_GPIO_MODE_INPUT);
    uint64_t startTimeMicroSeconds = currentMicroSeconds();

    while(LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {	// wait for sensor drop pin to low level
        if (currentMicroSeconds() - startTimeMicroSeconds >= DHT_RESPONSE_TIMEOUT_US) {
            DHTPointer->readStatus = DHT_CONNECT_ERROR;
            return;
        }
    }

    startTimeMicroSeconds = currentMicroSeconds();
    while(!LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {	  // wait for sensor set pin to high level
        if (currentMicroSeconds() - startTimeMicroSeconds >= DHT_RESPONSE_TIMEOUT_US) {
            DHTPointer->readStatus = DHT_ACK_L_ERROR;
            return;
        }
    }

    startTimeMicroSeconds = currentMicroSeconds();
    while(LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {	// wait for sensor start data transmission
        if (currentMicroSeconds() - startTimeMicroSeconds >= DHT_RESPONSE_TIMEOUT_US) {
            DHTPointer->readStatus = DHT_ACK_H_ERROR;
            return;
        }
    }
}

static void readDHTSensorData(DHT *DHTPointer) {
    uint64_t startTimeMicroSeconds = currentMicroSeconds();

    for (uint8_t i = 0; i < DHT_RESPONSE_DATA_SIZE_BITS;) {
        if (!LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {		// wait for next bit

            uint64_t startDataTimeoutCounter = currentMicroSeconds();
            while (!LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {
                if (currentMicroSeconds() - startDataTimeoutCounter > DHT_RESPONSE_TIMEOUT_US) {
                    DHTPointer->readStatus = DHT_TIMEOUT_ERROR;
                    return;
                }
            }

            delay_us(35); 	// low pulse voltage pulse is 26-28us and high pulse is 70us, so take approx high pulse middle delay

            if (LL_GPIO_IsInputPinSet(DHTPointer->GPIOx, DHTPointer->GPIOPin)) {
                storeReceivedData(DHTPointer, i, 0x01);
            } else {
                storeReceivedData(DHTPointer, i, 0x00);
            }

            startTimeMicroSeconds = currentMicroSeconds();
            i++;
        }

        if (currentMicroSeconds() - startTimeMicroSeconds >= DHT_RESPONSE_TIMEOUT_US) {
            DHTPointer->readStatus = DHT_TIMEOUT_ERROR;
            return;
        }
    }

    LL_GPIO_SetPinMode(DHTPointer->GPIOx, DHTPointer->GPIOPin, LL_GPIO_MODE_OUTPUT);    // finish data transmission, set to high
    LL_GPIO_SetOutputPin(DHTPointer->GPIOx, DHTPointer->GPIOPin);
}

static void storeReceivedData(DHT *DHTPointer, uint8_t bitIndex, uint8_t dataBit) {
    if (bitIndex < 8) {											                        // from 0 - 7 bits receiving RH integral values
        DHTPointer->integralRH = setBitToDataStore(DHTPointer->integralRH, dataBit);
    } else if (bitIndex >= 8 && bitIndex < 16) {				                        //  from 8 - 15 bits receiving RH decimal values
        DHTPointer->decimalRH = setBitToDataStore(DHTPointer->decimalRH, dataBit);
    } else if (bitIndex >= 16 && bitIndex < 24) {				                        // from 16 - 23 bits receiving temperature integral values
        DHTPointer->integralT = setBitToDataStore(DHTPointer->integralT, dataBit);
    } else if (bitIndex >= 24 && bitIndex < 32) {				                        // from 24 - 32 bits receiving temperature decimal values
        DHTPointer->decimalT = setBitToDataStore(DHTPointer->decimalT, dataBit);
    } else {													                        // final part is check sum
        DHTPointer->checkSum = setBitToDataStore(DHTPointer->checkSum, dataBit);
    }
}

static DHTReadStatus validateCheckSum(DHT *DHTPointer) {
    uint8_t dataSum = DHTPointer->integralRH + DHTPointer->decimalRH + DHTPointer->integralT + DHTPointer->decimalT;
    return DHTPointer->checkSum == dataSum ? DHT_OK : DHT_CHECKSUM_ERROR;
}

void convertDataForDHT22(DHT *DHTPointer) {
    uint16_t resultData = combineTwo8BitValuesToOne(DHTPointer->integralRH, DHTPointer->decimalRH);
    DHTPointer->integralRH = resultData / 10;	// parse integral humidity data
    DHTPointer->decimalRH = resultData % 10;	// parse decimal humidity data

    bool isTemperatureBelowZeroDegree = ((DHTPointer->integralT >> 7) & 0x01) == 1; // if temperature bits starts with one, then it is negative(see DHT22 datasheet)
    if (isTemperatureBelowZeroDegree) {
        DHTPointer->integralT = (int16_t) (DHTPointer->integralT ^ (1 << 7)); // replace first bit to zero, because it is temperature sign and no need to use it for temp calculation
        resultData = combineTwo8BitValuesToOne(DHTPointer->integralT, DHTPointer->decimalT);
        DHTPointer->integralT = (int16_t)(-(resultData / 10));	                    // parse integral temperature data and set it negative
        DHTPointer->decimalT = resultData % 10;		                                // parse decimal temperature data
    } else {
        resultData = combineTwo8BitValuesToOne(DHTPointer->integralT, DHTPointer->decimalT);
        DHTPointer->integralT = (int16_t)(resultData / 10);
        DHTPointer->decimalT = resultData % 10;
    }
}