#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

// set pin 4 as the slave select for the digital pot:
const int CS = 10;

uint8_t start_capture = 1;

ArduCAM myCAM(OV2640,CS);

void setup() {
  while (!Serial);

  SPI.setClockDivider(SPI_CLOCK_DIV128);  

  uint8_t vid,pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(921600);
  //Serial.println("ArduCAM Start!"); 

  // set the SPI_CS as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin();
  //myCAM.write_reg(ARDUCHIP_MODE, 0x00);
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);

  if(temp != 0x55)
  {
  	Serial.println("SPI1 interface Error!");
  	while(1);
  }
  
  //Check if the camera module type is OV2640
  //myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);


  if((vid != 0x26) || (pid != 0x42)) {
    Serial.println("Can't find OV2640 module!");
  }


  //Change to JPEG capture mode and initialize the OV5642 module	
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  //myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
  myCAM.clear_fifo_flag();
  //myCAM.write_reg(ARDUCHIP_FRAMES,0x01);
}

void loop() {
  //Serial.println("Waiting");
  delay(5000);	

 
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();	 
  //Start capture
  myCAM.start_capture();

  while(!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)) {
    delay(10);
  }

  //delay(6000);

  read_fifo_burst(myCAM);
  myCAM.clear_fifo_flag();


  delay(60000);

}

void read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp, temp_last;
  uint32_t length = 0;
  length = myCAM.read_fifo_length();
  if (length >= 393216 ) // 384kb
  {
    Serial.println("Over size.");
    return;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println("Size is 0.");
    return;
  }

  //temp = myCAM.read_fifo();

  //length--;
  while ( length-- )
  {
    temp_last = temp;
    temp = myCAM.read_fifo();
    Serial.write(temp);
    Serial.flush();
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      break;
    delayMicroseconds(12);
  }

  //myCAM.CS_HIGH();
}
									    