#include "defines.h"
#include "RfidReader.h"
#include "Arduino.h"
#include <EEPROM.h>

RfidReader::RfidReader(void)
{
	pinMode(RFIDREADERENABLEPIN,OUTPUT);
	digitalWrite(RFIDREADERENABLEPIN, HIGH);     // Disable the RFID reader
}

bool RfidReader::scan(void)
{
	digitalWrite(RFIDREADERENABLEPIN, LOW);     // Activate the RFID reader
	while(1){
		if(Serial.available() > 0) {
			if((val = Serial.read()) == 10){ 			// check for header 
				bytesread = 0; 
				while(bytesread<10){           			 // read 10 digit code 
					if( Serial.available() > 0){ 
						val = Serial.read(); 
          				if((val == 10)||(val == 13)) { 	 // if header or stop bytes before the 10 digit reading 
           					break;                       // stop reading 
          				}
          				tag[bytesread] = val;
          				bytesread++;                   	// ready to read next digit  
				 	}
				}
				if(bytesread == 10 && compareCurrentKey() && !dontSpam) {              // if 10 digit read is complete 
					tag[10] = '\0';			//end the char
		         	dontSpam = 1;
		         	Serial.println("dontSpam");
		         	Serial.end();		//End serial to empty buffers
		         	return true;
		          	break;
        		}   
			}
			bytesread = 0; 
      		digitalWrite(RFIDREADERENABLEPIN, HIGH);      // deactivate the RFID reader for a moment so it will not flood
      		delay(1000);        		// wait for a bit 
      		digitalWrite(RFIDREADERENABLEPIN, LOW);       // Activate the RFID reader
		}
		if(!compareCurrentKey()){
	    	storeCurrentKey();
	    	dontSpam = 0;
	    }
	    return false;

	}
	
}


void RfidReader::resetRfid()
{
	Serial.begin(2400);		//Start up serial again
	dontSpam = 0;
	digitalWrite(RFIDREADERENABLEPIN, HIGH); 
	delay(1000); 
	digitalWrite(RFIDREADERENABLEPIN, LOW);
	for(int i=0; i<10; i++) {
      EEPROM.write(i, 0);
   }
}
/**
 *  Write the key in 'tag' array into the EEPROM
 */
void RfidReader::storeCurrentKey()
{
   for(int i=0; i<10; i++) {
      EEPROM.write(i, tag[i]);
   }
}

/**
 * Compare the key currently in 'tag' array with the one we have stored
 */
bool RfidReader::compareCurrentKey()
{
   for(int i=0; i<10; i++) {
      if(EEPROM.read(i) != tag[i]) return false;  // not a match  
   }
   return true;  //if we got here, it must be the same key that is stored
}

