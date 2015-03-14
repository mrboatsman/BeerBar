#ifndef Bitcoin_h
#define Bitcoin_h 
#include <Ethernet.h>

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

class Bitcoin
{
	public:
		Bitcoin();
		double getBalance(char *tag);
		float getBeerPriceperCl(void);
		char* getBeerPrice(void);
		void payment(char *tag, float amount);
		bool depositFiat(char *tag, int fiatAmount);
		void displayQrCode(char *tag);
		void displayBalance(char *tag);
	private:
		int BTCbalance;
		bool startRead;
		bool startReadCoins;
		char coinsStr[10];
		int permChangeBalance;
		int webStrPos;
		int dispPosX;
		int dispPosY;
		int dispOffcetX;
		int dispOffcetY;
		byte server[];
		EthernetClient client;             
		Adafruit_PCD8544 display;
};

#endif
