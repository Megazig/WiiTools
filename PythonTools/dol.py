import struct

class DOLHeader(object):
	def __init__(self, data=None):
		self.offsetText = [0 for x in xrange(7)]
		self.offsetData = [0 for x in xrange(11)]
		self.addressText = [0 for x in xrange(7)]
		self.addressData = [0 for x in xrange(11)]
		self.sizeText = [0 for x in xrange(7)]
		self.sizeData = [0 for x in xrange(11)]
		self.addressBSS = 0
		self.sizeBSS = 0
		self.entrypoint = 0
		if data:
			offset = 0
			for x in xrange(7):
				self.offsetText[x] = struct.unpack('>I', data[x*4:x*4+4])[0]
				self.addressText[x] = struct.unpack('>I', data[28+44+x*4:28+44+x*4+4])[0]
				self.sizeText[x] = struct.unpack('>I', data[28+44+28+44+x*4:28+44+28+44+x*4+4])[0]
			for x in xrange(11):
				self.offsetData[x] = struct.unpack('>I', data[28+x*4:28+x*4+4])[0]
				self.addressData[x] = struct.unpack('>I', data[28+44+28+x*4:28+44+28+x*4+4])[0]
				self.sizeData[x] = struct.unpack('>I', data[28+44+28+44+28+x*4:28+44+28+44+28+x*4+4])[0]
				self.addressBSS, self.sizeBSS, self.entrypoint = struct.unpack('>III', data[28+44+28+44+28+44:28+44+28+44+28+44+12])
	def __str__(self):
		out  = ""
		for x in xrange(7):
			out += "offset text: %08x\n" % self.offsetText[x]
		for x in xrange(7):
			out += "address text: %08x\n" % self.addressText[x]
		for x in xrange(7):
			out += "size text: %08x\n" % self.sizeText[x]
		for x in xrange(11):
			out += "offset data: %08x\n" % self.offsetData[x]
		for x in xrange(11):
			out += "address data: %08x\n" % self.addressData[x]
		for x in xrange(11):
			out += "size data: %08x\n" % self.sizeData[x]
		out += "BSS address: %08x\n" % self.addressBSS
		out += "BSS size: %08x\n" % self.sizeBSS
		out += "entrypoint: %08x\n" % self.entrypoint
		return out
	def sorted(self, sort=True):
		out = []
		for x in xrange(7):
			if self.addressText[x] != 0:
				out.append(self.addressText[x])
		for x in xrange(11):
			if self.addressData[x] != 0:
				out.append(self.addressData[x])
		if sort:
			out.sort()
		return out

