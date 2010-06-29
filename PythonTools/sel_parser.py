import sys
import struct
from dol import *

# fix for windows shitty case-insensitive filenames
try:
	from StructX import Struct
except ImportError:
	from Struct import Struct

class SELHeader(Struct):
	__endian__ = Struct.BE
	def __format__(self):
		self.unk1 = Struct.uint32
		self.unk2 = Struct.uint32
		self.unk3 = Struct.uint32
		self.unk4 = Struct.uint32
		self.elfname_offset = Struct.uint32
		self.elfname_length = Struct.uint32
		self.unk5 = Struct.uint32
		self.unk6 = Struct.uint32

		self.unk7 = Struct.uint32
		self.unk8 = Struct.uint32
		self.unk9 = Struct.uint32
		self.unk10 = Struct.uint32
		self.unk11 = Struct.uint32
		self.unk12 = Struct.uint32
		self.unk13 = Struct.uint32
		self.unk14 = Struct.uint32

		self.symbol_table_offset = Struct.uint32
		self.symbol_table_length = Struct.uint32
		self.string_table_offset = Struct.uint32
		self.unk15 = Struct.uint32
		self.unk16 = Struct.uint32
		self.unk17 = Struct.uint32
		self.unk18 = Struct.uint32
		self.unk19 = Struct.uint32

	def __str__(self):
		out = ''
		out += '      Unk1: %08x -       Unk2: %08x -      Unk3: %08x -  Unk4: %08x\n' % (self.unk1,self.unk2,self.unk3,self.unk4)
		out += 'ELF Offset: %08x - ELF Length: %08x -      Unk5: %08x -  Unk6: %08x\n' % (self.elfname_offset,self.elfname_length,self.unk5,self.unk6)
		out += '      Unk7: %08x -       Unk8: %08x -      Unk9: %08x - Unk10: %08x\n' % (self.unk7,self.unk8,self.unk9,self.unk10)
		out += '     Unk11: %08x -      Unk12: %08x -     Unk13: %08x - Unk14: %08x\n' % (self.unk11,self.unk12,self.unk13,self.unk14)
		out += 'SymbOffset: %08x - SymbLength: %08x - StrOffset: %08x - Unk15: %08x\n' % (self.symbol_table_offset,self.symbol_table_length,self.string_table_offset,self.unk15)
		out += '     Unk16: %08x -      Unk17: %08x -     Unk18: %08x - Unk19: %08x' % (self.unk16,self.unk17,self.unk18,self.unk19)
		return out

class Symbol(Struct):
	__endian__ = Struct.BE
	def __format__(self):
		self.str_offset = Struct.uint32
		self.symb_address = Struct.uint32
		self.section = Struct.uint32
		self.elf_hash = Struct.uint32

	def __str__(self):
		return 'StrOffset: %08x - Address: %08x - Section: %08x - ElfHash: %08x - %s' % (self.str_offset,self.symb_address,self.section,self.elf_hash,GetString(self.str_offset))

	def to_idc(self):
		if self.section != 0xb and self.section != 0x7:
			addr = segments[self.section]+self.symb_address
			return 'MakeFunction(0x%08X, BADADDR); MakeName(0x%08X, "%s");' % (addr,addr,GetString(self.str_offset))
		else:
			return ""

	def to_map(self):
		if self.section != 0xb and self.section != 0x7:
			addr = segments[self.section]+self.symb_address
			return '\t%s = 0x%08X;' % (GetString(self.str_offset),addr)
		else:
			return ""

def GetString(offset):
	offset += header.string_table_offset
	return data[offset:data.find('\0',offset)]

def main():
	global data
	global header
	global segments

	try:
		filename = sys.argv[1]
	except:
		filename = 'mh3.sel'

	f = open(filename, 'rb')
	data = f.read()
	f.close()

	try:
		filename2 = sys.argv[2]
	except:
		filename2 = main.dol

	f2 = open(filename2, 'rb')
	data2 = f2.read()
	f2.close()


	DoIDC = ('-idc' in sys.argv)
	MakeMap = ('-map' in sys.argv)
	if DoIDC:
		print '#include <idc.idc>'
		print 'static main() {'
	if MakeMap:
		print 'OUTPUT_FORMAT ("binary")'
		print
		print 'SECTIONS {'

	dol = DOLHeader(data2)
	#print dol
	#sorted_list = dol.sorted(True)
	sorted_list = dol.sorted(False)
	'''
	for offs in sorted_list:
		print "%08x" % offs
	'''

	header = SELHeader()
	header.unpack(data)

	if not DoIDC and not MakeMap:
		print '=== .SEL Header ==='
		print str(header)
		print
		print 'Original ELF filename: ' + data[header.elfname_offset:header.elfname_offset+header.elfname_length]
		print
		print '=== Symbols ==='


	'''
	segments = {
		1: 0x80004000, # .text1
		2: 0x800066E0, # .text2
		0xFFF1: 0
	}
	'''
	segments_count = 0
	segments = {}
	for x in xrange(len(sorted_list)):
		segments_count += 1
		segments[x+1] = sorted_list[x]
	segments[0xfff1] = 0

	SymbolCount = header.symbol_table_length / 0x10
	Symbols = []
	for i in xrange(SymbolCount):
		s = Symbol()
		s.unpack(data[header.symbol_table_offset+(i*0x10):header.symbol_table_offset+(i*0x10)+0x10])
		if DoIDC:
			'''
			for x in xrange(segments_count):
				print "%s\t%08x" % (GetString(s.str_offset), segments[x+1]+s.symb_address)
			sys.exit(1)
			'''
			print s.to_idc()
		elif MakeMap:
			print s.to_map()
		else:
			print str(s)
		Symbols.append(s)

	if DoIDC:
		print '}'
	if MakeMap:
		print '\t'
		print '\t.text : {'
		print '\t\tFILL (0)'
		print '\t\t'
		print '\t\t__text_start = . ;'
		print '\t\t*(.init)'
		print '\t\t*(.text)'
		print '\t\t*(.ctors)'
		print '\t\t*(.dtors)'
		print '\t\t*(.rodata)'
		print '\t\t*(.fini)'
		print '\t\t__text_end  = . ;'
		print '\t}'
		print '}'

if __name__ == "__main__":
	main()
