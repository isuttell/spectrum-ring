#define N_PIXELS  24 // Total Number of Pixels
#define LED_PIN   9

#define LIN_OUT 1 // use the log output function
#define FHT_N 64 // set to 256 point fht

const uint8_t SIGNAL_MIN_CUTOFF = 80; // Signal must be this strong

#include <Adafruit_NeoPixel.h>
#include <avr/wdt.h>
#include <FHT.h> // include the library

Adafruit_NeoPixel
	strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t findMax(uint8_t arr[], int n);
uint16_t findMax(uint16_t arr[], int n);

int binValue;
byte r;
byte g;
byte b;

void setup()
{
	wdt_reset();
	// Serial.begin(115200); // use the serial port

	wdt_enable(WDTO_8S);

	strip.begin();

	soundVisualisation();
}


void loop(){}


void soundVisualisation() {

	ADCSRA = 0xe5; // set the adc to free running mode
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((13 >> 3) & 0x01) << MUX5);
	ADMUX = 0x40 | (13 & 0x7); // use adc13
	DIDR2 = 13; // turn off the digital input for adc13

	// Use out own while loop to reduce timing jutter
	while(1) {
	TIMSK0 = 0; // turn off timer0 for lower jitter
	for (int i = 0 ; i < FHT_N ; i++) {
		while(!(ADCSRA & 0x10)); // wait for adc to be ready
		ADCSRA = 0xf5; // restart adc
		byte m = ADCL; // fetch adc data
		byte j = ADCH;
		int k = (j << 8) | m; // form into an int
		k -= 0x0200; // form into a signed int
		k <<= 6; // form into a 16b signed int
		fht_input[i] = k; // put real data into bins
	}
	TIMSK0 = 1; // turn on timer0 so we can use millis() again

	// process data
	fht_window(); // window the data for better frequency response
	fht_reorder(); // reorder the data before doing the fht
	fht_run(); // process the data in the fht
	fht_mag_lin(); // take the output of the fht


	// int maxBin;
	// maxBin = findMax(fht_lin_out, FHT_N/2);
	// Serial.print("bin: ");
	// Serial.print(maxBin);
 //  Serial.print("\t");

  for(byte i = 0; i < N_PIXELS; i++) {
      binValue = fht_lin_out[i+1] - SIGNAL_MIN_CUTOFF;
      if(binValue < 0) {
        binValue = 0;
      }

      if(binValue > 0 && binValue < 600) {
        b = 255 * (binValue / 1200.0);
      } else {
        b = 0;
      }
      if(binValue > 300 && binValue < 900) {
        r = 255 * (binValue / 1200.0);
      } else {
        r = 0;
      }
      if(binValue > 600) {
        g = 255 * (binValue / 1200.0);
      } else {
        g = 0;
      }
      strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();

	// Keep the color the same for 33ms (30fps) or the LEDs can flicker too quickly
	delay(33);
	}
}

uint16_t findMax(uint8_t arr[], int n)
{
  uint16_t m = 0;
  uint8_t val = 0;
  for (int i = 0; i < n; i++)
  {
    if (arr[i] > val)
    {
      m = i;
      val = arr[i];
    }
  }
  return m;
}

uint16_t findMax(uint16_t arr[], int n)
{
  uint16_t m = 0;
  uint16_t val = 0;
  for (int i = 0; i < n; i++)
  {
    if (arr[i] > val)
    {
      m = i;
      val = arr[i];
    }
  }
  return m;
}