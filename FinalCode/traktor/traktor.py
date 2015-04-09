import sys
import serial
import numpy as np
import matplotlib
from matplotlib import pyplot as plt
import time
import datetime as dt
import threading

# Data
gpsData = {'systime':np.array([[],[]]),'time':np.array([[],[]]),'lat':np.array([[],[]]),'long':np.array([[],[]]),'alt':np.array([[],[]]),'sat':np.array([[],[]])}
sensorData = {'systime':np.array([[],[]]),'ET':np.array([[],[]]),'EH':np.array([[],[]]),'EHT':np.array([[],[]]),'IH':np.array([[],[]]),'IHT':np.array([[],[]]),'MP':np.array([[],[]]),'MT':np.array([[],[]]),'HH':np.array([[],[]]),'HT':np.array([[],[]]),'V':np.array([[],[]]),'AP':np.array([[],[]]),'AT':np.array([[],[]]),'BP':np.array([[],[]]),'BT':np.array([[],[]])}

# GPS Data Parser
def parseGPS(gpsStr):
    global gpsData
    #print 'Parsing GPS string'
    splitGps = (gpsStr.replace('<GPS>','').replace('<\GPS>','').replace('</GPS>','').lower()).split(',')
    now = dt.datetime.now()
    for el in splitGps:
        try:
            pair = el.split(':') # Split element
            if pair[0] is 'time':
                gpsData['time'] = np.append(gpsData['time'],[[now],[pair[1]]],axis=1)
            elif 'lat' in pair[0]:
                lat = pair[1].split(' ')
                lat = float(lat[0]) + float(lat[1])/60.0
                gpsData['lat'] = np.append(gpsData['lat'],[[now],[lat]],axis=1)
            elif 'lon' in pair[0]:
                lon = pair[1].split(' ')
                lon = float(lon[0]) + float(lon[1])/60.0
                gpsData['long'] = np.append(gpsData['long'],[[now],[lon]],axis=1)
            elif 'alt' in pair[0]:
                gpsData['alt'] = np.append(gpsData['alt'],[[now],[float(pair[1])/1000.0]],axis=1)
            elif 'sat' in pair[0]:
                gpsData['sat'] = np.append(gpsData['sat'],[[now],[int(pair[1])]],axis=1)
        except Exception, e:
            #print 'Exception in parsing GPS string', str(e)
            continue

# Sensor Data Parser
def parseSensors(dataStr):
    global sensorData
    #print 'Parsing data string'
    splitData = (dataStr.replace('<DATA>','').replace('</DATA>','').lower()).split(',')
    now = dt.datetime.now()
    for el in splitData:
        try:
            pair = el.split(':')
            if pair[0] is 'et':
                sensorData['ET'] = np.append(sensorData['ET'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'eh':
                sensorData['EH'] = np.append(sensorData['EH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'eht':
                sensorData['EHT'] = np.append(sensorData['EHT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'ih':
                sensorData['IH'] = np.append(sensorData['IH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'iht':
                sensorData['IHT'] = np.append(sensorData['IHT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'mp':
                sensorData['MP'] = np.append(sensorData['MP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'mt':
                sensorData['MT'] = np.append(sensorData['MT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'hh':
                sensorData['HH'] = np.append(sensorData['HH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'ht':
                sensorData['HT'] = np.append(sensorData['HT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'v':
                sensorData['V'] = np.append(sensorData['V'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'ap':
                sensorData['AP'] = np.append(sensorData['AP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'AT':
                sensorData['AT'] = np.append(sensorData['AT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'bp':
                sensorData['BP'] = np.append(sensorData['BP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] is 'bt':
                sensorData['BT'] = np.append(sensorData['BT'],[[now],[float(pair[1])]],axis=1)
        except Exception, e:
            #print 'Exception in parsing Data string', str(e)
            continue

# Top level Rx line parser
def parseLine(line):
    if '<GPS>' in line:
        parseGPS(line)
    elif '<DATA>' in line:
        parseSensors(line)

# Subplot data
def subplotData(data, key, color='b'):
    try:
        plt.plot((data[key])[0,:],(data[key])[1,:],color)
    except Exception, e:
        print 'Error plotting subplot', str(e)

# Plot GPS fields figure
def plotGps():
    plt.figure(1)
    plt.subplot(221)
    subplotData(gpsData,'lat')
    plt.subplot(222)
    subplotData(gpsData,'long')
    plt.subplot(223)
    subplotData(gpsData,'alt')
    plt.subplot(224)
    subplotData(gpsData,'sat')
    plt.draw()
    plt.show(block=False)

# Plot sensor data figure
def plotSensors():
    plt.figure(2)
    plt.subplot(421) # External Temperature
    subplotData(sensorData,'ET','b')
    #plt.hold(True)
    #subplotData(sensorData,'EHT','r')
    #subplotData(sensorData,'MT','g')
    #subplotData(sensorData,'HT','y')
    #plt.hold(False)

    plt.subplot(422) # Internal Temperature
    subplotData(sensorData,'IHT', 'b')
    #plt.hold(True)
    #subplotData(sensorData,'BT','r')
    #plt.hold(False)

    plt.subplot(423) # External Humidity
    subplotData(sensorData,'EH','b')
    #plt.hold(True)
    #subplotData(sensorData,'HH','r')
    #plt.hold(False)

    plt.subplot(424) # Internal Humidity
    subplotData(sensorData, 'IH')

    plt.subplot(425) # External Pressure
    subplotData(sensorData,'MP','b')
    #plt.hold(True)
    #subplotData(sensorData,'AP','r')
    #plt.hold(False)

    plt.subplot(426)
    subplotData(sensorData, 'BP')

    plt.draw()
    plt.show(block=False)

def plotHealth():
    plt.figure(3)
    plt.subplot(211)
    subplotData(sensorData,'V')
    currentVoltage = (sensorData['V'])[1,-1]
    plt.title('Battery Voltage ('+str(currentVoltage)+'V)')
    plt.subplot(212)
    plt.draw()
    plt.show(block=False)

def serialThread():
    print 'Starting serial thread'
    while(True):
        #print 'Reading line'
        line = port.readline()
        print '>:',line
        parseLine(line)
        time.sleep(0.01)

def initializePlots():
    plt.figure(1)
    plt.subplot(221)
    plt.title('GPS Latitude')
    plt.xlabel('Time')
    plt.ylabel('Latitude (Deg)')
    plt.ylim(35,50)
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(222)
    plt.title('GPS Longitude')
    plt.ylabel('Longitude (Deg)')
    plt.ylim(-90,-70)
    plt.grid()
    plt.subplot(223)
    plt.title('GPS Altitude')
    plt.xlabel('Time')
    plt.ylabel('Altitude (km)')
    plt.grid()
    plt.subplot(224)
    plt.title('GPS Satellite Count')
    plt.xlabel('Time')
    plt.ylabel('Sat. Count')
    plt.ylim(0,20)
    plt.grid()
    plt.tight_layout()
    plt.ion()
    plt.show(block=False)

    plt.figure(2)
    plt.subplot(421)
    plt.title('Ext. Temp.')
    plt.ylabel(u'Temp. (\u2103)')
    plt.grid()
    plt.subplot(422)
    plt.title('Int Temp.')
    plt.ylabel(u'Temp. (\u2103)')
    plt.grid()
    plt.subplot(423)
    plt.title('Ext. Humidity')
    plt.ylabel('Rel. Humidity [%]')
    plt.ylim(0,100)
    plt.grid()
    plt.subplot(424)
    plt.title('Int. Humidity')
    plt.ylabel('Rel. Humidity [%]')
    plt.ylim(0,100)
    plt.grid()
    plt.subplot(425)
    plt.title('Ext. Pressure')
    plt.ylabel('Pressure [kPa]')
    plt.grid()
    plt.subplot(426)
    plt.title('Int. Pressure')
    plt.ylabel('Pressure [kPa]')
    plt.grid()
    plt.tight_layout()
    plt.ion()
    plt.show(block=False)

    healthFig = plt.figure(3)
    plt.subplot(211)
    plt.title('Battery Voltage')
    plt.ylabel('Voltage (V)')
    plt.ylim(5,9)
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(212)
    plt.title('RSSI')
    plt.xlabel('Time')
    plt.xticks(rotation='vertical')
    plt.tight_layout()
    plt.grid()
    plt.ion()
    plt.show(block=False)

## Main Entry Point
print ''
print 'TrakTor Balloon Ground Station'
print '(C) 2015 Cody Hyman'
if len(sys.argv) > 1:
    portName = str(sys.argv[1])
else:
    portName = '/dev/ttyACM0'
print 'Opening serial port to ground radio at', portName
port = serial.Serial(portName, 460800, timeout=1)
if(port.isOpen()):
    print 'Serial port opened successfully'
print 'Acquiring Data'

plt.switch_backend('GtkAgg')
# Logfile
fWriteFlag = False
log = None
if(len(sys.argv) > 2):
    fWriteFlag = True
    log = open('logFile.txt','w+')

initializePlots() # Initliali

# Start serial daemon
serThread = threading.Thread(name='serial_thread', target=serialThread)
serThread.daemon = True
serThread.start()

# Update plots
while(True):
    plotGps()
    plotSensors()
    plotHealth()
    figMan = plt.get_current_fig_manager()
    #figMan.window.state('zoomed')
    time.sleep(0.05)