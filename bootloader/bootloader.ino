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
 * stm_read, blocks until byte recv from STM
 * return: byte, value recv from STM
 * param: none
 */
inline byte stm_read()
{
  while(!STM.available());  // wait to recv byte
  return STM.read();
}

//----------------------------------------------------------------

/*
 * stmCmdGeneric, executes chosen generic command
 * return: bool, true if successful
 *         arr, values returned by command
 *         len, lengh of arr
 * param: byte   cmd, command code
 *        byte* arr, array to hold returned values
 *        int*  len, max length of arr
 */
bool stmCmdGeneric(byte cmd, byte* arr, int* len)
{
  stm_send(cmd);         // send command
  stm_send(cmd ^ 0xFF);  // send checksum

  if(stm_read() != 0x79)
  {
    // ACKNOWLEDGE FAILED
    return false;
  }
  else
  {
    int recv_len = stm_read() + 1;      // number of bytes in response
    if(recv_len > *len) recv_len = *len;  // limit recv_len to size of arr

    for(int i = 0; i < recv_len; i++)
    {
      arr[i] = stm_read();    // load response into arr
    }
    
    *len = recv_len;   // update arr length
  }
}

//----------------------------------------------------------------

/*
 * printArr, prints array to console in HEX format
 * return: none
 * param: byte* arr, array to be printed
 *        int   len, length of array
 */
void printArr(byte* arr, int len)
{
  for(int i = 0; i < len; i++)
  {
    Serial.print(arr[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
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
  byte arr[512] = { 0 };
  int len = 512;
  
  // issue Get command
  if( stmCmdGeneric(0x00, arr, &len) )
  {
    printArr(arr, len);
  }
  
  while(1);
}
