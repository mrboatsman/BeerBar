#ifndef Payment_h
#define Payment_h
#include <SoftwareSerial.h>

class Payment
{
	public:
		Payment();
		bool coinAcceptor();
		void acceptCoins(bool set);
		int getFiatBalance();
		void resetFiatBalance();
		char coinsStr[10];
  	private:
  		int fiatBalance;
    	SoftwareSerial acceptorSerial;
};

#endif