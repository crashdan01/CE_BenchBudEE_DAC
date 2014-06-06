
/*  

A sketch to control the 8-Bit DAC, MCP4801, on the Contextual Electronics Bench BuddEE board.
If you are using the board that was provided with the class, uncomment the LDAC and SHDN pin values
at the top of the sketch, pins 3 and 2 respectively.  For the custom board designed by Dan, uncomment
the second set of values.

The MCP4801 communicates using SPI, and the board will also modulate the DAC output by applying a PWM
signal on Arduino Digital 10.

MCP4801 Datasheet: http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en547815

*/


#include "SPI.h"                                       // Include the SPI library

// Sets the pin values for the latch and shutdown pins
const int pin_dac_latch_n = 3;                         // Official Board: Latch pin
const int pin_dac_shdn_n  = 2;                         // Official Board: SHDN pin
//const int pin_dac_latch_n = 2;                       // Custom Board: Latch pin
//const int pin_dac_shdn_n  = 3;                       // Custom Board: SHDN pin

// Sets the pin values for Chip Select and PWM.
const int pin_dac_cs_n    = 7;
const int pin_dac_pwm     = 10;

// Set bits for the MCP4801 gain options and chip activate.  These will be passed
// to the chip as part of each SPI command.
const int spi_dac_gain_1    = (1u << 13);              // Gain 1: gain = 1x
const int spi_dac_gain_2    = (0u << 13);              // Gain_2: gain = 2x
const int spi_dac_active    = (1u << 12);              // Enable Vout, pin 8
const int spi_dac_shutdown  = (0u << 12);              // Disable Vout, pin 8 tied internally to 500K Ω

// By the datasheet, the Vref of the MPC4801 is 2.048V.  As it is an 8-Bit DAC, that means 
// each increment results in a change at Vout of 2.048V / 255 = 0.008V.
// For the 10-bit DAC, MCP4811, dac_increment = 1023.
// For the 12-bit DAC, MCP4821, dac_increment = 4095
const int dac_vref          = 2048;
const int dac_increment     = 255;


void dac_setup()                                       // Function to prepare the DAC for SPI communication.
{
    pinMode(pin_dac_cs_n,    OUTPUT);
    pinMode(pin_dac_latch_n, OUTPUT);
    pinMode(pin_dac_shdn_n,  OUTPUT);
    
    digitalWrite(pin_dac_cs_n,    HIGH);
    digitalWrite(pin_dac_latch_n, LOW);
    digitalWrite(pin_dac_shdn_n,  HIGH);
    
    SPI.begin();
}


void dac_set(int value)  // Sets the DAC output using the variable "value" passed from the output_set function.
{
    value = value << 4;  
  
    // Creates the full 16-bit word that will be passed to the DAC.  See datasheet for
    // full register description. In general, this is the command you will want to use.
    
    uint16_t dac_cmd = value | spi_dac_active | spi_dac_gain_1;
    
    // These commands are for reference.  The first can be used to turn DAC output off
    // programmatically, the second tells the DAC to output using a 2x gain.
    //uint16_t dac_cmd = value | spi_dac_shutdown | spi_dac_gain_1;
    //uint16_t dac_cmd = value | spi_dac_active | spi_dac_gain_2;
    
    /*
    Serial.print ("value in Decimal: ");              // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (value);                           // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("value in Binary: ");               // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (value, BIN);                      // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("spi_dac_active in Binary: ");      // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (spi_dac_active, BIN);             // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("spi_dac_gain_1 in Binary: ");      // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (spi_dac_gain_1, BIN);             // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("Fully Assembled: ");               // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print (lowByte(dac_cmd), BIN);             // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (highByte(dac_cmd), BIN);          // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("Actual value sent via SPI: ");     // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (dac_cmd, BIN);                    // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println ("");                              // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    */
    
    // Send the command via SPI.  The DAC register is 16-bits long, but only accepts the command
    // one byte at a time, so after taking chip select LOW, send first 8-bits of dac_cmd then the
    // second 8-bits, then take chip select HIGH to commit the change.
    digitalWrite(pin_dac_cs_n, LOW);
    SPI.transfer(highByte(dac_cmd));
    SPI.transfer(lowByte(dac_cmd));
    digitalWrite(pin_dac_cs_n, HIGH);
}


int output_set(int milli_amps)                        // Converts the value of mA set from the main loop into the value to be sent to the DAC.
{
    long dac_value;
    
    // The datasheet formula to determine Vout, rewritten using this sketch's variables is
    // "milli_amps = dac_vref * dac_value / dac_increment".  We shuffle the formula to find dac_value...

    dac_value = (long(milli_amps) * long(dac_increment)) / long(dac_vref);
    
    /*
    Serial.print ("milli_amps value: ");            // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (milli_amps);                    // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.print ("dac_increment value: ");         // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (dac_increment);                 // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT    
    Serial.print ("dac_vref value: ");              // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (dac_vref);                      // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT    
    Serial.print ("dac_value is: ");                // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    Serial.println (dac_value);                     // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
    */
    
    dac_set(dac_value);                             // Send the value to the DAC

    return 0;
}

void setup()
{
    
    dac_setup();
    pinMode      (pin_dac_pwm, OUTPUT);
    digitalWrite (pin_dac_pwm, LOW);
    
    // Serial.begin(9600);                             // UNCOMMENT FOR DEBUG SERIAL MONITOR OUTPUT
}

void loop()
{

    analogWrite(pin_dac_pwm, 0);                    // Sets the PWM output to zero.

    output_set(100);                                // Send the desired output in milliamps
    delay (100); 
}
