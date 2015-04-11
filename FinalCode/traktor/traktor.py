#!/usr/bin/python

# TrakTor Balloon Ground Station
# (C) 2015 Cody Hyman
# Last Modified 9 APR 2015

import sys
import serial
import numpy as np
import matplotlib
from matplotlib import pyplot as plt
import time
import datetime as dt
import threading
import signal

from PyQt4 import QtGui, QtCore

# Global Data
gpsData = {'systime':np.array([[],[]]),'time':np.array([[],[]]),'lat':np.array([[],[]]),'long':np.array([[],[]]),'alt':np.array([[],[]]),'sat':np.array([[],[]])}
sensorData = {'systime':np.array([[],[]]),'ET':np.array([[],[]]),'EH':np.array([[],[]]),'EHT':np.array([[],[]]),'IH':np.array([[],[]]),'IHT':np.array([[],[]]),'MP':np.array([[],[]]),'MT':np.array([[],[]]),'HH':np.array([[],[]]),'HT':np.array([[],[]]),'V':np.array([[],[]]),'AP':np.array([[],[]]),'AT':np.array([[],[]]),'BP':np.array([[],[]]),'BT':np.array([[],[]])}

# Serial port
port = None 

def sigint(signum, frame):
    sys.exit()
signal.signal(signal.SIGINT, sigint)

## Telemetry Textwindow
class Traktor_TelemetryTextWindow(QtGui.QWidget):
    def __init__(self):
        super(Traktor_TelemetryTextWindow, self).__init__()
        self.initGUI()
        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(50)
        self.timer.timeout.connect(self.updateGUI)
        self.start()

    def start(self):
        self.timer.start()

    def stop(self):
        self.timer.stop()

    def initGUI(self):
        self.resize(300,600)
        self.setWindowTitle('TrakTor Ground Station Telemetry')

        grid = QtGui.QGridLayout()
        etgrid = QtGui.QGridLayout()
        ehgrid = QtGui.QGridLayout()
        epgrid = QtGui.QGridLayout()
        itgrid = QtGui.QGridLayout()
        ihgrid = QtGui.QGridLayout()
        ipgrid = QtGui.QGridLayout()
        hlthgrid = QtGui.QGridLayout()
        gpsgrid = QtGui.QGridLayout()

        self.setLayout(grid)

        # External Temperature
        extTempLabel = QtGui.QLabel('External Temperature')
        etLabel = QtGui.QLabel(u'TMP275 [\u2103]')
        ehtLabel = QtGui.QLabel(u'Si7020 [\u2103]')
        htLabel = QtGui.QLabel(u'HIH6030 [\u2103]')
        mtLabel = QtGui.QLabel(u'MS5607 [\u2103]')

        self.etValue = QtGui.QLineEdit()
        self.etValue.setReadOnly(True)
        self.ehtValue = QtGui.QLineEdit()
        self.ehtValue.setReadOnly(True)
        self.htValue = QtGui.QLineEdit()
        self.htValue.setReadOnly(True)
        self.mtValue = QtGui.QLineEdit()
        self.mtValue.setReadOnly(True)

        # External Humidity
        extHumLabel = QtGui.QLabel('External Humidity')
        ehLabel = QtGui.QLabel('Si7020 [%RH]')
        hhLabel = QtGui.QLabel('HIH6030 [%RH]')

        self.ehValue = QtGui.QLineEdit()
        self.ehValue.setReadOnly(True)
        self.hhValue = QtGui.QLineEdit()
        self.hhValue.setReadOnly(True)

        # External Pressure
        extPressLabel = QtGui.QLabel('External Pressure')
        mpLabel = QtGui.QLabel('MS5607 [kPa]')
        apLabel = QtGui.QLabel('MPXM2102 [kPa]')

        self.mpValue = QtGui.QLineEdit()
        self.mpValue.setReadOnly(True)
        self.apValue = QtGui.QLineEdit()
        self.apValue.setReadOnly(True)

        # Internal Temperature
        intTempLabel = QtGui.QLabel('Internal Temperature')
        btLabel = QtGui.QLabel(u'BMP280 [\u2103]')
        ihtLabel = QtGui.QLabel(u'Si7020 [\u2103]')

        self.btValue = QtGui.QLineEdit()
        self.btValue.setReadOnly(True)
        self.ihtValue = QtGui.QLineEdit()
        self.ihtValue.setReadOnly(True)

        # Internal Humidity
        intHumLabel = QtGui.QLabel('Internal Humidity')
        ihLabel = QtGui.QLabel('Si7020 [%RH]')

        self.ihValue = QtGui.QLineEdit()
        self.ihValue.setReadOnly(True)

        #Internal Pressure
        intPressLabel = QtGui.QLabel('Internal Pressure')
        bpLabel = QtGui.QLabel('BMP280 [kPa]')
        self.bpValue = QtGui.QLineEdit()
        self.bpValue.setReadOnly(True)

        #Internal Voltage
        hlthLabel = QtGui.QLabel('System Health')
        vLabel = QtGui.QLabel('Battery Volt. [V]')
        self.vValue = QtGui.QLineEdit()
        self.vValue.setReadOnly(True)

        gpsLabel = QtGui.QLabel('GPS')
        latLabel = QtGui.QLabel(u'Latitude [\u00B0]')
        lonLabel = QtGui.QLabel(u'Longitude [\u00B0]')
        altLabel = QtGui.QLabel('Altitude [km]')
        satLabel = QtGui.QLabel('Satellites [#]')
        self.latValue = QtGui.QLineEdit()
        self.latValue.setReadOnly(True)
        self.lonValue = QtGui.QLineEdit()
        self.lonValue.setReadOnly(True)
        self.altValue = QtGui.QLineEdit()
        self.altValue.setReadOnly(True)
        self.satValue = QtGui.QLineEdit()
        self.satValue.setReadOnly(True)

        grid.addWidget(extTempLabel,0,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(etLabel,1,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.etValue,1,1)
        grid.addWidget(ehtLabel,2,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.ehtValue,2,1)
        grid.addWidget(htLabel,3,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.htValue,3,1)
        grid.addWidget(mtLabel,4,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.mtValue,4,1)

        grid.addWidget(extHumLabel,5,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(ehLabel,6,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.ehValue,6,1)
        grid.addWidget(hhLabel,7,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.hhValue,7,1)

        grid.addWidget(extPressLabel,8,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(mpLabel,9,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.mpValue,9,1)
        grid.addWidget(apLabel,10,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.apValue,10,1)

        grid.addWidget(intTempLabel,11,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(btLabel,12,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.btValue,12,1)
        grid.addWidget(ihtLabel,13,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.ihtValue,13,1)

        grid.addWidget(intHumLabel,14,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(ihLabel,15,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.ihValue,15,1)

        grid.addWidget(intPressLabel,16,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(bpLabel,17,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.bpValue,17,1)

        grid.addWidget(hlthLabel,18,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(vLabel,19,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.vValue,19,1)

        grid.addWidget(gpsLabel,20,0,1,2,QtCore.Qt.AlignCenter)
        grid.addWidget(latLabel,21,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.latValue,21,1)
        grid.addWidget(lonLabel,22,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.lonValue,22,1)
        grid.addWidget(altLabel,23,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.altValue,23,1)
        grid.addWidget(satLabel,24,0,QtCore.Qt.AlignRight)
        grid.addWidget(self.satValue,24,1)

        self.show()

    @QtCore.pyqtSlot()
    def updateGUI(self):
        global gpsData
        global sensorData
        self.updateSensors(sensorData)
        self.updateGPS(gpsData)

    def updateSensors(self, data):
        self.etValue.setText((data['ET'])[1,-1])
        self.ehtValue.setText((data['EHT'])[1,-1])
        self.htValue.setText((data['HT'])[1,-1])
        self.mtValue.setText((data['MT'])[1,-1])

        self.ehValue.setText((data['EH'])[1,-1])
        self.hhValue.setText((data['HH'])[1,-1])

        self.mpValue.setText((data['MP'])[1,-1])
        self.apValue.setText((data['AP'])[1,-1])

        self.ihtValue.setText((data['IHT'])[1,-1])
        self.btValue.setText((data['BT'])[1,-1])

        self.ihValue.setText((data['IH'])[1,-1])
        self.bpValue.setText((data['BP'])[1,-1])

        self.vValue.setText((data['V'])[1,-1])

    def updateGPS(self, data):
        self.latValue.setText(str((data['lat'])[1,-1]))
        self.lonValue.setText(str((data['lon'])[1,-1]))
        self.altValue.setText(str((data['alt'])[1,-1]))
        self.satValue.setText(str((data['sat'])[1,-1]))

# GPS Data Parser
def parseGPS(gpsStr):
    global gpsData
    #print 'Parsing GPS string'
    splitGps = (gpsStr.replace('<GPS>','').replace('<\GPS>','').replace('</GPS>','').lower()).split(',')
    # TODO: Verify correct string
    now = dt.datetime.now()
    for el in splitGps:
        try:
            pair = el.split(':') # Split element
            if pair[0] == 'time':
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
    #print 'Parsing data string\n\n\n\n\n\n'
    splitData = (dataStr.replace('<DATA>','').replace('</DATA>','').lower()).split(',')
    now = dt.datetime.now()
    for el in splitData:
        try:
            pair = el.split(':')
            if pair[0] == 'et':
                sensorData['ET'] = np.append(sensorData['ET'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'eh':
                sensorData['EH'] = np.append(sensorData['EH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'eht':
                sensorData['EHT'] = np.append(sensorData['EHT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'ih':
                sensorData['IH'] = np.append(sensorData['IH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'iht':
                sensorData['IHT'] = np.append(sensorData['IHT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'mp':
                sensorData['MP'] = np.append(sensorData['MP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'mt':
                sensorData['MT'] = np.append(sensorData['MT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'hh':
                sensorData['HH'] = np.append(sensorData['HH'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'ht':
                sensorData['HT'] = np.append(sensorData['HT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'v':
                sensorData['V'] = np.append(sensorData['V'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'ap':
                sensorData['AP'] = np.append(sensorData['AP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'AT':
                sensorData['AT'] = np.append(sensorData['AT'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'bp':
                sensorData['BP'] = np.append(sensorData['BP'],[[now],[float(pair[1])]],axis=1)
            elif pair[0] == 'bt':
                sensorData['BT'] = np.append(sensorData['BT'],[[now],[float(pair[1])]],axis=1)
        except Exception, e:
            print 'Exception in parsing Data string', str(e)
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
        return plt.plot((data[key])[0,:],(data[key])[1,:],color, label=key)
    except Exception, e:
        print 'Error plotting subplot', str(e)

# Plot GPS fields figure
def plotGps():
    global gpsData
    plt.figure(1)
    plt.subplot(221)
    subplotData(gpsData,'lat')
    try:
        plt.title('GPS Latitude ('+str(gpsData['lat'][1,-1]+')'))
    except Exception, e:
        print ''
    plt.subplot(222)
    subplotData(gpsData,'long')
    try:
        plt.title('GPS Longitude ('+str(gpsData['lon'][1,-1]+')'))
    except Exception, e:
        print ''
    plt.subplot(223)
    subplotData(gpsData,'alt')
    try:
        plt.title('GPS Altitude ('+str(gpsData['alt'][1,-1]+')'))
    except Exception, e:
        print ''
    plt.subplot(224)
    subplotData(gpsData,'sat')
    try:
        plt.title('GPS Satellites ('+str(gpsData['sat'][1,-1]+')'))
    except Exception, e:
        print ''
    plt.draw()
    plt.show(block=False)

# Plot sensor data figure
def plotSensors():
    global sensorData
    plt.figure(2)
    #print 'Plotting sensors', np.size(sensorData['EH']),'\n\n\n\n\n'
    plt.subplot(321) # External Temperature
    etplt, = subplotData(sensorData,'ET','b')
    ehtplt, = subplotData(sensorData,'EHT','r')
    mtplt, = subplotData(sensorData,'MT','g')
    htplt, = subplotData(sensorData,'HT','y')
    plt.legend([etplt, ehtplt, mtplt, htplt],['TMP275','Si7020','MS5607','HIH6030'],loc='lower left')
    try:
        plt.title('Ext. Temperature ('+str(sensorData['ET'][1,-1])+'/'+str(sensorData['EHT'][1,-1])+'/'+str(sensorData['MT'][1,-1])+'/'+str(sensorData['HT'][1,-1])+u'\u2103)')
    except:
        print ''
    plt.draw()
    plt.subplot(322) # Internal Temperature
    ihtplt, = subplotData(sensorData,'IHT', 'b')
    btplt, = subplotData(sensorData,'BT','r')
    plt.legend([ihtplt,btplt],['Si7020','BMP280'],loc='lower left')
    try:
        plt.title('Int. Temperature ('+str(sensorData['IHT'][1,-1])+u'\u2103)')
    except:
        print ''
    plt.draw()
    plt.subplot(323) # External Humidity
    ehplt, = subplotData(sensorData,'EH','b')
    hhplt, = subplotData(sensorData,'HH','r')
    plt.legend([ehplt,hhplt],['Si7020','HIH6030'])
    try:
        plt.title('Ext. Humidity ('+str(sensorData['EH'][1,-1])+'/'+str(sensorData['HH'][1,-1])+'%)')
    except:
        print ''
    plt.draw()
    plt.subplot(324) # Internal Humidity
    ihplt, = subplotData(sensorData, 'IH')
    try:
        plt.title('Int. Humidity ('+str(sensorData['IH'][1,-1])+'%)')
    except:
        print ''
    plt.draw()
    plt.subplot(325) # External Pressure
    mpplt, = subplotData(sensorData,'MP','b')
    applt, = subplotData(sensorData,'AP','r')
    plt.legend([mpplt, applt],['MS5607','MPXM2102'],loc='lower left')
    try:
        plt.title('Ext. Pressure ('+str(sensorData['MP'][1,-1])+'/'+str(sensorData['AP'][1,-1])+'kPa)')
    except:
        print ''
    plt.draw()
    plt.subplot(326)
    bpplt = subplotData(sensorData, 'BP')
    try:
        plt.title('Int. Pressure ('+str(sensorData['BP'][1,-1])+'kPa)')
    except:
        print ''
    plt.draw()
    plt.show(block=False)

# Plot system health figure
def plotHealth():
    global sensorData
    try:
        plt.figure(3)
        plt.subplot(211)
        subplotData(sensorData,'V')
        currentVoltage = (sensorData['V'])[1,-1]
        plt.title('Battery Voltage ('+str(currentVoltage)+'V)')
        plt.subplot(212)
        plt.draw()
        plt.show(block=False)
    except Exception, e:
        pass

# Plot initialization
def initializePlots():
    plt.figure(1)
    plt.subplot(221)
    plt.ion()
    plt.title('GPS Latitude')
    plt.xlabel('Time')
    plt.ylabel('Latitude (Deg)')
    plt.ylim(35,50)
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(222)
    plt.ion()
    plt.title('GPS Longitude')
    plt.ylabel('Longitude (Deg)')
    plt.xticks(rotation='vertical')
    plt.ylim(-90,-70)
    plt.grid()
    plt.subplot(223)
    plt.ion()
    plt.title('GPS Altitude')
    plt.xlabel('Time')
    plt.ylabel('Altitude (km)')
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(224)
    plt.ion()
    plt.title('GPS Satellite Count')
    plt.xlabel('Time')
    plt.ylabel('Sat. Count')
    plt.xticks(rotation='vertical')
    plt.ylim(0,20)
    plt.grid()
    plt.tight_layout()
    plt.ion()
    plt.show(block=False)

    plt.figure(2)
    plt.subplot(321)
    plt.ion()
    plt.title('Ext. Temp.')
    plt.ylim(-70,40)
    plt.ylabel(u'Temp. (\u2103)')
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(322)
    plt.ion()
    plt.title('Int Temp.')
    plt.ylim(-70,50)
    plt.ylabel(u'Temp. (\u2103)')
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(323)
    plt.ion()
    plt.title('Ext. Humidity')
    plt.ylabel('Rel. Humidity [%]')
    plt.ylim(0,100)
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(324)
    plt.ion()
    plt.title('Int. Humidity')
    plt.ylabel('Rel. Humidity [%]')
    plt.ylim(0,100)
    plt.xticks(rotation='vertical')
    plt.ylim(0,100)
    plt.grid()
    plt.subplot(325)
    plt.ion()
    plt.title('Ext. Pressure')
    plt.ylabel('Pressure [kPa]')
    plt.ylim(0,120)
    plt.xticks(rotation='vertical')
    plt.grid()
    plt.subplot(326)
    plt.ion()
    plt.title('Int. Pressure')
    plt.ylabel('Pressure [kPa]')
    plt.xticks(rotation='vertical')
    plt.ylim(0,120)
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

# Serial Daemon Thread
def serialThread():
    global port
    global gpsData
    [log,lograw] = newLogs()
    gpslog = newGpsLog()
    print 'Starting serial thread'
    serCounter = 0
    gpsCounter = 0
    while(True):
        if(port != None):
            #print 'Reading line'
            line = port.readline()
            if(len(line) > 0):
                serCounter = serCounter + 1
                #if(serCounter > 500):
                #    log.close()
                #    lograw.close()
                #    [log, lograw] = newLogs()
            if(lograw != None):
                lograw.write(line)
            # TODO: Log formatted data as CSV
            print '>:',line
            parseLine(line)
            
            gpsCounter = gpsCounter + 1
            if(gpsCounter >= 60):
                gpsCounter = 0
                gpslog.close()
                gpslog = newGpsLog()
            lat = ''
            alt = ''
            lon = ''
            gtime = dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            try:
                lat = str(gpsData['lat'][1,-1])
            except Exception, e:
                print 'Error parsing Lat for file'
            try:
                lon = str(gpsData['lon'][1,-1])
            except Exception, e:
                print 'Error parsing Lon for file'  
            try:
                alt = str(1000.0*gpsData['alt'][1,-1])
            except Exception, e:
                print 'Error parsing Alt for file'
            try:
                gpslog.write(str(gpsCounter)+','+ gtime + ',' + lat +','+ lon + ',' + alt +'\n')
            except Exception, e:
                print 'Error writing GPS log'

            time.sleep(0.01)

def newGpsLog():
    ftime = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    dir = '/home/cody/school/aoss584/traktorlogs/gps/'
    gpslog = open(dir+'gps_' + ftime + '.csv','w+')
    gpslog.write('name,time,latitude,longitude,altitude\n')
    return gpslog

def newLogs():
    log = None
    lograw = None
    ftime = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    dir = '/home/cody/school/aoss584/traktorlogs/'
    lograw = open(dir+'lograw_'+ ftime+'.txt', 'w+')
    log = open(dir+'logfile_' + ftime + '.txt','w+')
    return [log, lograw]

# Plotting Daemon Thread
def plotRoutine(args=None):
    # Update plots:
    plotGps()
    plotSensors()
    plotHealth()

# Qt App thread
def qtThread():
    print 'Starting Qt Application'
    app = QtGui.QApplication(sys.argv)
    tele = Traktor_TelemetryTextWindow()
    sys.exit(app.exec_())

## Main Entry Point
def main():
    global port
    print ''
    print '===== TrakTor Balloon Ground Station ====='
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


    # Start serial daemon
    serThread = threading.Thread(name='serial_thread', target=serialThread)
    serThread.daemon = True
    serThread.start()

    initializePlots() # Initlialize plots

    #app = QtGui.QApplication(sys.argv)
    #tele = Traktor_TelemetryTextWindow()
    #sys.exit(app.exec_())


    while(True):
        plotRoutine()
        plt.pause(0.5)
        #time.sleep(0.2)

if __name__ == '__main__':
    main()