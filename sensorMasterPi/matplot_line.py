#!/usr/bin/python

# import modules
import sqlite3
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy

# connect to db and create it if needed
conn = sqlite3.connect('rf24weather.sqlite', detect_types = sqlite3.PARSE_COLNAMES)
cur = conn.cursor()

# fetch dates
cur.execute('''SELECT date "[timestamp]" FROM rf24weather WHERE device == 1''')
dates1 = cur.fetchall()

# fetch temperatures
cur.execute('''SELECT temp, humidity FROM rf24weather WHERE device == 1''')
temps1 = cur.fetchall()

# format the dates on x-axis
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%d.%m %Hh'))
#plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%d/%m'))

# set a minor tick for every 24 hours
plt.gca().xaxis.set_minor_locator(mdates.DayLocator())

# plot lines
plt.plot(dates1,temps1)
plt.gcf().autofmt_xdate()

# add labels
plt.ylabel('Degrees C')
plt.title('Temperaturen Obere Egg 2a')

# add legend
leg = plt.legend(['Temperatur', 'Luftfeuchtigkeit'], loc='upper right', shadow=True, numpoints=1)
leg.get_frame().set_alpha(0.3)

# add y-axis grid with tight scaling and some padding
plt.gca().yaxis.grid(True, which='major')
plt.gca().set_ymargin(0.05)
plt.gca().autoscale(tight=True)
plt.gca().xaxis.grid(True, which='major')


# save with a smaller border
plt.savefig('matplot_line.png', bbox_inches='tight', pad_inches=0.2)
