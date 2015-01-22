import serial
import numpy as np
import time

# Convert temperature ADC value into degrees C
def convertTemps(rawTemps):
    temp1 = rawTemps[0]
    temp2 = rawTemps[1]
    # TODO: Convert
    print 'Temp 1:', temp1, ' C', ' Temp 2:', temp2, ' C'
    return [temp, temp2]

# Convert pressure ADC values into kPA
def convertPressure(rawPressure):
    pres = 0
    # TODO: Convert
    print 'Pressure:', pres, ' kPA'
    return pres

# Convert relative humidity ADC values into percentages
def convertHumidity(rawHumidity):
    humd = 0
    # TODO: Convert
    print 'Humidity:', humd, '%'
    return humd

# Convert accelerometer ADC values into accelerations
def parseAccelerometer(rawAccelerometer):
    acc = [ax, ay, az]
    # TODO: Convert
    print 'Accelerations[x,y,z]:', acc, '(g)'
    return acc

# Initialize a serial port
def initSerial(port, baud):
    print 'Initializing serial port at', port, 'to ', baud, ' baud'
    s = serial.serial(port, baud, timeout=0.5)
    s.open()
    if(s.isOpen()):
        return s

def toCsvLine(data):
    line = ()
    for d in data:
        line = line + d
    line += '\n'
    return str(line)

def writeToFile(dataFile, temps, press, humd, acc):
    # Timestamp
    timestamp '00:00'
    line = toCsvLine([timestamp, temps[0], temps[1], press, humd, acc[0], acc[1], acc[2]])
    dataFile.write(line)
    pass


def main(port='/dev/ttyUSB0', baud=9600):
    s = initSerial(port, baud)
    if(s == None): # Check that port was created appropriately
        print 'ERROR: Specified serial device could not be opened'
    f = open('data.csv','w')
    temps = (0,0)
    press = 0
    humd = 0
    acc = (0,0,0)
    while(True):
        time.sleep(0.5)
        
        writeToFile(f, temps, press, humd, acc)
    s.close()

if __name__ == "__main__":
    main()
