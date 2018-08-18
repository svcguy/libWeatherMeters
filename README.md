# libWeatherMeters
STM32CubeMX based library for the Sparkfun Weather Meters

This library uses the STM32CubeMX HAL drivers to interface with the Sparkfun Weather Meters

STM32CubeMX - https://www.st.com/en/development-tools/stm32cubemx.html
Sparkfun Weather Meters - https://www.sparkfun.com/products/8942

The library performs the following tasks:

* Reads the wind vane and returns the direction as an integer and optionally a string
* Reads the anemometer and returns the wind speed in MPH
* Reads the rain bucket and returns the rainfall in in/hr

The library is written in C

The library requires the following microcontroller hardware resources:

* One ADC channel with DMA to read the wind vane
* One Timer configured as a digital counter to read the anemometer
* One Timer configured as a digital counter to read the rain bucket

The sensor requires some basic electronics to interface to the micro, see https://www.sparkfun.com/products/13956 for more information

The wind vane can be read as often as you'd like, the anemometer is setup to be read once a second and the rain bucket, once a minute
