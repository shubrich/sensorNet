#!/usr/bin/env python
import sqlite3
from math import log
from time import strftime, localtime
from datetime import datetime

# dewpoint calculation function
def dewPointFast(celsius, humidity):
	a = 17.271
	b = 237.7
	temp = (a * celsius) / (b + celsius) + log(humidity*0.01)
	Td = (b * temp) / (a - temp)
	return Td

# connect to db and create it if needed
conn = sqlite3.connect('/var/tmp/rf24weather.db', detect_types = sqlite3.PARSE_COLNAMES)
conn.execute('''CREATE TABLE IF NOT EXISTS rf24weather(id INTEGER PRIMARY KEY, device int, voltage float, temp float, humidity float, dewpoint float, date timestamp, UNIQUE(date))''')

# open file for reading
file = open("/var/tmp/rf24weather.csv", "r")

# read the line
line = file.readline()

# split on commas into array
data = line.split(',')
device = data[0]
dht_temp = data[1]
dht_humidity = data[2]
voltage = data[3]
timestamp_date = datetime.fromtimestamp(float(data[-1]))
pretty_date = strftime("%a %d %b %Y, %H:%M:%S", localtime(float(data[-1])))

# calculate dewpoint
dht_dewpoint = 0
round_temp = int(float(dht_temp))
round_humid = int(float(dht_humidity))
if (round_temp != 0 and round_humid != 0):
    dht_dewpoint = dewPointFast(round_temp, round_humid)

# write to database
try:
    with conn:
        # if successful automatically commit
        conn.execute('''INSERT INTO rf24weather(device,voltage,temp,humidity,dewpoint,date) VALUES(?,?,?,?,?,?)''',
        (device, voltage, dht_temp, dht_humidity, dht_dewpoint, timestamp_date))
except sqlite3.IntegrityError:
    # rollback automatically
    print('Record already exists in database')
finally:
    # close cleanly
    conn.close()

# print results
print "Device: %s" % device
print "Date:",pretty_date

if (dht_dewpoint != 0):
    print "DHT11 temperature: %s" % dht_temp
    print "DHT11 humidity: %s" % dht_humidity
    print "DHT11 dewpoint: %.2fc" % dht_dewpoint
    print "Voltage: %smV" % voltage
