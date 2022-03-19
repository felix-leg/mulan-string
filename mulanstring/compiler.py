#!/usr/bin/python3
#-*- coding: utf-8 -*-

"""
Compiles the MuLan-String library into
one file ready to use.
"""

# cSpell:words charconv cctype mulanstring

import pathlib

THIS_FILE = pathlib.Path(__file__).resolve()

def readContentsOfFile(path):
	readFlag = False
	read = ""
	
	with open(path, 'r') as f:
		for line in f:
			if line.strip() == "//CUT-START":
				readFlag = True
				continue
			if line.strip() == "//CUT-END":
				readFlag = False
				continue
			if readFlag:
				read += line
	
	return read

def loadFiles(root, listOfFiles):
	read = "\n"
	for header in listOfFiles:
		read += readContentsOfFile(root / header)
		read += "\n"
	return read

header = "/**\n"
with open( THIS_FILE.parent.parent / "LICENSE" , 'r') as lic:
	header += "".join(lic.readlines())
header += "\n**/\n\n"

#list of all required headers from the C++ standard library
header += """
#include <string>
#include <charconv>
#include <cmath>
#include <string>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>
#include <variant>


"""

#list of headers required by GetText backend
gettext_headers = """
#include <libintl.h>
#include <locale.h>


"""

mainHeadersList = [
	"errors.h",
	"mls_locale.h",
	"preparser.h",
	"template.h"
]

mainCodeList = [
	"errors.cpp",
	"mls_locale.cpp",
	"preparser.cpp",
	"template.cpp"
]

if __name__ == "__main__":
	MLS_ROOT = THIS_FILE.parent.parent / "src"
	#make the main file without backends
	vaniliaMLS = "#ifndef MULAN_STRING\n#define MULAN_STRING\n"
	vaniliaMLS += header
	vaniliaMLS += loadFiles(MLS_ROOT, mainHeadersList)
	vaniliaMLS += "\n\n\n#ifdef MULAN_STRING_IMPLEMENTATION\n\n\n"
	vaniliaMLS += loadFiles(MLS_ROOT, mainCodeList)
	vaniliaMLS += "\n\n\n#endif\n#endif\n\n\n"
	
	with open( THIS_FILE.parent / "mulanstring.hpp" , "w") as f:
		f.write( vaniliaMLS )
	
	#make the file with GetText backend
	gettextMLS = "#ifndef MULAN_STRING\n#define MULAN_STRING\n"
	gettextMLS += header + gettext_headers
	gettextMLS += loadFiles(MLS_ROOT, mainHeadersList + ["gettext_backend.h"])
	gettextMLS += "\n\n\n#ifdef MULAN_STRING_IMPLEMENTATION\n\n\n"
	gettextMLS += loadFiles(MLS_ROOT, mainCodeList + ["gettext_backend.cpp"])
	gettextMLS += "\n\n\n#endif\n#endif\n\n\n"
	
	with open( THIS_FILE.parent / "mulanstring-gettext.hpp" , "w") as f:
		f.write( gettextMLS )
