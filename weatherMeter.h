/** @file weatherMeter.h
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

#ifndef _weatherMeter_H
#define _weatherMeter_H

#include <stdint.h>

#ifdef USE_HAL_DRIVER
#include "adc.h"
#include "tim.h"
#else
#error "HAL Module Not Enabled"
#endif /* USE_HAL_DRIVER */

/**
 * @brief   WIND_VANE_ADC_BUF_SIZE - set this to the number of samples
 *          you want the ADC to take before the DMA transfer is 
 *          initiated
 */
#define WIND_VANE_ADC_BUF_SIZE 64
/**
 * @brief   WIND_VANE_CODE_BAND - this value will set the window size
 *          when measuring the ADC.  Adjust this value to compensate
 *          for noise in the system
 */
#define WIND_VANE_CODE_BAND  20

/**
 * @brief   An enum to hold the wind vane directions
 *          WIND_VANE_DIRECTIONS_COUNT will be used as
 *          both a count of the enum elements as well
 *          as a general error code
 */
typedef enum WIND_VANE_DIRECTIONS
{
    N = 0,
    NNE,
    NE,
    ENE,
    E,
    ESE,
    SE,
    SSE,
    S,
    SSW,
    SW,
    WSW,
    W,
    WNW,
    NW,
    NNW,
    WIND_VANE_DIRECTIONS_COUNT
} windVaneDir_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Wind vane initialization function
 * @param   hadc - A pointer to the handle of the ADC that will
 *          be making the wind vane measurements
 * @retval  0 on success, 1 on failure
 */
int8_t initWindVane( ADC_HandleTypeDef* hadc );
/**
 * @brief   Call this to do the averaging of the ADC buffer.  For
 *          best results, call this in the DMA transfer complete
 *          callback.  Attention must be paid to the size of the buffer
 *          so as not to spend too much time in the ISR.
 * @param   None
 * @retval  None
 */
void processWindVane( void );
/**
 * @brief   Searches the values table and returns the current direction
 *          matching the most recent ADC average value
 * @param   None
 * @retval  The current wind vane direction.
 */
windVaneDir_t getWindVaneDirection( void );
/**
 * @brief   Retrieves the string matching the direction given.
 * @param   direction - The direction to retreive the string for.
 * @param   string - A pointer to a string to store the value.  Size of the
 *          array must be 4 chars.
 * @retval  None
 */
void getWindVaneDirString( windVaneDir_t direction, uint8_t *string );

/**
 * @brief   Wind speed initialization function
 * @param   htim - A pointer to the handle for the Timer that will be
 *          acting as the counter for the anemometer
 * @retval  0 on success, 1, on failure
 */
int8_t initWindSpeed( TIM_HandleTypeDef *htim );
/**
 * @brief   Call this function to read the counter and
 *          update the variable
 * @param   None
 * @retval  None
 */
void processWindSpeed( void );
/**
 * @brief   Returns the raw count of the wind speed sensor
 * @param   None
 * @retval  The raw count of the wind speed sensor
 */
uint32_t getWindSpeedCount( void );
/**
 * @brief   Convience function to convert raw count to MPH
 * @param   None
 * @retval  A double that indicates the average wind speed in MPH
 */
double getWindSpeed_MPH( void );
/**
 * @brief   Rain bucket initialization function
 * @param   htim - A pointer to the handle for the Timer that will be
 *          acting as the counter for the rain bucket
 * @retval  0 on success, 1, on failure
 */
int8_t initRainBucket( TIM_HandleTypeDef *htim );
/**
 * @brief   Call this function once a minute to read the counter and
 *          update the variable
 * @param   None
 * @retval  None
 */
void processRainBucket( void );
/**
 * @brief   Returns the converted count in inches per hour
 * @param   None
 * @retval  Average rainfall in inches per hour
 */
double getRainfall_inperhr( void );

#ifdef __cplusplus
}
#endif

#endif /* _weatherMeter_H */