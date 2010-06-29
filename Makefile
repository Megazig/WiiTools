CXX			:=	g++
COMMON		:=	common.cpp endian.cpp wii.cpp dol.cpp functions.cpp
CXXFLAGS	:=	-O2 -Wall
PROGRAMS	:=	check_fwrite create_jump find_debug_functions mega_compare mega_info_creator

all: 					$(PROGRAMS)
check_fwrite:			PROGRAM_NAME = check_fwrite
create_jump:			PROGRAM_NAME = create_jump
find_debug_functions:	PROGRAM_NAME = find_debug_functions
mega_compare:			PROGRAM_NAME = mega_compare
mega_info_creator:		PROGRAM_NAME = mega_info_creator

$(PROGRAMS):
	$(CXX) $(CXXFLAGS) $(COMMON) $(PROGRAM_NAME).cpp -o $(PROGRAM_NAME)

clean:
	@echo Cleaning binaries...
	@rm -f $(PROGRAMS)

