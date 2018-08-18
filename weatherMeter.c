/** @file weatherMeter.c
* 
* @brief    An STM32CubeMX compatible library to interface to the Sparkfun
*           Weather Meters (https://www.sparkfun.com/products/8942)
*
* @par       
* 	 COPYRIGHT NOTICE: (c) 2018 Andy Josephson 
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "weatherMeter.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifdef  DEBUG
#include "uart_printf.h"
#include "stm32f1xx_hal.h"
#endif

/**
 * @brief Wind Vane Direction Strings.  E, NE, SSE, etc.
 */
const unsigned char WIND_VANE_DIR_STRING[][4] = {   "N",
                                                    "NNE",
                                                    "NE",
                                                    "ENE",
                                                    "E",
                                                    "ESE",
                                                    "SE",
                                                    "SSE",
                                                    "S",
                                                    "SSW",
                                                    "SW",
                                                    "WSW",
                                                    "W",
                                                    "WNW",
                                                    "NW",
                                                    "NNW" };
/**
 * @brief Wind Vane ADC values.  Measured for my setup.
 */
uint32_t WIND_VANE_VALUES[WIND_VANE_DIRECTIONS_COUNT] = {       3541,
                                                                2476,
                                                                2660,
                                                                1123,
                                                                1171,
                                                                1029,
                                                                1606,
                                                                1334,
                                                                2042,
                                                                1869,
                                                                3159,
                                                                3073,
                                                                3881,
                                                                3635,
                                                                3762,
                                                                3341 };

/**
 * @brief   A Handle to the ADC that will be making the wind vane readings
 *          This ADC needs to be configured to do a DMA transfer and an
 *          appropriate buffer allocated with size WIND_VANE_ADC_BUF_SIZE
 */
ADC_HandleTypeDef* hwindVaneAdc;
/**
 * @brief   The ADC Buffer
 */
uint32_t _adcBuf[WIND_VANE_ADC_BUF_SIZE];
/**
 * @brief   A holder for the average of the ADC buffer
 */
static volatile uint32_t _average;

/**
 * @brief   A Handle to the timer that will act as the counter for the
 *          anemometer
 */
TIM_HandleTypeDef* hwindSpeedTimer;
/**
 * @brief   The raw count from the timer of the anemometer
 */
static volatile uint32_t _windSpeedCount = 0;
/**
 * @brief   The conversion factor from the datasheet to convert counts
 *          to MPH
 */
const double windSpeedConversion_MPH = 1.492;

/**
 * @brief   A Handle to the timer that will act as the counter for the
 *          rain bucket
 */
TIM_HandleTypeDef* hrainBucketCounter;
/**
 * @brief   The raw count from the timer of the rain bucket
 */
static volatile uint32_t _rainBucketCount = 0;
/**
 * @brief   The conversion factor from the datasheet to convert counts
 *          to inches of rain per hour
 */
const double rainBucketConversion_inPerHr = 0.011;

int8_t initWindVane( ADC_HandleTypeDef* hadc )
{
    if( hadc == NULL )
    {   // Something wrong with the ADC Handle
        return 1;
    }
    else
    {   // Grab a reference to the ADC handle and start the ADC
        hwindVaneAdc = hadc;
        HAL_ADC_Start_DMA( hadc, _adcBuf, WIND_VANE_ADC_BUF_SIZE );
        return 0;
    }
}

void processWindVane( void )
{
    _average = 0;

    // Average the buffer
    for( int i=0; i<WIND_VANE_ADC_BUF_SIZE; i++ )
    {
        _average += _adcBuf[i];
    }
    _average /= WIND_VANE_ADC_BUF_SIZE;
}

windVaneDir_t getWindVaneDirection( void )
{
    // Run through the table of ADC values, applying a window and return the
    // one that matches
    for( int i=0; i<WIND_VANE_DIRECTIONS_COUNT; i++ )
    {
        if( ( _average >= ( WIND_VANE_VALUES[i] - WIND_VANE_CODE_BAND ) ) &&
            ( _average <= ( WIND_VANE_VALUES[i] + WIND_VANE_CODE_BAND ) ) )
        {
            return( (windVaneDir_t)i );
        }
    }

    // If this return is reached something has gone wrong
    return WIND_VANE_DIRECTIONS_COUNT;
}

void getWindVaneDirString( windVaneDir_t direction, uint8_t *string )
{
    if( direction < WIND_VANE_DIRECTIONS_COUNT )
    {   // There's a valid direction
        strcpy( (char *)string, (char *)WIND_VANE_DIR_STRING[direction] );
    }
    else if( direction == WIND_VANE_DIRECTIONS_COUNT )
    {   // There's an error
        strcpy( (char *)string, "ERR" );
    }
}

int8_t initWindSpeed( TIM_HandleTypeDef *htim )
{
    if( htim == NULL )
    {   // There's something wrong with the timer handle
        return 1;
    }
    else
    {   // Grab a reference to the timer and start it
        hwindSpeedTimer = htim;
        HAL_TIM_Base_Start( htim );
        return 0;
    }
}

void processWindSpeed( void )
{
    // Grab the current count from the timer
    _windSpeedCount = hwindSpeedTimer->Instance->CNT;
    // Clear it out
    hwindSpeedTimer->Instance->CNT = 0;
}

uint32_t getWindSpeedCount( void )
{
    return _windSpeedCount;
}

double getWindSpeed_MPH( void )
{
    return( windSpeedConversion_MPH * _windSpeedCount );
}

int8_t initRainBucket( TIM_HandleTypeDef *htim )
{
    if( htim == NULL )
    {   // There's something wrong with the timer handle
        return 1;
    }
    else
    {   // Grab a reference to the timer handle and start it
        hrainBucketCounter = htim;
        HAL_TIM_Base_Start( htim );
        return 0;
    }
}

void processRainBucket( void )
{
    // Grab the current count
    _rainBucketCount = hrainBucketCounter->Instance->CNT;
    // Clear it out
    hrainBucketCounter->Instance->CNT = 0;
}

double getRainfall_inperhr( void )
{
    return( _rainBucketCount * rainBucketConversion_inPerHr * 60 );
}
// End of file - weatherMeter.c
