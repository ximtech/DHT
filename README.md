# DHT

**STM32** LL(Low Layer) library for DHT11, DHT22 sensors

<img src="https://github.com/ximtech/DHT/blob/main/example/dht.PNG" alt="image" width="300"/>

### DHT11

- Ultra low cost
- 3 to 5V power and I/O
- 2.5mA max current use during conversion (while requesting data)
- Good for 20-80% humidity readings with 5% accuracy
- Good for 0-50°C temperature readings ±2°C accuracy
- No more than 1 Hz sampling rate (once every second)
- Body size 15.5mm x 12mm x 5.5mm
- 4 pins with 0.1" spacing

### DHT22

- Low cost
- 3 to 5V power and I/O
- 2.5mA max current use during conversion (while requesting data)
- Good for 0-100% humidity readings with 2-5% accuracy
- Good for -40 to 80°C temperature readings ±0.5°C accuracy
- No more than 0.5 Hz sampling rate (once every 2 seconds)
- Body size 15.1mm x 25mm x 7.7mm
- 4 pins with 0.1" spacing

### Add as CPM project dependency

How to add CPM to the project, check the [link](https://github.com/cpm-cmake/CPM.cmake)

```cmake
CPMAddPackage(
        NAME DHT
        GITHUB_REPOSITORY ximtech/DHT
        GIT_TAG origin/main)
```

### Project configuration

1. Start project with STM32CubeMX:
    * [GPIO configuration](https://github.com/ximtech/DHT/blob/main/example/config.PNG)
2. Select: Project Manager -> Advanced Settings -> GPIO -> LL
3. Generate Code
4. Add sources to project:

```cmake
include_directories(${includes} 
        ${DHT_SENSOR_DIRECTORY})   # source directories

file(GLOB_RECURSE SOURCES ${sources} 
        ${DHT_SENSOR_SOURCES})    # source files
```

3. Then Build -> Clean -> Rebuild Project

## Wiring

- <img src="https://github.com/ximtech/DHT/blob/main/example/wiring.PNG" alt="image" width="300"/>

## Usage

- Usage example: [link](https://github.com/ximtech/DHT/blob/main/example/example.c)