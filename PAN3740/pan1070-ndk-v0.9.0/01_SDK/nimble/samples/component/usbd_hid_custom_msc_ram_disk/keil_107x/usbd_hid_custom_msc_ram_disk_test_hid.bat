@echo off

echo Start USB HID Custom INOUT Test..
echo=

:: Use this test script if CONFIG_HID_ASSIGN_REPORT_ID is enabled in sdk_config.h
REM python ..\..\..\..\component\usb_device\CherryUSB\tools\test_srcipts\test_hid_inout.py 0xFFFF 0x9F10

:: Use this test script if CONFIG_HID_ASSIGN_REPORT_ID is disabled in sdk_config.h
python ..\..\..\..\component\usb_device\CherryUSB\tools\test_srcipts\test_hid_inout_implicit_report_id.py 0xFFFF 0x9F10

pause
