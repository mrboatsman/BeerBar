#!/usr/bin/env python
import cgi       
from qrcode import *
from PIL import Image
from numpy import *
from array import *
import bitcoinrpc
from bitcoinrpc.exceptions import BitcoinException, InsufficientFunds
import urllib2
import json
import time

exFile = open('excahnge.txt')

form = cgi.FieldStorage()
clientApiKey = form.getvalue('apiKey')
clientTag = form.getvalue('tag')
mode = form.getvalue('mode')
amount = form.getvalue('amount')
liter = form.getvalue('liter')

#Liters of beers
amoutBeer = 10
#Beer price for amount
beerPriceSek = 24*4
#beerPriceSek = 0
revenuePercent = float(1.00)



if clientApiKey == "AsCpwo2dlop":
    exFile = open('excahnge.txt', 'r')

    exchange_cache = exFile.read().split(" ")
    timestamp = int(time.time())

    if(timestamp-int(exchange_cache[0]) > 1800):  #cache for 30 minutes
        exFile = open('excahnge.txt', 'w')

        response = urllib2.urlopen('http://openexchangerates.org/api/latest.json?app_id=7f41d23e4eeb43d9a4605047d2c8b51d')
        data = json.load(response)
        SEKInUSD = data["rates"]["SEK"]
        USDinBTC = data["rates"]["BTC"]
        btcInSek = SEKInUSD/USDinBTC
        exFile.write("%s %s %s" % (timestamp, SEKInUSD, USDinBTC)) 

    else:
        exFile = open('excahnge.txt', 'r')
        SEKInUSD = float(exchange_cache[1])
        USDinBTC = float(exchange_cache[2])
        btcInSek = SEKInUSD/USDinBTC

    conn = bitcoinrpc.connect_to_remote('beer', 'yey', host='127.0.0.1', port=8332)
    if(mode == None):


        list =  conn.listaccounts()
        if clientTag in list:
            tag = conn.getaddressesbyaccount(clientTag)[0]
        else:
           conn.setaccount(conn.getaddressesbyaccount("null")[0],clientTag)
           tag = conn.getaddressesbyaccount(clientTag)[0]

        qr = QRCode(version=1, error_correction=ERROR_CORRECT_L, border=0, box_size=1)
        qr.add_data("bitcoin:%s?amount=0.0018&label=Beer" % tag)
        qr.make() # Generate the QRCode itself

        # Change size to fit in 32 x 29 for easyer handling later
        imn = qr.make_image()
        im = Image.new('1',(33,33))         #Create new canvas in size 32x29
        im.paste(imn, (0,0))
        arr =  im.getdata()
        strArray =  map(str, arr)

        #Formating data
        i = 0
        g = ''

        for f in strArray:
            if(f == "0"):
               f="1"
            if(f == "255"):
               f="0"
            if(i%8 == 0):
                if(i==0):
                    g = g + ">" + f
                else:
                    g = g + "" + f
            else:
                g = g + f
            i += 1
        print g + "<"

    elif(mode == "payment"):
       print "ok"
       print conn.move(clientTag,"",float(amount))
    elif(mode == "price"):
        '''Beer price in BTC/cl'''
        print ">%0.8f<" % round((1/float(btcInSek)*(beerPriceSek/amoutBeer)/100)*revenuePercent,8)
    elif(mode == "beerprice"):
        '''Beer price in mBTC/L'''
        print ">%s<" % str((1/float(btcInSek)*(beerPriceSek/amoutBeer))*1000*revenuePercent)[0:5]
    elif(mode == "balance"):
        print  ">%0.8f<" % conn.getbalance(clientTag,0)
    elif(mode == "deposit"):
        amountInSEK = float(amount)
        amountInBTC = round(amountInSEK/btcInSek,8)
        print conn.move("",clientTag,amountInBTC,3,"%s SEK" % amountInSEK)
    elif(mode == "tapped"):
        spentBTC = float(liter)*round((1/btcInSek*(beerPriceSek/amoutBeer)),8)
        print conn.move(clientTag, "",spentBTC,3,"%sL" % liter)
