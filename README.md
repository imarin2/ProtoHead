# ProtoHead

This sketch uses the Arduino Wire library to make a flexible I2C slave implementation.

The status of this project is ALPHA/UNTESTED. Please report any issue.

Though the code is pretty general (and can be reused for any other project), it is intended to be run in an arduino within a head of the FABtotum Personal Fabricator.

It supports single and multibyte read/write cycles.

The code MUST include an IDENTIFICATION REGISTER (comprising at least one entry (byte, int,...) to identify the device). This is READ ONLY and does not change (unless you re-flash the device).

The code MAY include:
- A STATUS REGISTER: READ ONLY, but may change depending on the device status
- A DATA REGISTER: READ/WRITE, can be read/written by the master or the slave
- A CONFIGURATION REGISTER: R/W, but includes a procedure to potentially invalidate any DATA that may not be adequate after the configuration change.


HOW TO USE IT?

1. Well decide how many entries you need in each register and the type
2. Modify the configuration to match your register map:

#define IDENTIFICATION_REGISTER_OFFSET   0x0
#define STATUS_REGISTER_OFFSET 0x8 // First register of the status register (If no status register, use the last register of the Identification region)
#define DATA_REGISTER_OFFSET 0x8 // First register of the data register (If no data register, use the last register of the status region)
#define CONFIGURATION_REGISTER_OFFSET 0x8 // First register of the configuration register (If no configuration register, use the last register of the data region)
#define LAST_REGISTER_OFFSET 0x8 

This configuration has only an IDENTIFICATION register with 8 bytes 0x0-0x7.
The status register would start in 0x8, but does not contain any element because the DATA register would also start in 0x8, which also does not contain any element because the CONFIGURATION register
would also start in 0x8, which happens to be where the LAST_REGISTER (the next valid byte) would start.

If you would like to have a CONFIGURATION register with 2 bytes, then you could use this config:
#define IDENTIFICATION_REGISTER_OFFSET   0x0
#define STATUS_REGISTER_OFFSET 0x8 // First register of the status register (If no status register, use the last register of the Identification region)
#define DATA_REGISTER_OFFSET 0x8 // First register of the data register (If no data register, use the last register of the status region)
#define CONFIGURATION_REGISTER_OFFSET 0x8 // First register of the configuration register (If no configuration register, use the last register of the data region)
#define LAST_REGISTER_OFFSET 0x10 // Last valid register of the I2C device.

If, in addition, you would like to have a STATUS register with 3 bytes, then you could use this config:
#define IDENTIFICATION_REGISTER_OFFSET   0x0
#define STATUS_REGISTER_OFFSET 0x8 // First register of the status register (If no status register, use the last register of the Identification region)
#define DATA_REGISTER_OFFSET 0x11 // First register of the data register (If no data register, use the last register of the status region)
#define CONFIGURATION_REGISTER_OFFSET 0x11 // First register of the configuration register (If no configuration register, use the last register of the data region)
#define LAST_REGISTER_OFFSET 0x13 // Last valid register of the I2C device.

3. Modify the data structure to indicate the data and its values (it may have int, long or any other data type, the code sends the corresponding bytes for you)

struct RegisterRegionStruct {
unsigned char HeadProductID;  // 0x0
unsigned char SerialID[6];    // 0x1-0x6
unsigned char SerialCRC[1];   // 0x7
// insert here status, data and configuration registers
};

4. Define the values of the IDENTIFICATION register

// identification register information
#define SERIAL_N_FAM_DEV_CODE 0xFF // 0xFF for development product
#define SERIAL_N_0 0xFF            // 0xFF for development product
#define SERIAL_N_1 0xFF            // 0xFF for development product
#define SERIAL_N_2 0xFF            // 0xFF for development product
#define SERIAL_N_3 0xFF            // 0xFF for development product
#define SERIAL_N_4 0xFF            // 0xFF for development product
#define SERIAL_N_5 0xFF            // 0xFF for development product
#define SERIAL_N_CRC 0xFF          // The CRC, we did not agree to a CRC yet and FABtotum does not check it so 0xFF for development

5. Define the SLAVE ADDRESS

/*** FABTOTUM STANDARDIZED VALUES - DO NOT CHANGE UNLESS FABLIN CHANGES THEM - ***/

#define SERIAL_ID_ADDR                   80 

6. Load the IDENTIFICATION register in setup():

  // Initialize identification register
  registers.data.HeadProductID = SERIAL_N_FAM_DEV_CODE;
  registers.data.SerialID[0] = SERIAL_N_0;  
  registers.data.SerialID[1] = SERIAL_N_1;
  registers.data.SerialID[2] = SERIAL_N_2;
  registers.data.SerialID[3] = SERIAL_N_3;
  registers.data.SerialID[4] = SERIAL_N_4;
  registers.data.SerialID[5] = SERIAL_N_5;
  registers.data.SerialCRC[0] = SERIAL_N_CRC;

7. If needed, modify the LOOP to ensure that the status/data/configuration values have the values according to your application. Not needed if only an INDENTIFICATION register is used.

8. If DATA or CONFIGURATION registers are defined, then modify these functions according to your application requirements (what to do when the master writes to DATA or CONFIGURATION registers)

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

You are done!!
