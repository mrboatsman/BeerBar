/*
Class for one way commincation from the Coin Acceptor
Using software serial because hardware serial interface
is used by the RFID reader.
*/

#include "Payment.h"
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "defines.h"
class SoftwareSerial;

Payment::Payment(void) : acceptorSerial(SOFTRX, SOFTTX)
{
	acceptorSerial.begin(4800);
	pinMode(RELEASECOINSPIN, OUTPUT);
	
} 

void Payment::acceptCoins(bool set)
{
	if(set)
	{
		analogWrite(RELEASECOINSPIN, 0);			//Out of Digital I/O
	}else{
		analogWrite(RELEASECOINSPIN, 255);
	}
}

bool Payment::coinAcceptor()
{
	if (acceptorSerial.available()){
		int serValue = acceptorSerial.read();
		if(serValue !=255){
			fiatBalance = fiatBalance+serValue;
		}
		return true;
	}else{
		return false;
	}
}
int Payment::getFiatBalance()
{
	return fiatBalance;
}
void Payment::resetFiatBalance()
{
	fiatBalance = 0;
}
