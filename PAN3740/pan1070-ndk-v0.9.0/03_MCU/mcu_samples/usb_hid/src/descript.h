/****************************************

  descript.h
  Endpoint 0 descriptor definitions for MUSBFSFC firmware

  Note the defined structures assume WORDS are stored little-endian.

****************************************/

#ifndef _DESCRIPT_H_
#define _DESCRIPT_H_

/* Command bit fields */
#define M_CMD_DATADIR   0x80

/* Request Type Field */
#define M_CMD_TYPEMASK  0x60
#define M_CMD_STDREQ    0x00
#define M_CMD_CLASSREQ  0x20
#define M_CMD_VENDREQ   0x40
#define M_CMD_STDDEVIN  0x80
#define M_CMD_STDDEVOUT 0x00
#define M_CMD_STDIFIN   0x81
#define M_CMD_STDIFOUT  0x01
#define M_CMD_STDEPIN   0x82
#define M_CMD_STDEPOUT  0x02

/* Standard Request Codes */
#define GET_STATUS              0x00
#define CLEAR_FEATURE           0x01
#define SET_FEATURE             0x03
#define SET_ADDRESS             0x05
#define GET_DESCRIPTOR          0x06
#define SET_DESCRIPTOR          0x07
#define GET_CONFIGURATION       0x08
#define SET_CONFIGURATION       0x09
#define GET_INTERFACE           0x0A
#define SET_INTERFACE           0x0B
#define SYNCH_FRAME             0x0C

/* Standard Descriptor Types */
#define M_DST_DEVICE     0x01
#define M_DST_CONFIG     0x02
#define M_DST_STRING     0x03
#define M_DST_INTERFACE  0x04
#define M_DST_ENDPOINT   0x05
#define M_DST_DEVQUAL    0x06
#define M_DST_OTHERSPEED 0x07
#define M_DST_POWER      0x08
#define M_CMD_DESCMASK   0xFF00
#define M_CMD_DEVICE     (M_DST_DEVICE<<8)
#define M_CMD_CONFIG     (M_DST_CONFIG<<8)
#define M_CMD_STRING     (M_DST_STRING<<8)
#define M_CMD_DEVQUAL    (M_DST_DEVQUAL<<8)
#define M_CMD_OTHERSPEED (M_DST_OTHERSPEED<<8)
#define M_CMD_REPORT	 (0X2200)

/* Endpoint transfer types */
#define M_EP_TFMASK     0x03
#define M_EP_CONTROL    0x00
#define M_EP_ISO        0x01
#define M_EP_BULK       0x02
#define M_EP_INTR       0x03

/* Standard Device Feature Selectors */
#define M_FTR_DEVREMWAKE 0x0001

/* Define values for Standard Device Descriptor */
#define M_DEVDSC_CLASS          		0x00    /* Device Class */
#define M_DEVDSC_SUBCLASS       		0x00    /* Device SubClass */
#define M_DEVDSC_PROTOCOL       		0x00    /* Device Protocol */
#define M_DEVDSC_VENDOR         		0x0000  /* Vendor ID */
#define M_DEVDSC_PRODUCT        		0x0000  /* Product ID */
#define M_DEVDSC_RELEASE        		0x0000  /* Release number */
#define M_DEVDSC_MANUI          		0       /* Manufacturer string index */
#define M_DEVDSC_PRODI          		0       /* Product string index */
#define M_DEVDSC_SERIALI        		0       /* Serial number string index */
#define M_DEVDSC_CONFIGS        		2       /* Number of configurations */


/* Define values for descriptors in configuration 1 */
#define M_CFG1_NUMIFS           		2       /* Number of interfaces */
#define M_CFG1_CFGI             		0       /* Configuration description string index */
#define M_CFG1_ATTRIBUTES       		0x00    /* Configuration attributes */
#define M_CFG1_MAXPOWER         		0       /* Maximum power (units of 2 mA) */

/* Interface 0, alternate 0 */
#define M_CFG1_I0A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG1_I0A0_IFCLASS     		0       /* Interface class */
#define M_CFG1_I0A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG1_I0A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG1_I0A0_IFI         		0       /* Interface string index */
#define M_CFG1_I0A0_0_EPADDR    		0x81    /* Endpoint address (endpoint 1 IN) */
#define M_CFG1_I0A0_0_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I0A0_0_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I0A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG1_I0A0_1_EPADDR    		0x01    /* Endpoint address (endpoint 1 OUT) */
#define M_CFG1_I0A0_1_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I0A0_1_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I0A0_1_INTER     		0       /* Endpoint interval */

/* Interface 1, alternate 0 */
#define M_CFG1_I1A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG1_I1A0_IFCLASS     		0       /* Interface class */
#define M_CFG1_I1A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG1_I1A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG1_I1A0_IFI         		0       /* Interface string index */
#define M_CFG1_I1A0_0_EPADDR    		0x82    /* Endpoint address (endpoint 2 IN) */
#define M_CFG1_I1A0_0_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I1A0_0_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I1A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG1_I1A0_1_EPADDR    		0x02    /* Endpoint address (endpoint 2 OUT) */
#define M_CFG1_I1A0_1_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I1A0_1_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I1A0_1_INTER     		0       /* Endpoint interval */

/* Interface 1, alternate 1 */
#define M_CFG1_I1A1_NUMEPS      		2       /* Number of endpoints */
#define M_CFG1_I1A1_IFCLASS     		0       /* Interface class */
#define M_CFG1_I1A1_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG1_I1A1_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG1_I1A1_IFI         		0       /* Interface string index */
#define M_CFG1_I1A1_0_EPADDR    		0x82    /* Endpoint address (endpoint 2 IN) */
#define M_CFG1_I1A1_0_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I1A1_0_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I1A1_0_INTER     		0       /* Endpoint interval */
#define M_CFG1_I1A1_1_EPADDR    		0x02    /* Endpoint address (endpoint 2 OUT) */
#define M_CFG1_I1A1_1_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG1_I1A1_1_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG1_I1A1_1_INTER     		0       /* Endpoint interval */


/* wangqiao add */
/* Interface 2, alternate 0 */
#define M_CFG1_I2A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG1_I2A0_IFCLASS     		0       /* Interface class */
#define M_CFG1_I2A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG1_I2A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG1_I2A0_IFI         		0       /* Interface string index */
#define M_CFG1_I2A0_0_EPADDR    		0x83    /* Endpoint address (endpoint 2 IN) */
#define M_CFG1_I2A0_0_ATTRIB    		0x01    /* Endpoint attributes (ISO) */
#define M_CFG1_I2A0_0_MAXP      		0x0040  /* Endpoint max packet size (64) */
#define M_CFG1_I2A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG1_I2A0_1_EPADDR    		0x03    /* Endpoint address (endpoint 2 OUT) */
#define M_CFG1_I2A0_1_ATTRIB    		0x01    /* Endpoint attributes (ISO) */
#define M_CFG1_I2A0_1_MAXP      		0x0040  /* Endpoint max packet size (64) */
#define M_CFG1_I2A0_1_INTER     		0       /* Endpoint interval */


/* Define values for descriptors in configuration 2 */
#define M_CFG2_NUMIFS           		2       /* Number of interfaces */
#define M_CFG2_CFGI             		0       /* Configuration description string index */
#define M_CFG2_ATTRIBUTES       		0x00    /* Configuration attributes */
#define M_CFG2_MAXPOWER         		0       /* Maximum power (units of 2 mA) */

/* Interface 0, alternate 0 */
#define M_CFG2_I0A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG2_I0A0_IFCLASS     		0       /* Interface class */
#define M_CFG2_I0A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG2_I0A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG2_I0A0_IFI         		0       /* Interface string index */
#define M_CFG2_I0A0_0_EPADDR    		0x81    /* Endpoint address (endpoint 1 IN) */
#define M_CFG2_I0A0_0_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG2_I0A0_0_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG2_I0A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG2_I0A0_1_EPADDR    		0x01    /* Endpoint address (endpoint 1 OUT) */
#define M_CFG2_I0A0_1_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG2_I0A0_1_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG2_I0A0_1_INTER     		0       /* Endpoint interval */

/* Interface 1, alternate 0 */
#define M_CFG2_I1A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG2_I1A0_IFCLASS     		0       /* Interface class */
#define M_CFG2_I1A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG2_I1A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG2_I1A0_IFI         		0       /* Interface string index */
#define M_CFG2_I1A0_0_EPADDR    		0x82    /* Endpoint address (endpoint 2 IN) */
#define M_CFG2_I1A0_0_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG2_I1A0_0_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG2_I1A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG2_I1A0_1_EPADDR    		0x02    /* Endpoint address (endpoint 2 OUT) */
#define M_CFG2_I1A0_1_ATTRIB    		0x02    /* Endpoint attributes (bulk) */
#define M_CFG2_I1A0_1_MAXP      		0x0020  /* Endpoint max packet size (64) */
#define M_CFG2_I1A0_1_INTER     		0       /* Endpoint interval */

/* wangqiao add */
/* Interface 2, alternate 0 */
#define M_CFG2_I2A0_NUMEPS      		2       /* Number of endpoints */
#define M_CFG2_I2A0_IFCLASS     		0       /* Interface class */
#define M_CFG2_I2A0_IFSUBCLASS  		0       /* Interface subclass */
#define M_CFG2_I2A0_IFPROTOCOL  		0       /* Interface protocol */
#define M_CFG2_I2A0_IFI         		0       /* Interface string index */
#define M_CFG2_I2A0_0_EPADDR    		0x83    /* Endpoint address (endpoint 2 IN) */
#define M_CFG2_I2A0_0_ATTRIB    		0x01    /* Endpoint attributes (ISO) */
#define M_CFG2_I2A0_0_MAXP      		0x0040  /* Endpoint max packet size (64) */
#define M_CFG2_I2A0_0_INTER     		0       /* Endpoint interval */
#define M_CFG2_I2A0_1_EPADDR    		0x03    /* Endpoint address (endpoint 2 OUT) */
#define M_CFG2_I2A0_1_ATTRIB    		0x01    /* Endpoint attributes (ISO) */
#define M_CFG2_I2A0_1_MAXP      		0x0040  /* Endpoint max packet size (64) */
#define M_CFG2_I2A0_1_INTER     		0       /* Endpoint interval */


#pragma pack(1)
/* Command structure */
typedef struct{
	uint8_t  	bmRequestType;
	uint8_t  	bRequest;
	uint16_t  	wValue;
	uint16_t  	wIndex;
	uint16_t  	wLength;
}USB_CommandDef;
//typedef USB_CommandDef *PCOMMAND;

/* Standard Device Descriptor */
typedef struct{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint16_t  	bcdUSB;
	uint8_t  	bDeviceClass;
	uint8_t  	bDeviceSubClass;
	uint8_t  	bDeviceProtocol;
	uint8_t  	bMaxPacketSize0;
	uint16_t  	idVendor;
	uint16_t  	idProduct;
	uint16_t  	bcdDevice;
	uint8_t  	iManufacturer;
	uint8_t  	iProduct;
	uint8_t  	iSerialNumber;
	uint8_t  	bNumConfigurations;
}USB_DeviceDscrDef;
//typedef USB_DeviceDscrDef *PSTD_DEV_DSCR;

/* Standard Configuration Descriptor */
typedef struct{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint16_t  	wTotalLength;
	uint8_t  	bNumInterfaces;
	uint8_t  	bConfigurationValue;
	uint8_t  	iConfiguration;
	uint8_t  	bmAttributes;
	uint8_t  	bMaxPower;
}USB_CfgDscrDef;
//typedef USB_CfgDscrDef *PSTD_CFG_DSCR;

/* Standard Interface Descriptor */
typedef struct{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint8_t  	bInterfaceNumber;
	uint8_t  	bAlternateSetting;
	uint8_t  	bNumEndpoints;
	uint8_t  	bInterfaceClass;
	uint8_t  	bInterfaceSubClass;
	uint8_t  	bInterfaceProtocol;
	uint8_t  	iInterface;
}USB_InterfaceDscrDef;
//typedef USB_InterfaceDscrDef *PSTD_IF_DSCR;

/* Standard Endpoint Descriptor */
typedef struct{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint8_t  	bEndpointAddress;
	uint8_t  	bmAttributes;
	uint16_t 	wMaxPacketSize;
	uint8_t  	bInterval;
}USB_EndPointDscrDef;
typedef USB_EndPointDscrDef *PSTD_EP_DSCR;

/* Standard hid Descriptor */
typedef struct{
	uint8_t  	bLength;
	uint8_t  	bDescriptorType;
	uint16_t  	bcdHID;
	uint8_t  	bCountryCode;
	uint8_t  	bNumDescriptors;
	uint8_t  	bDescriptorTypeClass;
	uint16_t 	wDescriptorLength;
}USB_HidDscrDef;

/* hid report Descriptor */

typedef struct{
	uint16_t  				bUsagePage;
	uint16_t  				bUsageMin;
	uint16_t  				bUsageMax;
	uint16_t  				bLogicMin;
	uint16_t  				bLogicMax;
	uint16_t  				bReportCount;
	uint16_t  				bReportSize;
	uint16_t  				bInput;
	uint16_t  				bReportCount1;
	uint16_t  				bReportSize1;
	uint16_t  				bInput1;
}USB_ButtonDscrDef;

typedef struct{
	uint16_t  				bUsagePage;
	uint16_t  				bUsageX;
	uint16_t  				bUsageY;
	uint16_t  				bLogicMin;
	uint16_t  				bLogicMax;
	uint16_t  				bReportCount;
	uint16_t  				bReportSize;
	uint16_t  				bInput;	
}USB_CoordinateDscrDef;
typedef struct{
	uint16_t  				bPointer;
	uint16_t  				bCollection;
	USB_ButtonDscrDef		button;
	USB_CoordinateDscrDef	coord;
	uint16_t  				bEndCollection;
}USB_UsrDscrDef;

typedef struct{
	uint16_t  				bUsagePage;
	uint16_t  				bUsageType;
	uint16_t  				bCollection;
	USB_UsrDscrDef			usr;
	uint16_t  				bEndCollection;
}USB_ReportDscrDef;





/*
  Define configuration structures
  The configuration structures need to be modified to match the required configuration
  Each configuration should start with a configuration descriptor
  followed by 1 or more interface descriptors (each of which is followed by
  endpoint descriptors for all the endpoints in that interface).
  The interface descriptors should start with interface 0, alternate setting 0.
  Followed by any alternate settings for interface 0.
  Then interface 1 (if more than 1 interface in the configuration), followed
  by any alternate settings for interface 1.
  This arrangement should be repeated for all interfaces in the configuration.
*/

/* Configuration 1 */
typedef struct{
	USB_InterfaceDscrDef   stdif;        /* Interface 1 Alternate 1 Descriptor */
//	USB_HidDscrDef		  hid0;
	USB_EndPointDscrDef   stdep_0;      /* 1st Endpoint Descriptor for Interface 1 */
	USB_EndPointDscrDef   stdep_1;      /* 2nd Endpoint Descriptor for Interface 1 */
}USB_CfgDef;
/* Configuration 1 */
typedef struct{
	USB_CfgDscrDef  stdcfg;         /* Required Standard Configuration Descriptor */
	USB_CfgDef	  cfg_00;		  /* Interface 0 Descriptor &Endpoint Descriptor*/
	USB_CfgDef	  cfg_10;		  /* Interface 1 Alternate 0 Descriptor & Endpoint Descriptor */
	USB_CfgDef	  cfg_11;		  /* Interface 1 Alternate 1 Descriptor & Endpoint Descriptor */
	USB_CfgDef	  cfg_20;	     /* Interface 2 Descriptor & Endpoint Descriptor*/	
}__attribute((aligned (4))) USB_ConfigTypeDef;
//typedef USB_ConfigTypeDef *PM_CFG_1;

/* Configuration 2 */
typedef struct{
	USB_CfgDscrDef  stdcfg;         /* Required Standard Configuration Descriptor */
	USB_CfgDef	  cfg_00;		  /* Interface 0 Descriptor &Endpoint Descriptor*/
	USB_CfgDef	  cfg_10;		  /* Interface 1 Alternate 0 Descriptor & Endpoint Descriptor */
	USB_CfgDef	  cfg_20;	     /* Interface 2 Descriptor & Endpoint Descriptor*/
}__attribute((aligned (4))) USB_ConfigTypeDef_2;
//typedef USB_ConfigTypeDef_2 *PM_CFG_2;

/* Top level structure containing pointers to each configuration */
typedef struct{
	USB_ConfigTypeDef 		*pcfg1;          /* Pointer to M_CFG_1 */
	USB_ConfigTypeDef_2		*pcfg2;          /* Pointer to M_CFG_2 */
}__attribute((aligned (4))) USB_ConfigPointTypeDef;
//typedef M_CFG *PM_CFG;

extern USB_ConfigTypeDef  		cfg1;
extern USB_ConfigTypeDef_2  	cfg2;
extern USB_ConfigPointTypeDef    cfg;
extern USB_DeviceDscrDef	stddevdsc;
extern USB_HidDscrDef gHidDsc;

#endif /* _DESCRIPT_H_ */

