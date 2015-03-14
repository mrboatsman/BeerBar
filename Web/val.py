import mexbtcapi
from mexbtcapi.concepts.currencies import SEK
from mexbtcapi.concepts.currency import Amount

for api in mexbtcapi.apis:
	try:
        	exchange_rate = api.market(SEK).getTicker().sell
		got = exchange_rate.convert( "10000000"*SEK )
        except:
                pass

print 30*(float(str(got).split(" ")[0])/10000000)
