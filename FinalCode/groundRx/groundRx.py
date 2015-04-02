import serial


class GroundRx(object):
	def __init__(self, com):
		self.port = serial.Serial(com, 9600, timeout=1)
		if(self.port.isOpen()):
			print 'RxLink Serial Port Opened'
		pass




def main():
	initSerial()
	while(TRUE):




