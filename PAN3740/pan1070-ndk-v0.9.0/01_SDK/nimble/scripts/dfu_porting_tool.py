
"""
File        :  dfu_porting_tool.py
CreateDate  :  2025-04-07
Author      :  Gaoqiu
Copyright   :  Panchip
"""

import sys
import xml.etree.ElementTree as ET

__version__ = '1.0'

if len(sys.argv) != 2:
    print("Usage: python dfu_porting_tool.py <*.uvprojx>")

uvprojx_file = sys.argv[1]
print("uvprojx file path:", uvprojx_file)

# uvprojx_file = "./ble_periph_hr.uvprojx"

dfu_header_file_path = r"""
;..\..\..\..\component\dfu;
..\..\..\..\component\dfu\tinycbor\include;
..\..\..\..\component\dfu\mcumgr\cborattr\include;
..\..\..\..\component\dfu\mcumgr\cmd\img_mgmt\include;
..\..\..\..\component\dfu\mcumgr\cmd\os_mgmt\include;
..\..\..\..\component\dfu\mcumgr\mgmt\include;
..\..\..\..\component\dfu\mcumgr\smp\include;
..\..\..\..\component\dfu\mcumgr\util\include;
"""

dfu_header_file_path = dfu_header_file_path.replace("\n", "")

xml = ET.parse(uvprojx_file)
root = xml.getroot()

# find Cads node from root
Cads = []
for node in root.iter("Cads"):
    #print(node.tag)
    Cads.append(node)

# find IncludePath node for Cads
if Cads is not None:
    for node in Cads[0].iter("IncludePath"):
        if node.text.find(dfu_header_file_path,0) == -1:
            text = node.text
            text += dfu_header_file_path
            #print(node.tag, "\n", dfu_header_file_path.replace(";", "\n"))
            node.text = text
            xml.write(uvprojx_file,encoding="utf-8",xml_declaration=True)
            print("header add completely!")
        else:
            print("header file existed!!")
else:
    print("No found Cads node....")