#!/usr/bin/env python
# -*- coding: utf-8 -*-


import serial
from random import random
from time import sleep
from random import random
import logging
import argparse
import sys

class MySerialVisaWrapper(object):
	"""
	Create a virtual Visa Like Serial Wrapper for Setup Where No Visa Agent is available
	"""
	def __init__(self, com_port):
		self.log = logging.getLogger(__name__)
		self.log.info("MySerialVisaWrapper instance created")
		

	def write(self, msg):
		self.log.info("MySerialVisaWrapper Writing {}".format(msg))
		msg += '\n'
		self.ser.write(msg.encode())

	def query(self, msg):
		self.write(msg)
		try:
			ans = self.ser.readline()
			return ans.lstrip()
		except self.ser.SerialTimeoutException:
			self.log.error("MySerialVisaWrapper Query TimeOut with {}".format(msg))

	def __del__(self):
		self.ser.close()



if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--debug', action='store_true')
	parser.add_argument('--com_port', default=11)
	args = parser.parse_args()

	if args.debug:
		# Set up logging utilities
		debug_level = logging.DEBUG
		FORMAT = '%(asctime)s :: %(levelname)s :: %(name)s :: %(lineno)d :: %(funcName)s :: %(message)s'
		logging.basicConfig(level=debug_level, format=FORMAT, stream=sys.stdout)

	log = logging.getLogger(__name__)


	# ser = serial.Serial('COM{}'.format(args.com_port), 9600, timeout=1, stopbits=2)
	ser = serial.Serial('COM{}'.format(args.com_port), 9600)
	while True:
		line = ser.readline()
		if len(line):
			# print(line)
			if (line[0]==0):
				line = line[1:]
			s = line.decode().replace("\r\n","")
			# s = s[1:]
			print(s)
			if (s==":p"):
				print('Sync')
				ser.write("o\r\n".encode())
			if (s==":C"):
				print('Connect Request')
				ser.write("o\r\n".encode())
			if s.startswith(":D|"):
				print('Data Tx frame')
				if " "in s:
					print('Error Extra Space!')
				if s.count('|') == 17:
					ser.write("o\r\n".encode())
				else:
					print('Error Nb Separators!')
					ser.write("f\r\n".encode())
			if s.startswith(":P"):
				print('Param Rx frame')
				ser.write("o|2016|10|20|14|03|38|550|10|511|1145|5|3.6|3.3|0.12\r\n".encode())


	

