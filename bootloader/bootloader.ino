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

#define STM Serial1   // serial port connected to STM32

byte recv = 0x00;

//----------------------------------------------------------------
// HELPER FUNCTIONS
//----------------------------------------------------------------

/*
 * stm_send, send byte over STM serial connection
 * return: none
 * param: byte cmd, byte to send
 */
inline void stm_send(byte cmd)
{
  STM.write(cmd);
}

//----------------------------------------------------------------

/*
 * stm_init, init STM USART connection
 * return: bool, true if successful
 * param: none
 */
bool stm_init()
{
  // STM waits for 0x7F to init USART
  stm_send(0x7F);
  while(!STM.available());  // wait for ACK

  return ( (STM.read() == 0x79) ? true : false);
}

//----------------------------------------------------------------
// SETUP
//----------------------------------------------------------------

void setup() {
  // Init serial
  Serial.begin(115200);           // debug console
  STM.begin(57600, SERIAL_8E1);   // STM UART connection // 8bits, even parity, 1 stop
  
  delayMicroseconds(1227);        // wait for STM bootloader to load
  
  if(stm_init() == true)
  {
    Serial.println("INIT SUCCESSFULL");
  }
  else 
  {
    Serial.println("INIT FAILED");
    while(1);
  }
  
}

//----------------------------------------------------------------
// LOOP
//----------------------------------------------------------------

void loop() {  

}
