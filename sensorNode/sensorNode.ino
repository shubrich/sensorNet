/* 
  Sensor node
  Gather temperature and humidity and send them to a master node
*/

#include <DHT.h>
#include <SPI.h>
#include <RF24Network.h>
#include <RF24.h>
#include <Ports.h>
#include <stdlib.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302)

const int LED_PIN = 9;
const int BUTTON_PIN = 4;
const int DHT22_PIN = 3;

// Init network (RF_CE, RF_CSN)
RF24 radio(8,7);
RF24Network network(radio);
const uint16_t thisNode = 1;
const uint16_t otherNode = 0;

// Init DHT22 sensor
DHT dht(DHT22_PIN, DHTTYPE);

ISR(WDT_vect) { Sleepy::watchdogEvent(); }

void blink(int times) {
  for(int i=0; i<times; i++) {
     digitalWrite(LED_PIN, HIGH);
     delay(5000); 
     digitalWrite(LED_PIN, LOW);
     delay(5000);
  }
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(75); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

static void setPrescaler (uint8_t mode) {
    cli();
    CLKPR = bit(CLKPCE);
    CLKPR = mode;
    sei();
}

void setup () {
  setPrescaler(4); // Reduce clock speed to 1 MHz
  
  // LED
  pinMode(LED_PIN, OUTPUT);
  // button
  pinMode(BUTTON_PIN, INPUT);
  
  // enable pull-up resistor for the button
  digitalWrite(BUTTON_PIN, HIGH);
  
  // init radio
  SPI.begin();
  radio.begin();
  network.begin(90, thisNode);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  
  MCUCR |= _BV(BODS) | _BV(BODSE);          //turn off the brown-out detector  
  
  dht.begin();
}

void loop() {
  // pump network
  network.update();
  setPrescaler(0);  // Reset clock speed to 16 MHz to get a reliable reading from the sensors
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  setPrescaler(4);  // Reduce clock speed back to 1 MHz
  // TODO: Read the internal temperature sensor MCP9700 connected to A3?

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
      Serial.println("Failed to read from DHT");
  } 
  else {
    long voltage = readVcc();

    // pad numbers and convert to string
    char txBuffer[32] = "";
    // txBuffer[0] = nodeId
    // txBuffer[1] = temperature in C
    // txBuffer[2] = humidity
    // txBuffer[3] = Vcc
    
    char tempF[7]; // buffer for temp incl. decimal point & possible minus sign + nul termination
    char rhPct[7]; // buffer for humidity incl. decimal point + nul termination
    
    dtostrf(t, 6, 2, tempF); // Min. 6 chars wide incl. decimal point, 2 digits right of decimal
    dtostrf(h, 6, 2, rhPct);
    
    sprintf(txBuffer,"%d,%s,%s,%d", thisNode, tempF, rhPct, voltage);

    // transmit
    RF24NetworkHeader header(otherNode);
    network.write(header,&txBuffer,sizeof(txBuffer));
//    blink(1);    
    
    // power off sensors
    radio.powerDown();

    // sleep for 30 minutes
    for (byte i = 0; i < 30; ++i) {
        Sleepy::loseSomeTime(60000);
    }

//    Sleepy::loseSomeTime(10000);
   
    // power on radio
    radio.powerUp();
  }
}
