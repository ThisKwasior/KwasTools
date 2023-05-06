#! /bin/sh

g++ pcmodeltool.cpp -o pcmodeltool.elf -Wall -O3 -s -static \
    ../utils/pugixml.cpp ../kwaslib/he/BINA.c \
    ../kwaslib/utils/type_readers.c ../kwaslib/utils/path_utils.c