/*
Created by: Anders Lindén - anderslinden.se
BeerBot, for good beverage with with leverage in mind.

Pay with Bitcoins and/or with fiat coins, every user
recive a beer account with unique Bitcoin address for deposit and beer only withdraw. 

Payment with fiat need bitcoins in the "Bank" account on the server.

Authorization with RFID. 

Hardware: Arduino Ethernet
          RFID reader: Parallax RFID Card Reader - Serial (#28140)
          LCD Display: Adafruit Nokia 5110/3310 Monochrome LCD (PRODUCT ID: 338)
          Solenoid Valve: Plastic Water Solenoid Valve - 12V - 1/2" Nominal
          Flow meter: Liquid Flow Meter - Plastic 1/2" NPS Threaded
          Fiat Coint Acceptor: DG600F Coin Acceptor
          Relay: HFD27/005-H(051)
*/

#include "defines.h"
#include <Ethernet.h>
#include <TimerOne.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include "RfidReader.h"
#include "Payment.h"
#include "Bitcoin.h"

//Enumurates for state machine
enum enumState{
  welcomeScreen,
  rfidScan,
  paymentScreen,
  beverageFlow,
  accountReceipt
};

//Start with welcome screen
enumState state = welcomeScreen;

Adafruit_PCD8544 display = Adafruit_PCD8544(LCDSCLK, LCDDIN, LCDDC, LCDCS, LCDRST);

RfidReader rfidread = RfidReader();
Payment payment = Payment();
Bitcoin bitcoin = Bitcoin();



// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;

uint16_t tmpPulse;

bool balanceShowed = false;
bool qrCodeShowed = false;
bool flowShowed = false;
float pricePerCl;
bool flowBalance = false;
double balance;
float amount;
float liters;
int showRecipt2Isrs;
int showNoBalanceLimit;
bool doPaymentDone = false;
bool reciptShowed = false;

// Interrupt is called once a millisecond, looks for any pulses from the sensor!

SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;

    flowShowed = false;
    if(state == paymentScreen){
      state = beverageFlow;
    }
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}


void timerIsr()
{
  if(state == accountReceipt)
    {
      if(showRecipt2Isrs > 1)
      {
        state = welcomeScreen;
        showRecipt2Isrs == 0;
      }
      showRecipt2Isrs++;
    }else if(state == beverageFlow){
    if(tmpPulse == pulses){
      rfidread.resetRfid();
      state = accountReceipt;
      showRecipt2Isrs = 0;
    }
    tmpPulse = pulses;

    if(balance-amount <= 0)   //In a connected system this is useless because the valve is closed which will make the flowmeeter stand at zero
     {
       rfidread.resetRfid();
       state = accountReceipt;
     } 
  }else if(state == paymentScreen){
      //bitcoin.displayBalance(rfidread.tag);
      balanceShowed = false;
      showNoBalanceLimit++;
  }
}

void setup()
{

  //Don't accept fiat during boot up
  pinMode(RELEASECOINSPIN, OUTPUT);
  analogWrite(RELEASECOINSPIN, 255);
  pinMode(VALVERELAYPIN, OUTPUT);
  analogWrite(VALVERELAYPIN, 0); //close valve
  // Enter a MAC address for your controller below.
  // Newer Ethernet shields have a MAC address printed on a sticker on the shield
  byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x60, 0xDB };

  // Set the static IP address to use if the DHCP fails to assign
  IPAddress ip(192,168,1,177);

  if (Ethernet.begin(mac) == 0) {
    //Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  delay(1000);  // give the Ethernet shield a second to initialize:
  
  pinMode(FLOWSENSORPIN, INPUT);
  pinMode(A0,INPUT);

  useInterrupt(true);
  //Initiate the display
  display.begin();
  display.setContrast(40);
  Timer1.initialize(5000000);   // set a timer of length 5 seconds
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  // RFID reader SOUT pin connected to Serial RX pin at 2400bps 
  Serial.begin(2400); 
  rfidread.resetRfid();
}

void loop()
{
  switch (state)
  {
    case welcomeScreen:
      pricePerCl = bitcoin.getBeerPriceperCl();
      analogWrite(VALVERELAYPIN, 0); //close valve
      pulses = 0; // reset pulses from flow meeter
      payment.acceptCoins(false);
      doPaymentDone = false;
      balanceShowed = false;
      qrCodeShowed = false;
      showNoBalanceLimit = 0;
      reciptShowed = false;
      flowBalance = false;

      rfidread.resetRfid();
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setCursor(0,15);
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setCursor(0,2);
      display.println("Ready to scan!");
      display.display();
      display.setCursor(8,15);
      display.println("Beer price:");
      display.display();
      display.setCursor(30,25);
      display.println(bitcoin.getBeerPrice());
      display.display();
      display.setCursor(30,35);
      display.println("mBTC/L");
      display.display();
      state = rfidScan;
    break;
    case rfidScan:
      if(rfidread.scan()){
        state = paymentScreen;
        display.clearDisplay();
      }
    break;
    case paymentScreen:      
      payment.acceptCoins(true);

      if(showNoBalanceLimit > 5)
      {
        state = welcomeScreen;
      }
      if(analogRead(A0) > 1000){
         state = welcomeScreen;
      }
      
      if(payment.coinAcceptor()){
        if(payment.getFiatBalance() > 0){
          if(bitcoin.depositFiat(rfidread.tag, payment.getFiatBalance())){
            payment.resetFiatBalance();
            balanceShowed = false;
          }
        }
      }


      if(!balanceShowed)
      {
        balanceShowed = true;
        bitcoin.displayBalance(rfidread.tag);
        display.display();
        if(bitcoin.getBalance(rfidread.tag) > 0){
          analogWrite(VALVERELAYPIN, 255);  //Open Valve
        }
      }
      if(!qrCodeShowed){
          display.setTextSize(1);
          display.setTextColor(BLACK);
          display.setCursor(12,0);
          display.println("Bitcoin QR!");
          display.display();
          display.setCursor(39 ,15);
          display.println("Balance");
          bitcoin.displayQrCode(rfidread.tag);
          qrCodeShowed = true;
      }
      
    break;
    case beverageFlow:
      if(!flowBalance)
      {
        balance = bitcoin.getBalance(rfidread.tag);
        flowBalance = true;
        payment.acceptCoins(false);
      }
      
      if(!flowShowed){
        if(pulses>0){
        liters = pulses;
        liters /= 8;
        liters /= 0.6;
        amount = liters*pricePerCl;
        }
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextColor(WHITE, BLACK);
        display.println("   -Beer-   ");
        display.setTextColor(BLACK, WHITE);
        display.print(liters,0);
        display.print("/");
        display.print(balance/pricePerCl,0);
        display.println(" cl");
        display.setCursor(3,16);
        display.setTextColor(WHITE, BLACK);
        display.print("-Amount left-");
        display.setTextColor(BLACK, WHITE);
        display.print(balance-amount,8);
        display.print(" BTC");
        display.setCursor(5,32);
        display.setTextColor(WHITE, BLACK);
        display.println("-Total cost-");
        display.setTextColor(BLACK, WHITE);
        display.print(amount,8);
        display.print(" BTC");
        display.display();
        flowShowed = true;
        if((balance-amount) <= 0){
          analogWrite(VALVERELAYPIN, 0);  //Close Valve
        }
      }

    break;
    case accountReceipt:
      
      if(!doPaymentDone){
        liters = pulses;
        liters /= 8;
        liters /= 0.6;
        amount = liters*pricePerCl;
        balance = bitcoin.getBalance(rfidread.tag);
        if((balance-amount) <= 0){
          bitcoin.payment(rfidread.tag, balance);
        }else{
          bitcoin.payment(rfidread.tag, amount);
        }
        doPaymentDone = true;
      }

      analogWrite(VALVERELAYPIN, 0);  //Close Valve

      if(analogRead(A0) > 1000){
        state = welcomeScreen;  
      }
       
      if(!reciptShowed){
        display.clearDisplay();
        display.setCursor(3,0);
        display.setTextColor(WHITE, BLACK);
        display.print("   Receipt   ");
        display.setTextColor(BLACK, WHITE);
        display.print(liters,0);
        display.println(" cl");
        display.setCursor(3,16);
        display.setTextColor(WHITE, BLACK);
        display.print("-Amount left-");
        display.setTextColor(BLACK, WHITE);
        display.print(balance-amount,8);
        display.print(" BTC");
        display.setCursor(5,32);
        display.setTextColor(WHITE, BLACK);
        display.println("-Total cost-");
        display.setTextColor(BLACK, WHITE);
        display.print(amount,8);
        display.print(" BTC");
        display.display();
        reciptShowed = true;
        }
        if(rfidread.scan()){
         state = welcomeScreen;
        }
    break;
  }

}
