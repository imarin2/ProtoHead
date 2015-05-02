//********************************//
//* FABtotum ProtoHEAD I2C slave *//

#include <Wire.h>
#include <util/atomic.h>

/*** HEAD SPECIFIC CONFIGURATION ***/

#define STATUS_REGISTER_OFFSET 0x8 // First register of the status register (If no status register, use the last register of the Identification region)
#define DATA_REGISTER_OFFSET 0x8 // First register of the data register (If no data register, use the last register of the status region)
#define CONFIGURATION_REGISTER_OFFSET 0x8 // First register of the configuration register (If no configuration register, use the last register of the data region)
#define LAST_REGISTER_OFFSET 0x8 // Last valid register of the I2C device.

// Modify this register structure in accordance with the OFFSETS above
struct RegisterRegionStruct {
unsigned char HeadProductID;  // 0x0
unsigned char SerialID[6];    // 0x1-0x6
unsigned char SerialCRC[1];   // 0x7
// insert here status, data and configuration registers
};

// identification register information
#define SERIAL_N_FAM_DEV_CODE 0xFF // 0xFF for development product
#define SERIAL_N_0 0xFF            // 0xFF for development product
#define SERIAL_N_1 0xFF            // 0xFF for development product
#define SERIAL_N_2 0xFF            // 0xFF for development product
#define SERIAL_N_3 0xFF            // 0xFF for development product
#define SERIAL_N_4 0xFF            // 0xFF for development product
#define SERIAL_N_5 0xFF            // 0xFF for development product
#define SERIAL_N_CRC 0xFF          // The CRC, we did not agree to a CRC yet and FABtotum does not check it so 0xFF for development


/*** FABTOTUM STANDARDIZED VALUES - DO NOT CHANGE UNLESS FABLIN CHANGES THEM - ***/

#define SERIAL_ID_ADDR                   80 
#define IDENTIFICATION_REGISTER_OFFSET   0x0 


/******** CONFIGURATION ENDS HERE *********/

// Calculate the length of the register areas
#define IDENTIFICATION_REGISTER_LENGTH (STATUS_REGISTER_OFFSET-IDENTIFICATION_REGISTER_OFFSET)
#define STATUS_REGISTER_LENGTH (DATA_REGISTER_OFFSET-STATUS_REGISTER_OFFSET)
#define DATA_REGISTER_LENGTH (CONFIGURATION_REGISTER_OFFSET-DATA_REGISTER_OFFSET)
#define CONFIGURATION_REGISTER_LENGTH (LAST_REGISTER_OFFSET-CONFIGURATION_REGISTER_OFFSET)
#define REGISTER_LENGTH (LAST_REGISTER_OFFSET-IDENTIFICATION_REGISTER_OFFSET)

// This allows support for multi-byte writes
#define RECEPTION_BUFFER_LENGTH DATA_REGISTER_LENGTH+CONFIGURATION_REGISTER_LENGTH+1 // Number of bytes expected from Master (+1 is the address byte)

union RegisterUnion {
struct RegisterRegionStruct data;
unsigned char byteArray[REGISTER_LENGTH];
};

// I2C variables 
union RegisterUnion registers;
byte receivedBuffer[RECEPTION_BUFFER_LENGTH];

unsigned char registerChangedStart=0x0;         // First register that changed on Master action
unsigned char registerChangedEnd=0x0;           // Last register that changed on Master action

bool informationValid=false;                // Indicates whether the current register information is in accordance with the latest configuration or not

void setup()
{
  // Initialize identification register
  registers.data.HeadProductID = SERIAL_N_FAM_DEV_CODE;
  registers.data.SerialID[0] = SERIAL_N_0;  
  registers.data.SerialID[1] = SERIAL_N_1;
  registers.data.SerialID[2] = SERIAL_N_2;
  registers.data.SerialID[3] = SERIAL_N_3;
  registers.data.SerialID[4] = SERIAL_N_4;
  registers.data.SerialID[5] = SERIAL_N_5;
  registers.data.SerialCRC[0] = SERIAL_N_CRC;
    
  Wire.begin(SERIAL_ID_ADDR); 

  Wire.onRequest(requestEvent);

  Wire.onReceive(receiveEvent);
  
}


void loop()
{
  // Check whether configuration were changed by master
  if(registerChangedStart!=0x0 && CONFIGURATION_REGISTER_LENGTH>0 && registerChangedEnd>=CONFIGURATION_REGISTER_OFFSET){
    informationValid = false;  //let the master know we don’t have any post correction data yet
    onConfigChanged();
    return; 
  }
  
  // Check whether data were changed by master
  if(registerChangedStart!=0x0 && DATA_REGISTER_LENGTH>0 && registerChangedStart>=DATA_REGISTER_OFFSET)
    onDataChanged();    
  
/*
  // Use atomic blocks to change data registers to avoid corruption when interrupted while reading or writing
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    
  }
*/

  informationValid = true;
}


void onConfigChanged() 
{

    /* INSERT HERE THE CODE NEEDED BY THIS HEAD ON A CONFIGURATION CHANGE */


  
    /* STOP INSERTING HERE */
    // Look if data was also changed
    if(registerChangedStart!=0x0 && DATA_REGISTER_LENGTH>0 && registerChangedStart>=DATA_REGISTER_OFFSET) {
       registerChangedEnd=CONFIGURATION_REGISTER_OFFSET-1; // allow to trigger onDataChanged in the next loop cycle
    }
    else{
    registerChangedStart=0x0;
    registerChangedEnd=0x0;
    }
  
}

void onDataChanged() 
{
    /* INSERT HERE THE CODE NEEDED BY THIS HEAD ON A DATA CHANGE */



    /* STOP INSERTING HERE */
    registerChangedStart=0x0;
    registerChangedEnd=0x0;  
}

void requestEvent()
{
  // This sends all the bytes after the direction requested, if the master expects less information, he will send NACK to stop the transfer
  // This allows for (faster) multibyte reads.
  Wire.write(registers.byteArray + receivedBuffer[0], LAST_REGISTER_OFFSET-receivedBuffer[0]);
}

 

void receiveEvent(int bytesReceived)
{
  unsigned int i;
  
  for(i=0; i < bytesReceived; i++){
    if(i < RECEPTION_BUFFER_LENGTH)
      receivedBuffer[i] = Wire.read();
    else
      Wire.read();  // dispose bytes that we do not expect
  }

  if(bytesReceived == 1){ // Master is setting the address to read from the I2C slave

    if(receivedBuffer[0]>=REGISTER_LENGTH) // address out of register
      receivedBuffer[0]=0x0;
    
    return; 

  }
  
  if(receivedBuffer[0] < DATA_REGISTER_OFFSET || receivedBuffer[0] >= REGISTER_LENGTH) // master is trying to write to a read only register or outside the register
    return;
  
  
  registerChangedStart=receivedBuffer[0];
  registerChangedEnd=registerChangedStart;
  
  registers.byteArray[registerChangedStart]=receivedBuffer[1];
  
  for(i=2;i < bytesReceived;i++){
    registers.byteArray[registerChangedStart+i-1]=receivedBuffer[i];
    registerChangedEnd++;
  }
  
}
