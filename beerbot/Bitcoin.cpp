#include "Bitcoin.h"
#include "defines.h"
Bitcoin::Bitcoin(void):display(LCDSCLK, LCDDIN, LCDDC, LCDCS, LCDRST)
{
  startRead = false; // is reading?
  startReadCoins = false; //reading balance
  
  permChangeBalance = 0; // do not update balance
  webStrPos = 0;
  
  dispOffcetX = 0;
  dispPosX = dispOffcetX; 
  dispPosX = 0+dispOffcetX;
  dispOffcetY = 15;
  dispPosY = dispOffcetY;
  display.begin();
  display.setContrast(60);
}


double Bitcoin::getBalance(char *tag){
  int webStrPos = 0;
  if (client.connect(SERVERADDRESS, 8080)){
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&tag=");
    client.print(tag);
    client.print("&mode=balance");
    client.println(" HTTP/1.0");
    client.println();  
  }while(1){
    if(client.available()){   
      char c = client.read();
      if (c == '>' ){  
        //'<' is our begining character
        startRead = true;
      }else if(startRead){
        if(c != '<'){   
          //'>' is our ending character
          coinsStr[webStrPos++] = c;
        }else{ 
        //got what we need here! We can disconnect now
        startRead = false;
        client.stop();
        client.flush();
        }
      }
    }else if(!client.connected()){
      break;
    }
  }
  double val = atof(coinsStr) ;
  return val;
}

bool Bitcoin::depositFiat(char *tag, int amount)
{
  if (client.connect(SERVERADDRESS, 8080)) {
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&tag=");
    client.print(tag);
    client.print("&mode=deposit");
    client.print("&amount=");
    client.print(amount);
    client.println(" HTTP/1.0");
    client.println();
    client.stop();
    return true; 
  } 
  return false;               
}

float Bitcoin::getBeerPriceperCl()
{
    int webStrPos = 0;
  if (client.connect(SERVERADDRESS, 8080)){
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&mode=price");
    client.println(" HTTP/1.0");
    client.println();  
  }
  while(1){
    if(client.available()){
      char c = client.read();
    if (c == '>' ){
      //'<' is our begining character
      startRead = true;
    }else if(startRead){
      if(c != '<'){   //'>' is our ending character
        coinsStr[webStrPos++] = c;
      }else{  //got what we need here! We can disconnect now
        startRead = false;
        client.stop();
        client.flush();
      }
    }
    }else if(!client.connected()){
      break;
    }
  }
  return atof(coinsStr) ;
}

char* Bitcoin::getBeerPrice(void)
{
  int webStrPos = 0;
  if (client.connect(SERVERADDRESS, 8080)){
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&mode=beerprice");
    client.println(" HTTP/1.0");
    client.println();  
  }
  while(1){
    if(client.available()){
      char c = client.read();
    if (c == '>' ){
      //'<' is our begining character
      startRead = true;
    }else if(startRead){
      if(c != '<'){   //'>' is our ending character
        coinsStr[webStrPos++] = c;
      }else{  //got what we need here! We can disconnect now
        startRead = false;
        client.stop();
        client.flush();
      }
    }
    }else if(!client.connected()){
      break;
    }
  }
  coinsStr[5] = '\0';
  return coinsStr;
}

void Bitcoin::displayQrCode(char *tag)
{ 
  if (client.connect(SERVERADDRESS, 8080)) {
    permChangeBalance = 0;
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&tag=");
    client.print(tag);
    client.println(" HTTP/1.0");
    client.println();
  }
  while(1)
  {
    if(client.available()){
      char c = client.read();
      if (c == '>' ){ //'<' is our begining character
        startRead = true;
        webStrPos = 0;
        dispPosX = dispOffcetX; 
        dispPosY = dispOffcetY;
      }else if(startRead){
        if(c != '<'){  //'>' is our ending character
          if(dispPosX > 32+dispOffcetX){
            dispPosY++;
            dispPosX = 0+dispOffcetX;
          }
          display.drawPixel(dispPosX++, dispPosY, c-'0');
        }else{  //got what we need here! We can disconnect now
          startRead = false;
          display.display();
          client.stop();
          client.flush();
        }
      }
    }else if(!client.connected()){
      break;
    }
  }
}
void Bitcoin::displayBalance(char *tag)
{
 
  float balance = getBalance(tag);
  if(balance >= 0.9){
    display.setCursor(37,26);
    display.setTextColor(WHITE, WHITE);
    display.println(getBalance(tag),3);
    display.display();
    display.setCursor(37,26);
    display.setTextColor(BLACK, WHITE);
    display.println(getBalance(tag),3);
    display.display();
    display.setCursor(50,36);
    display.println("BTC");
  }else{
    display.setCursor(38,26);
    display.setTextColor(WHITE, WHITE);
    display.println(getBalance(tag)*1000,3);
    display.display();
    display.setCursor(38,26);
    display.setTextColor(BLACK, WHITE);
    display.println(getBalance(tag)*1000,3);
    display.display();
    display.setCursor(50,36);
    display.println("mBTC");
  }
  display.display();
}

void Bitcoin::payment(char *tag, float amount)
{
  if (client.connect(SERVERADDRESS, 8080)) {
    client.print("GET /index.py?apiKey=");
    client.print(APIKEY);
    client.print("&tag=");
    client.print(tag);
    client.print("&mode=payment");
    client.print("&amount=");
    client.print(amount,8);
    client.println(" HTTP/1.0");
    client.println();
    client.stop();
  }               

}
