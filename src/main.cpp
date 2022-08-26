// *****************************************************************************
// Title		: This is a template
// File Name	: 'main.cpp'
// Target MCU	: Espressif ESP32 (Doit DevKit Version 1)
//
// Revision History:
// When			Who			Description of change
// -----------	-----------	-----------------------
// 01-AUG-2022	your name	program start
//
// *****************************************************************************

// Include Files
// *****************************************************************************
#include <Arduino.h>
#include <movingAvg.h>

const int LEDPIN = 15;
const int BUTTON = 17;
const int DEBOUNCETIME= 200;
unsigned long debounceMillis = 0;
int ledPinState = LOW;  
int ledBuiltState = LOW;   

int potCount = 0;
int tempCount = 0;
float potVolt = 0.0;
float tempVolt = 0.0;
float tempC = 0.0;
float tempF = 0.0;

const int POT_PIN = 36;
const int TEMP_PIN = 39;
const float adcConversion = 3.3/4095; 

volatile bool flagInterrupt = false;
volatile bool flagTimer0 = false;
volatile bool flagTimer1 = false;

movingAvg adcPotAvg(16);
movingAvg adcTempAvg(16);

hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
portMUX_TYPE timer0Mux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timer1Mux = portMUX_INITIALIZER_UNLOCKED;

// Interrupt Routine
// *****************************************************************************
// void IRAM_ATTR buttonPressed()
// {
// 	if ((millis() - debounceMillis) >= DEBOUNCETIME)
//   	{
// 		debounceMillis = millis();
// 		flagInterrupt = true;
//   	}
// }

void IRAM_ATTR timer0ISR() 
{
	portENTER_CRITICAL_ISR(&timer0Mux);
	flagTimer0 = true;
	portEXIT_CRITICAL_ISR(&timer0Mux);
}

void IRAM_ATTR timer1ISR() 
{
	portENTER_CRITICAL_ISR(&timer1Mux);
	flagTimer1 = true;
	portEXIT_CRITICAL_ISR(&timer1Mux);
}

// Begin Code
// *****************************************************************************
void setup()
{
	Serial.begin(9600);
	pinMode(LEDPIN, OUTPUT);
	pinMode(BUTTON, INPUT);
	pinMode(BUILTIN_LED, OUTPUT);

	//attachInterrupt(BUTTON, buttonPressed, HIGH);
	
	adcPotAvg.begin();
	adcTempAvg.begin();

	timer0= timerBegin(0, 80, true);
  	timerAttachInterrupt(timer0, &timer0ISR, true);
 	timerAlarmWrite(timer0, 1000000, true);
  	timerAlarmEnable(timer0);

	timer1= timerBegin(1, 80, true);
  	timerAttachInterrupt(timer1, &timer1ISR, true);
 	timerAlarmWrite(timer1, 500000, true);
	timerAlarmEnable(timer1);
}
// Main program
// *****************************************************************************
void loop()
{
	

	// if (flagInterrupt)						//button stuff
	// {
	// 	flagInterrupt = false;
	// 	ledPinState = !ledPinState;

	// 	if (ledPinState)
	// 	{
	// 		digitalWrite(LEDPIN, HIGH);
	// 	}	
	// 	else
	// 	{
	// 		digitalWrite(LEDPIN, LOW);
	// 	}

	// }

	

	if (flagTimer0)							//blue led heart beat 
	{
		portENTER_CRITICAL(&timer0Mux);
		flagTimer0 = false;
		portEXIT_CRITICAL(&timer0Mux);	
		ledBuiltState = !ledBuiltState;
		digitalWrite(LED_BUILTIN, ledBuiltState);
	}
	if (flagTimer1)
	{
		portENTER_CRITICAL(&timer1Mux);
		flagTimer1 = false;
		portEXIT_CRITICAL(&timer1Mux);	
		potCount = adcPotAvg.reading(analogRead(POT_PIN));			//potCount = analogRead(POT_PIN);
		potVolt = (potCount*adcConversion) + 0.1;
		Serial.printf("ADC pot: count = %d, volts = %f \r\n", potCount, potVolt);
		tempCount = adcTempAvg.reading(analogRead(TEMP_PIN));
		tempVolt = (tempCount * adcConversion);
		Serial.printf("ADC temp: count = %d, volts = %f \r\n", tempCount, tempVolt);
		tempC = tempVolt/.010; 			//check the scale its in millivolts 
		tempF = (tempC*1.8) + 32;
		Serial.printf("Temperature: %.2f%sF, or %.2f%sC \r\n", tempF,"\xB0", tempC, "\xB0");
	}
}