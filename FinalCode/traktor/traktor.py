import sys
import serial
import numpy as np
import matplotlib.pyplot as plt
import time
import datetime as dt
import re

gpsData = {'systime':[],'time':[],'lat':[],'long':[],'alt':[],'sat':[]}
sensorData = {'systime':[],'ET':[],'EH':[],'EHT':[],'IH':[],'IHT':[],'MP':[],'MT':[],'HH':[],'HT':[],'V':[],'AP':[],'AT':[],'BP':[],'BT':[]}

# GPS Data Parser
def parseGPS(gpsStr):
    global gpsData
    print 'Parsing GPS string'
    splitGps = (gpsStr.replace('<GPS>','').replace('</GPS>','').lower()).split(',')
    now = dt.datetime.now().time()
    for el in splitGps:
        try:
            pair = el.split(':') # Split element
            if pair[0] is 'time':
                gpsData['time'].append((now,pair[1]))
            elif 'lat' in pair[0]:
                lat = pair[1].split(' ')
                lat = float(lat[0]) + float(lat[1])/60.0
                gpsData['lat'].append((now,lat))
            elif 'lon' in pair[0]:
                lon = pair[1].split(' ')
                lon = float(lon[0]) + float(lon[1])/60.0
                gpsData['long'].append((now,lon))
            elif pair[0] is 'alt':
                gpsData['alt'].append((now,float(pair[1])))
            elif pair[0] is 'sat':
                gpsData['sat'].append((now,int(pair[1])))
        except Exception, e:
            print 'Exception in parsing GPS string', str(e)

# Sensor Data Parser
def parseSensors(dataStr):
    global sensorData
    print 'Parsing data string'
    splitData = (gpsStr.replace('<DATA>','').replace('</DATA>','').lower()).split(',')
    now = dt.datetime.now().time()
    for el in splitData:
        try:
            pair = el.split(':')
            if pair[0] is 'et':
                sensorData['ET'].append((now,float(pair[1])))
            elif pair[0] is 'eh':
                sensorData['EH'].append((now,float(pair[1])))
            elif pair[0] is 'eht':
                sensorData['EHT'].append((now,float(pair[1]))
            elif pair[0] is 'ih':
                sensorData['IH'].append((now,float(pair[1])))
            elif pair[0] is 'iht':
                sensorData['IHT'].append((now,float(pair[1])))
            elif pair[0] is 'mp':
                sensorData['MP'].append((now,float(pair[1])))
            elif pair[0] is 'mt':
                sensorData['MT'].append((now,float(pair[1])))
            elif pair[0] is 'hh':
                sensorData['HH'].append((now,float(pair[1])))
            elif pair[0] is 'ht':
                sensorData['HT'].append((now,float(pair[1])))
            elif pair[0] is 'v':
                sensorData['V'].append((now,float(pair[1])))
            elif pair[0] is 'ap':
                sensorData['AP'].append((now,float(pair[1])))
            elif pair[0] is 'AT':
                sensorData['AT'].append((now,float(pair[1])))
            elif pair[0] is 'bp':
                sensorData['BP'].append((now,float(pair[1])))
            elif pair[0] is 'bt':
                sensorData['BT'].append((now,float(pair[1])))
        except Exception, e:
            print 'Exception in parsing Data string', str(e)


    sensorData['EH'].append(0)
    sensorData['EHT'].append(0)
    sensorData['IH'].append(0)
    sensorData['IHT'].append(0)
    sensorData['MP'].append(0)
    sensorData['MT'].append(0)
    sensorData['HH'].append(0)
    sensorData['HT'].append(0)
    sensorData['V'].append(0)
    sensorData['AP'].append(0)
    sensorData['AT'].append(0)

def parseLine(line):
    if '<GPS>' in line:
        parseGPS(line)
    elif '<DATA>' in line:
        parseSensors(line)


print 'TrakTor Balloon Ground Station'
print '(C) 2015 Cody Hyman'
print 'Opening serial port to ground radio at', str(sys.argv[1])
port = serial.Serial(str(sys.argv[1]), 460800, timeout=1)
if(port.isOpen()):
    print 'Serial port opened successfully'
print 'Acquiring Data'

#plt.plot([0],[0])
#plt.show()
#plt.xlabel('Time (s)')

while(True):
    print 'Reading line'
    line = port.readline()
    #print 'Received line', str(line)
    parseLine(line)