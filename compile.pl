#!/usr/bin/perl -w
use strict;

sub compile
{
	my $cpp_file = shift;
	$cpp_file = `basename $cpp_file`;
	chomp $cpp_file;
	my $program_name = `basename $cpp_file`;
	$program_name =~ s/\..*//;
	chomp $program_name;

	print "g++ -Wall -O2 -o $program_name common.cpp endian.cpp wii.cpp dol.cpp functions.cpp $cpp_file\n";
	`g++ -Wall -O2 -o $program_name common.cpp endian.cpp wii.cpp dol.cpp functions.cpp $cpp_file`;
}

compile($ARGV[0]);
