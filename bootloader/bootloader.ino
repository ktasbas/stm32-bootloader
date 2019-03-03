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
 * stmReadMemory, reads the flash memory of the STM32
 * return: bool, true if successful
 *         arr, memory values returned from STM32
 *         len, number of bytes returned
 * param: int   addr, start address of flash memory to read
 *        byte* arr, array to store returned values
 *        int*  len, number of bytes to read (max 256)
 * 
 */
bool stmReadMemory(int addr, byte* arr, int* len)
{
  stmFlushRx();
  
  stmSend(0x11);          // send read command
  stmSend(0x11 ^ 0xFF);   // send checksum
  
  if(stmRead() != ACK) return false;  // wait for ACK
  
  byte byte3 = (addr >> 0) & 0xFF;    // addr lsb
  byte byte2 = (addr >> 8) & 0xFF;
  byte byte1 = (addr >> 16) & 0xFF;
  byte byte0 = (addr >> 24) & 0xFF;   // addr msb
  byte crc = byte0 ^ byte1 ^ byte2 ^ byte3;  // checksum

  stmSend(byte0);
  stmSend(byte1);
  stmSend(byte2);
  stmSend(byte3);
  stmSend(crc);

  if(stmRead() != ACK) return false;  // wait for ACK

  byte len_byte = static_cast<byte>(*len) - 1;
  stmSend(len_byte);    // length of read
  stmSend(len_byte ^ 0xFF);   // checksum

  if(stmRead() != ACK) return false;  // wait for ACK

  for(size_t i = 0; i <= len_byte; i++)
  {
    arr[i] = stmRead();
  }
  *len = len_byte;    // update length of array
  return true;
}

/*----------------------------------------------------------------
 * stmEraseMemory, performs a global erase of all flash memory
 * return: bool, true if successful
 * param: none
 */
bool stmEraseMemory(void)
{
  stmFlushRx();
  
  Serial.println("Sending erase command...");
  stmSend(0x43);          // send read command
  stmSend(0x43 ^ 0xFF);   // send checksum

  if(stmRead() != ACK) return false;  // wait for ACK

  Serial.println("Sending global code...");
  stmSend(0xFF);          // send global erase code
  stmSend(0xFF ^ 0xFF);   // send checksum

  if(stmRead() != ACK) return false;  // wait for ACK

  return true;
}

/*----------------------------------------------------------------
 * stmWriteMemory, writes array to STM32 flash memory
 * return: true if successful
 * param: int   addr, address to begin writing
 *        byte* arr, values to be written
 *        int   len, length of arr
 */
bool stmWriteMemory(int addr, byte* arr, int len)
{
  if (len > 256) return false;  // max write length

  byte len_byte = static_cast<byte>(len) - 1;   // number of bytes to write
  byte arr_crc = len_byte;    // XOR(arr, len) checksum for later
  
  for(size_t i = 0; i < len; i++)
  {
    arr_crc ^= arr[i];  // calculate XOR(arr, len)
  }
  
  stmFlushRx();   // clear DUE RX buffer from STM32
  
  stmSend(0x31);          // send write command
  stmSend(0x31 ^ 0xFF);   // send checksum

  if(stmRead() != ACK) return false;  // wait for ACK

  byte byte3 = (addr >> 0) & 0xFF;    // addr lsb
  byte byte2 = (addr >> 8) & 0xFF;
  byte byte1 = (addr >> 16) & 0xFF;
  byte byte0 = (addr >> 24) & 0xFF;   // addr msb
  byte crc = byte0 ^ byte1 ^ byte2 ^ byte3;  // checksum

  stmSend(byte0);
  stmSend(byte1);
  stmSend(byte2);
  stmSend(byte3);
  stmSend(crc);

  if(stmRead() != ACK) return false;  // wait for ACK

  stmSend(len_byte);    // length of write
  
  for(size_t i = 0; i < len; i++)
  {
    stmSend(arr[i]);  // send data bytes
  }

  stmSend(arr_crc);   // send checksum
  
  if(stmRead() != ACK) return false;  // wait for ACK
  
  return true;
}

/*----------------------------------------------------------------
 * stmFlushRx, clears DUEs RX buffer from STM32
 * return: none
 * param: none
 */
void stmFlushRx(void)
{
  while(STM.available()) STM.read();  // clear DUEs RX buffer
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
  
/*----------------------------------------------------------------
 * printArr, prints array to console in HEX format
 * return: none
 * param: byte* arr, array to be printed
 *        int   len, length of array
 *
void printArr(byte* arr, int len)
{
  for(int i = 0; i < len; i++)
  {
    Serial.print(arr[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
*/

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
  byte arr[256] = { 0 }, write_arr[256] = { 0 };
  int len = 256, write_len = 256;
  int addr = 0x08000000;
  
  if(stmReadMemory(addr, arr, &len)) printArr(arr);
  else
  {
    Serial.println("READ FAILED");
    while(1);
  }

  if(stmWriteMemory(addr, write_arr, write_len))
  {
    Serial.println("MEMORY WRITTEN");
  }
  else
  {
    Serial.println("WRITE FAILED");
    while(1);
  }

  if(stmReadMemory(addr, arr, &len)) printArr(arr);
  else
  {
    Serial.println("READ FAILED");
    while(1);
  }
  
  while(1);
}
