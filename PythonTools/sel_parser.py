import sys
import struct
from dol import *

# fix for windows shitty case-insensitive filenames
try:
	from StructX import Struct
except ImportError:
	from Struct import Struct

class SelSectionItem(Struct):
	__endian__ = Struct.BE
	def __format__(self):
		self.addr = Struct.uint32
		self.size = Struct.uint32

class SELHeader(Struct):
	__endian__ = Struct.BE
	def __format__(self):
		self.prev = Struct.uint32
		self.next = Struct.uint32
		self.section_cnt = Struct.uint32
		self.section_off = Struct.uint32
		self.path_offset = Struct.uint32
		self.path_length = Struct.uint32
		self.unk5 = Struct.uint32
		self.unk6 = Struct.uint32

		self.unk7 = Struct.uint32
		self.unk8 = Struct.uint32
		self.unk9 = Struct.uint32
		self.unk10 = Struct.uint32
		self.internalTableOffs = Struct.uint32
		self.internalTablelength = Struct.uint32
		self.externalTableOffs = Struct.uint32
		self.externalTableLength = Struct.uint32

		self.exportTableOffs = Struct.uint32
		self.exportTableLength = Struct.uint32
		self.exportTableNames = Struct.uint32
		self.importTableOffs = Struct.uint32
		self.importTableLength = Struct.uint32
		self.importTableNames = Struct.uint32

	def __str__(self):
		out = ''
		out += '      prev: %08x -       next: %08x -      section_cnt: %08x -  section_off: %08x\n' % (self.prev,self.next,self.section_cnt,self.section_off)
		out += 'ELF Offset: %08x - ELF Length: %08x -      Unk5: %08x -  Unk6: %08x\n' % (self.path_offset,self.path_length,self.unk5,self.unk6)
		out += '      Unk7: %08x -       Unk8: %08x -      Unk9: %08x - Unk10: %08x\n' % (self.unk7,self.unk8,self.unk9,self.unk10)
		out += '     internalTableOffs: %08x -      internalTablelength: %08x -     externalTableOffs: %08x - externalTableLength: %08x\n' % (self.internalTableOffs,self.internalTablelength,self.externalTableOffs,self.externalTableLength)
		out += 'SymbOffset: %08x - SymbLength: %08x - StrOffset: %08x - importTableOffs: %08x\n' % (self.exportTableOffs,self.exportTableLength,self.exportTableNames,self.importTableOffs)
		out += '     importTableLength: %08x -      importTableNames: %08x' % (self.importTableLength,self.importTableNames)
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
		if self.section != 0x3 and self.section != 0x4 and self.section != 10 and self.section != 13 and self.section != 0xfff1 and self.section != 241:
			addr = segments[ segment_naming[self.section] ]+self.symb_address
			return 'MakeFunction(0x%08X, BADADDR); MakeName(0x%08X, "%s");' % (addr,addr,GetString(self.str_offset))
		else:
			return ""

	def to_map(self):
		if self.section != 0x3 and self.section != 0x4 and self.section != 10 and self.section != 13 and self.section != 0xfff1 and self.section != 241:
			addr = segments[ segment_naming[self.section] ]+self.symb_address
			return '\t%s = 0x%08X;' % (GetString(self.str_offset),addr)
		else:
			return ""

def GetString(offset):
	offset += header.exportTableNames
	return data[offset:data.find('\0',offset)]

def main():
	global data
	global header
	global segments
	global segment_naming

	try:
		filename = sys.argv[1]
	except:
		filename = 'mh3.sel'

	f = open(filename, 'rb')
	data = f.read()
	f.close()

	DoIDC = ('-idc' in sys.argv)
	MakeMap = ('-map' in sys.argv)
	if DoIDC:
		print '#include <idc.idc>'
		print 'static main() {'
	if MakeMap:
		print 'OUTPUT_FORMAT ("binary")'
		print
		print 'SECTIONS {'

	header = SELHeader()
	header.unpack(data)

	if not DoIDC and not MakeMap:
		print '=== .SEL Header ==='
		print str(header)
		print
		print 'Original ELF filename: ' + data[header.path_offset:header.path_offset+header.path_length]
		print
		print '=== Symbols ==='


	SymbolCount = header.exportTableLength / 0x10
	Symbols = []

	segment_naming = ['None', '_f_init', '_f_text', '_f_ctors', '_f_dtors', '_f_rodata', '_f_data', '_f_bss', '_f_sbss', '_f_sdata2', '_f_zero', '_f_sdata', '_f_sbss2', '_f_zero2' ]
	segments = { }
	for i in xrange(SymbolCount):
		s = Symbol()
		s.unpack(data[header.exportTableOffs+(i*0x10):header.exportTableOffs+(i*0x10)+0x10])
		if s.section == 0xfff1:
			name = GetString(s.str_offset)
			segments[name] = s.symb_address
			if not DoIDC and not MakeMap:
				print name, hex(s.symb_address)
			'''
			if name not in segment_naming:
				print "Bad naming", name
			else:
				print "Ok  naming", name, segment_naming.index(name)
			'''
	if not DoIDC and not MakeMap:
		print

	for i in xrange(SymbolCount):
		s = Symbol()
		s.unpack(data[header.exportTableOffs+(i*0x10):header.exportTableOffs+(i*0x10)+0x10])
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
