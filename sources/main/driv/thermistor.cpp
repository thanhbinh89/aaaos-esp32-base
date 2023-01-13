
#include <math.h>
#include "thermistor.h"

#if defined(THERMISTOR_DEBUG_EN)
#include "app_log.h"
#endif

// Temperature for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// Number of ADC samples
#define NUMSAMPLES         5
// ADC resolution
#define ADC_RESOLUTION          4095
#define VERBOSE_SENSOR_ENABLED  1
/**
 * THERMISTOR
 *
 * Class constructor
 *
 * @param adcPin Analog pin where the thermistor is connected
 * @param nomRes Nominal resistance at 25 degrees Celsius
 * @param bCoef beta coefficient of the thermistor
 * @param serialRes Value of the serial resistor
 */

//THERMISTOR::THERMISTOR(uint8_t adcPin, uint16_t nomRes, uint16_t bCoef, uint16_t serialRes)
//{
//	analogPin = adcPin;
//	nominalResistance = nomRes;
//	bCoefficient = bCoef;
//	serialResistance = serialRes;
//}

/**
 * begin
 *
 * init adc for thermistor
 *
 * @return none
 */
void THERMISTOR::begin(uint8_t adcPin, uint16_t nomRes, uint16_t bCoef, uint16_t serialRes) {
	analogPin = adcPin;
	nominalResistance = nomRes;
	bCoefficient = bCoef;
	serialResistance = serialRes;
}

/**
 * read
 *
 * Read temperature from thermistor
 *
 * @return temperature in 0.1 ºC
 */
int THERMISTOR::read_i(void)
{
	float val = this->read_f();
	return (int)val * 10;
}

float THERMISTOR::read_f() {
	uint8_t i;
	uint16_t sample;
	float average = 0;

	// take N samples in a row, with a slight vTaskDelay
	for (i=0; i< NUMSAMPLES; i++)
	{
		sample = readADC1Channel(analogPin);
		average += sample;
		vTaskDelay(10);
	}
	average /= NUMSAMPLES;

#if defined(THERMISTOR_DEBUG_EN)
	APP_DBG("Average analog reading = :%d\n",(int)average);
#endif

	// convert the value to resistance
	average = ADC_RESOLUTION / average - 1;
	average = serialResistance / average;
#if defined(THERMISTOR_DEBUG_EN)
	APP_DBG("Thermistor resistance : %d\n",(int)average);
#endif

	float steinhart;
	steinhart = (average / nominalResistance);   // (R/Ro)
	steinhart = log(steinhart);                  // ln(R/Ro)
	steinhart /= bCoefficient;                   // 1/B * ln(R/Ro)
	steinhart += (float)1.0 / ((float)TEMPERATURENOMINAL + (float)273.15); // + (1/To)
	steinhart = (float)1.0 / steinhart;                 // Invert
	steinhart -= (float)273.15;                         // convert to C
#if defined(THERMISTOR_DEBUG_EN)
	APP_DBG("Temperature =:%d *C\n",(int)steinhart);
#endif
	return (steinhart);
}
