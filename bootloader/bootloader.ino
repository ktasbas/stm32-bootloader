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

#define ACK 0x79      // acknowledge byte from STM

byte global_arr[512] = { 0 };
int global_len = 512;

//----------------------------------------------------------------
// HELPER FUNCTIONS
//----------------------------------------------------------------

/*----------------------------------------------------------------

 * stmSend, send byte over STM serial connection
 * return: none
 * param: byte cmd, byte to send
 */
inline void stmSend(byte cmd)
{
  STM.write(cmd);
}

/*----------------------------------------------------------------
 * stmRead, blocks until byte recv from STM
 * return: byte, value recv from STM
 * param: none
 */
inline byte stmRead()
{
  while(!STM.available());  // wait to recv byte
  return STM.read();
}

/*----------------------------------------------------------------
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
  stmSend(cmd);         // send command
  stmSend(cmd ^ 0xFF);  // send checksum

  if(stmRead() != ACK)
  {
    // ACKNOWLEDGE FAILED
    return false;
  }
  else
  {
    int recv_len = stmRead() + 1;      // number of bytes in response
    if(recv_len > *len) recv_len = *len;  // limit recv_len to size of arr

    for(int i = 0; i < recv_len; i++)
    {
      arr[i] = stmRead();    // load response into arr
    }
    
    *len = recv_len;   // update arr length
  }
}

/*----------------------------------------------------------------
 * printArr, prints array to console in HEX format
 * return: none
 * param: array of any data type
 */
template< typename T, size_t len > void printArr(const T (&arr)[len])
{
  for(size_t i = 0; i < len; i++)
  {
    Serial.print(arr[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

/*----------------------------------------------------------------
 * stm_init, init STM USART connection
 * return: bool, true if successful
 * param: none
 */
bool stm_init()
{
  // STM waits for 0x7F to init USART
  stmSend(0x7F);
  byte recv = stmRead();
  if(recv != ACK) return false;   // ACK FAILED

  // get chip ID
  if( stmCmdGeneric(0x02, global_arr, &global_len) )
  {
    byte msb = global_arr[0];
    byte lsb = global_arr[1];
    int chip_id = (msb << 8) + (lsb << 0);
    
    if(chip_id == 0x410) return true;
  } 
  return false;
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
    Serial.println("INIT SUCCESSFUL");
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
  // issue Get command

  
  while(1);
}
