#include "blynk.h"
#include "WidgetLED.h"
#include "WidgetLCD.h"
#include "PietteTech_DHT.h"
#include "BlynkAuth.h"

// system defines
#define DHTTYPE  DHT22                // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   3         	          // Digital pin for communications
#define DHT_SAMPLE_INTERVAL  5000     // Sample every minute

int led2 = D7;
WidgetLED led1(9);
WidgetLED led3(11);
WidgetLCD lcd1(8);

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// globals
unsigned int DHTnextSampleTime;	    // Next time we want to start sample
bool bDHTstarted;		    // flag to indicate we started acquisition
int n;                              // counter

//this is coming from http://www.instructables.com/id/Datalogging-with-Spark-Core-Google-Drive/?ALLSTEPS
char resultstr[64]; //String to store the sensor data

char VERSION[64] = "0.04";

void setup()
{
  Blynk.begin(BLYNK_AUTH);
  DHTnextSampleTime = 0;  // Start the first sample immediately

  pinMode(led2, OUTPUT);

  Particle.variable("result", resultstr, STRING);
  Particle.publish("DHT22 - firmware version", VERSION, 60, PRIVATE);
}


// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht_wrapper() {
    DHT.isrCallback();
}

void loop()
{
    Blynk.run(); // all the Blynk magic happens here

    // Check if we need to start the next sample
    if (millis() > DHTnextSampleTime) {

    	if (!bDHTstarted) {		// start the sample
    	    DHT.acquire();
    	    bDHTstarted = true;
    	}

      if (!DHT.acquiring()) {		// has sample completed?

        float temp = ((float)DHT.getCelsius() * 9.0) / 5.0 + 32;
        int temp1 = (temp - (int)temp) * 100;

        char tempInChar[32];
        char humidityInChar[32];
        sprintf(tempInChar,"%0d.%d", (int)temp, temp1);
        Particle.publish("The temperature from the dht22 is:", tempInChar, 60, PRIVATE);

        //virtual pin 1 will be the temperature
        Blynk.virtualWrite(V1, tempInChar);
        Blynk.virtualWrite(V3, tempInChar);
        Blynk.virtualWrite(V5, tempInChar);

        //google docs can get this variable
        sprintf(resultstr, "{\"t\":%s}", tempInChar);

        float humid = (float)DHT.getHumidity();
        int humid1 = (humid - (int)humid) * 100;

        sprintf(humidityInChar,"%0d.%d", (int)humid, humid1);
        Particle.publish("The humidity from the dht22 is:", humidityInChar, 60, PRIVATE);

        //virtual pin 2 will be the humidity
        Blynk.virtualWrite(V2, humidityInChar);
        Blynk.virtualWrite(V4, humidityInChar);
        Blynk.virtualWrite(V6, humidityInChar);

        char tempMessage[15];
        char humidMessage[15];
        sprintf(tempMessage, "Temperat: %sF",tempInChar);
        sprintf(humidMessage, "Humidity: %s", humidityInChar);
        lcd1.clear();
        lcd1.print(0,0,tempMessage);
        lcd1.print(0,1,humidMessage);

        led1.on();
        led3.on();
        digitalWrite(led2, HIGH);
        delay(500);
        led1.off();
        led3.off();
        digitalWrite(led2, LOW);

        n++;  // increment counter
        bDHTstarted = false;  // reset the sample flag so we can take another
        DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
      }
    }
}
