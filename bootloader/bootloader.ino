// M. Kaan Tasbas | mktasbas@gmail.com
// March 2019

/*
 * This Arduino Due sketch will burn a bootloader onto an STM32F103
 * to allow programming via USB port using the Arduino IDE
 * 
 * Connection guide:
 * DUE          STM32
 * 18 (TX1) <-> A10 (USART1 RX)   USE 1K PULL-UP IN BETWEEN
 * 19 (RX1) <-> A09 (USART1 TX)   USE 1K PULL-UP IN BETWEEN
 * 3.3V     <-> 3.3V
 * GND      <-> GND
 * 
 * Preperation:
 * Place STM32 BOOT0 jumper to 1 and BOOT1 jumper to 0
 * 
 */

void setup() {
  

}

void loop() {
  
}
