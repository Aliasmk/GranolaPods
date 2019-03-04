#/'/

import smbus
import time

bus = smbus.SMBus(1)

def writeNumber(address, value):
	bus.write_byte(address, value)
	return -1

def readNumber(address):
	number = bus.read_byte(address)
	return number

def readData(address):
	time.sleep(0.01)
	value =  bus.read_i2c_block_data(address,0)
	return value

def sendCommand(address, command, p1,p2,p3):
	#Commands:
	# Stepper Direction (1 for CCW and 2 for CW). Speed (RPM)
	# 0x01: Stepper Free Rotate (direction, speed, 0)
	# 0x02: Stepper Timed Rotate (direction, speed, seconds)
	# 0x03: Stepper Steps Rotate (direction, speed, steps)
	# 0x04: Stepper Stop (0,0,0)
	# 0x05: Calibrate (0,0,0)
	# 0x10: Request Stepper (0,0,0)
	# 0x11: Request Hall Sensor (0,0,0)
	# 0xaa: Wait (command, response)
	data = [command, p1, p2, p3]
	print("Sending command: " + str(command) + " to " + str(address) + " with args " + str(p1) + ", " + str(p2) + ", " + str(p3) + ".")
	try:
		bus.write_i2c_block_data(address,0,data)
	except:
		pass
	time.sleep(0.01)

def waitOnUnity(address,command,response):
	#poll for data on that address until event is tripped (or 20 seconds)
	print("Waiting on Unity at " + str(address) + " for " + str(response) + " response to command " + str(command) + ".")
	sendCommand(address,0xaa,command,response,0)

	pollCount = 0;
	data = readData(address)

	while (data[1] is not response) and (pollCount < 20):
		data = readData(address)
		pollCount=pollCount+1
		time.sleep(1)
	if pollCount is 20:
		print("[!] i2c read error or Unity process incomplete")
		return False
	else:
		print("Unity process completed successfully.")
		return True

def newRequestCallback(pval, fval, cval):
	print("New Request Received:")
	print("Protein: " +str(pval) + "%")
	print("Fat: " +str(fval) + "%")
	print("Carbs: " + str(cval) + "%\n")
	#New Request Received. Perform the following actions:
	#sendCommand(0x9,0x1,1,60,0)
	#time.sleep(1)
	#sendCommand(0x9,0x4,0,0,0)
	#Calibrate Gantry Cart
	sendCommand(0xA,0x1,1,60,0)
	waitOnUnity(0xA,0x5,1)

	#Move Cart to First Ingredient
	sendCommand(0xA,0x1,2,150,0)
	waitOnUnity(0x9,0x11,1)
	sendCommand(0xA,0x4,0,0,0)
	time.sleep(1)
	sendCommand(0xA,0x1,1,20,0)
	time.sleep(2)
	sendCommand(0xA,0x4,0,0,0)
	time.sleep(1)

	#Dispense Ingredient
	sendCommand(0x9,0x1,1,40,0)
	time.sleep(2)
	sendCommand(0x9,0x4,0,0,0)
	time.sleep(2)

	#Move Cart Away From Ingredient
	sendCommand(0xA,0x1,2,150,0)
	time.sleep(3)
	sendCommand(0xA,0x4,0,0,0)

	print("\nGranolaPod Complete! Returning to UI.\n")
	time.sleep(3)
