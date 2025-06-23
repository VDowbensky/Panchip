"""
File name   : keil_map_parser.py
Create date : 2025-02-19
Author      : Panchip
Description : Used to parse the keil map file to obtain the usage of FLASH and RAM.
"""

import sys
import re
import os

__version__ = '1.0'

if len(sys.argv) != 2:
    print("Usage: python keil_map_parser.py <keil-map-file>")

map_file = sys.argv[1]
# print("map file path:", map_file)


def getFileData(file_path):
    try:
        with open(file_path, "r") as file:
            file_size = os.path.getsize(file_path)
            # print(f"file_size: {file_size} B")
            file_data = file.read(file_size)
            file.close()
            return file_data
    except FileNotFoundError:
        print("Error: open map file error!!!")
        return ""


map_data = getFileData(map_file)
if len(map_data) == 0:
    print("No map data found")
    exit(1)


obj1 = re.compile(r'Load Region (?P<section_name>.*?) \(Base: (.*?), Size: (?P<used_size>.*?), Max: (?P<flash_max_size>.*?), ABSOLUTE',re.S)
obj2 = re.compile(r"Execution Region (?P<section_name>.*?) \(Exec base: (?P<base_addr>.*?), Load base: (.*?), Size: (?P<usage_size>.*?), Max: (?P<region_size>.*?), ABSOLUTE", re.S)
obj3 = re.compile(r"0x(.*?) \s*0x(.*?) \s*0x(?P<ramfunc_size>.*?) Code\s*RO(.*?)\.ramfunc")


# find LOAD Flash section
flash_max_size = 0xFFFFFF
result = re.finditer(obj1, map_data)
for it in result:
    flash_max_size = int(it.group("flash_max_size"), 16)

# find .ramfunc
ram_func_size = 0
result = re.finditer(obj3, map_data)
for item in result:
    ram_func_size += int(item.group("ramfunc_size"), 16)

print("Memory Usage Info:")
print("*"*80)
print(" Memory region\t\tStart Addr\tUsed Size\tRegion Size\t %age Used")
# find others section
result = re.finditer(obj2, map_data)
for item in result:
    base_addr = int(item.group("base_addr"),16)
    section_name = item.group("section_name")
    usage_size = int(item.group("usage_size"),16)
    region_size = int(item.group("region_size"),16)

    if section_name == "IMG_HEADER" or section_name == "FLASH_TEXT_ENC" or section_name == "FLASH_TEXT_1":
        continue

    if section_name == "FLASH_TEXT" or section_name == "FLASH_TEXT_0" or section_name == "ER_IROM1":
        region_size = flash_max_size
        usage_size += ram_func_size

    print(f" {section_name:<13}\t\t0x{base_addr:08X}\t{usage_size:>8} B\t{region_size/1024:>8.2f} KB\t{usage_size/region_size*100: >8.2f} %")

print("*"*80)