#include <LowPower.h>
#include <TimeLib.h>    
#include <Wire.h>      
#include <DS3232RTC.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <OneWire.h>
#include <DallasTemperature.h>


DS3232RTC Clock;

// Data wire is plugged into port 2
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// LCD Object
Adafruit_PCD8544 MyLcd = Adafruit_PCD8544(13,12,11, 10, 9); //software SPI - is it better? For hardware: Adafruit_PCD8544(12, 5, 4)


float tempC;
char Timestring[14]; // Format the time output to the LCD

const int led = 0;
float BatteryV;


void setup()
{
	Serial.begin(9600);
	setTime(Clock.get());
//	setSyncProvider(Clock.get);   // the function to get the time from the RTC
	if (timeStatus() != timeSet)
		Serial.println("Unable to sync with the RTC");
	else
		Serial.println("RTC has set the system time");

	pinMode(led, OUTPUT);
	digitalWrite(led, 1);
	Wire.begin();
	analogReference(INTERNAL);

	SetupTemeratureSensor();

	// Nokia Display
	MyLcd.begin();
	MyLcd.setContrast(60);
	MyLcd.setTextColor(BLACK);
	MyLcd.clearDisplay();
	MyLcd.setFont();
	MyLcd.setCursor(0, 0);

}

void loop()
{
	setTime(Clock.get());
	BatteryV = float(analogRead(A0) / float(1024)*4.96); // Internal reference 1.1V, divider 550/150 kOhm. It might be that analog pin impedance influences
	ShowDisplay();

	// Enter power down state for 8 s with ADC and BOD module disabled
	// Next 3 lines disable built-in LED
	int spi_save = SPCR;
	SPCR = 0;
	digitalWrite(13, 0);
	LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
	// Back from sleep
	SPCR = spi_save;
}

void SetupTemeratureSensor()
{
	sensors.begin();
	sensors.getDeviceCount();
	sensors.getAddress(insideThermometer, 0);
	sensors.setResolution(insideThermometer, 12);
}

void ShowDisplay(void)
{
	sprintf(Timestring, "%02d:%02d   %02d.%02d", hour(), minute(), day(), month());
	MyLcd.clearDisplay();
	MyLcd.setTextSize(1);
	MyLcd.setFont();
	MyLcd.setCursor(2, 0);
	MyLcd.print(Timestring);
	MyLcd.setFont(&FreeSansBold12pt7b);
	sensors.requestTemperatures();
	tempC = floor(sensors.getTempC(insideThermometer) * 10 + 0.5) / 10;
	MyLcd.setCursor(16, 27);
	if (tempC < 0) MyLcd.setCursor(12, 27);
	MyLcd.print(tempC, 1);
	MyLcd.setFont();
	MyLcd.setCursor(68, 8);
	MyLcd.setTextSize(2);
	MyLcd.print("o");
	MyLcd.setTextSize(1);
//	MyLcd.setCursor((84 - 6 * message.length()) / 2, 31);
//	MyLcd.print(message);
	MyLcd.setCursor(0, 40);
	MyLcd.print(String("Battery ") + String(BatteryV, 2));
	MyLcd.setCursor(78, 40);
	MyLcd.print("V");
	MyLcd.display();
}
