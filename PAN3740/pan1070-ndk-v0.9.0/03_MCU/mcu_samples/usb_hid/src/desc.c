/**************************************************************************//**
* @file     desc.c
* @version  V1.00
* $Revision: 2 $
* $Date: 20/08/04 14:25 $
* @brief    Initialise descriptors for MUSBFSFC firmware.
*
* @note
* Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "musbfsfc.h"
#include "endpoint0.h"
#include "descript.h"
#include "usb.h"

//#pragma arm section code = "RAMCODE"
/* Device descriptor is global */
USB_DeviceDscrDef            stddevdsc;

/* Configuration structures as defined in config.h */
USB_ConfigTypeDef 		cfg1;
USB_ConfigTypeDef_2 	cfg2;
USB_ConfigPointTypeDef   cfg;
USB_HidDscrDef gHidDsc;

void DeviceDscInit(void)
{
	/* Initialise the Standard Device Descriptor */
	stddevdsc.bLength               = sizeof(USB_DeviceDscrDef);
	stddevdsc.bDescriptorType       = M_DST_DEVICE;
	stddevdsc.bcdUSB                = 0x0200;       /* USB spec rev is 2.0 */
	stddevdsc.bMaxPacketSize0       = M_EP0_MAXP;
	stddevdsc.idVendor              = 0x046d;//0x0501;
	stddevdsc.idProduct             = 0xc018;//0x5043;
	stddevdsc.bcdDevice             = 0x4301;//0x0057;
	stddevdsc.iManufacturer         = 1;
	stddevdsc.iProduct              = 2;
	stddevdsc.iSerialNumber         = 3;
	stddevdsc.bDeviceClass          = M_DEVDSC_CLASS;
	stddevdsc.bDeviceSubClass       = M_DEVDSC_SUBCLASS;
	stddevdsc.bDeviceProtocol       = M_DEVDSC_PROTOCOL;
	stddevdsc.bNumConfigurations    = 0x01;	
}

#define	INIT_CFG_1	(1)
#define	INIT_CFG_2	(2)

void ConfigDscInit(USB_CfgDscrDef *config,uint8_t idx)
{
	/* Initialise the descriptors for configuration 1 */
	config->bLength             = sizeof(USB_CfgDscrDef);
	config->bDescriptorType     = M_DST_CONFIG;
	if(INIT_CFG_1 == idx){
		config->wTotalLength        = sizeof(USB_ConfigTypeDef);
		config->bNumInterfaces      = 3;
		config->bConfigurationValue = 1;
		config->iConfiguration      = M_CFG1_CFGI;
		config->bmAttributes        = M_CFG1_ATTRIBUTES;
		config->bMaxPower           = M_CFG1_MAXPOWER;
	}
	if(INIT_CFG_2 == idx){
		config->wTotalLength        = sizeof(USB_ConfigTypeDef_2);
		config->bNumInterfaces      = 3;
		config->bConfigurationValue = 2;
		config->iConfiguration      = M_CFG2_CFGI;
		config->bmAttributes        = M_CFG2_ATTRIBUTES;
		config->bMaxPower           = M_CFG2_MAXPOWER;	
	}
}

void InterfaceDscInit(USB_InterfaceDscrDef *ifs,uint32_t ifs_alt_ep,uint32_t class_type)
{
	ifs->bLength            		= sizeof(USB_InterfaceDscrDef);
	ifs->bDescriptorType    		= M_DST_INTERFACE;
	ifs->bInterfaceNumber   		= (ifs_alt_ep >> 16)&0xff;
	ifs->bAlternateSetting  		= (ifs_alt_ep >> 8)&0xff;
	ifs->bNumEndpoints      		= ifs_alt_ep&0xff;
	ifs->bInterfaceClass    		= (class_type >> 24)&0xff;
	ifs->bInterfaceSubClass 		= (class_type >> 16)&0xff;
	ifs->bInterfaceProtocol 		= (class_type >> 8)&0xff;
	ifs->iInterface         		= class_type&0xff;
}

void EndPointDscInit(USB_EndPointDscrDef *ep,uint32_t addr_attr_invl,uint16_t max_pkt_size)
{
	ep->bLength          = sizeof(USB_EndPointDscrDef);
	ep->bDescriptorType  = M_DST_ENDPOINT;
	ep->bEndpointAddress = (addr_attr_invl >> 16)&0xff;
	ep->bmAttributes     = (addr_attr_invl >> 8)&0xff;
	ep->bInterval        = addr_attr_invl&0xff;
	ep->wMaxPacketSize   = max_pkt_size;
}


/****************************************
  Initialise the descriptors
  This routine is called after a USB reset to set the descriptors.
****************************************/
void InitialiseDesc(void)
{
	uint32_t ifs_alt_ep,class_type;
	uint32_t addr_attr_invl;
	
	DeviceDscInit();
	cfg.pcfg1 = &cfg1;
	cfg.pcfg2 = &cfg2;

	
	/* Initialise the descriptors for configuration 1 */
	ConfigDscInit(&cfg1.stdcfg,INIT_CFG_1);
	
	/* .. Interface 0 */
	ifs_alt_ep = (0<<16)|(0<<8)|M_CFG1_I0A0_NUMEPS;
	class_type = (M_CFG1_I0A0_IFCLASS<<24)|(M_CFG1_I0A0_IFSUBCLASS<<16)|(M_CFG1_I0A0_IFPROTOCOL<<8)| M_CFG1_I0A0_IFI;
	InterfaceDscInit(&cfg1.cfg_00.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG1_I0A0_0_EPADDR << 16)|(M_CFG1_I0A0_0_ATTRIB << 8)| M_CFG1_I0A0_0_INTER;
	EndPointDscInit(&cfg1.cfg_00.stdep_0,addr_attr_invl,M_CFG1_I0A0_0_MAXP);
	addr_attr_invl = (M_CFG1_I0A0_1_EPADDR << 16)|(M_CFG1_I0A0_1_ATTRIB << 8)| M_CFG1_I0A0_1_INTER;
	EndPointDscInit(&cfg1.cfg_00.stdep_1,addr_attr_invl,M_CFG1_I0A0_1_MAXP);	
	
	/* .. Interface 1, Alternate 0 */
	ifs_alt_ep = (1<<16)|(0<<8)|M_CFG1_I1A0_NUMEPS;
	class_type = (M_CFG1_I1A0_IFCLASS<<24)|(M_CFG1_I1A0_IFSUBCLASS<<16)|(M_CFG1_I1A0_IFPROTOCOL<<8)| M_CFG1_I1A0_IFI;
	InterfaceDscInit(&cfg1.cfg_10.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG1_I1A0_0_EPADDR << 16)|(M_CFG1_I1A0_0_ATTRIB << 8)| M_CFG1_I1A0_0_INTER;
	EndPointDscInit(&cfg1.cfg_10.stdep_0,addr_attr_invl,M_CFG1_I1A0_0_MAXP);
	addr_attr_invl = (M_CFG1_I1A0_1_EPADDR << 16)|(M_CFG1_I1A0_1_ATTRIB << 8)| M_CFG1_I1A0_1_INTER;
	EndPointDscInit(&cfg1.cfg_10.stdep_1,addr_attr_invl,M_CFG1_I1A0_1_MAXP);
	
	/* .. Interface 1, Alternate 1 */
	ifs_alt_ep = (1<<16)|(1<<8)|M_CFG1_I1A1_NUMEPS;
	class_type = (M_CFG1_I1A1_IFCLASS<<24)|(M_CFG1_I1A1_IFSUBCLASS<<16)|(M_CFG1_I1A1_IFPROTOCOL<<8)| M_CFG1_I1A1_IFI;
	InterfaceDscInit(&cfg1.cfg_11.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG1_I1A1_0_EPADDR << 16)|(M_CFG1_I1A1_0_ATTRIB << 8)| M_CFG1_I1A1_0_INTER;
	EndPointDscInit(&cfg1.cfg_11.stdep_0,addr_attr_invl,M_CFG1_I1A1_0_MAXP);
	addr_attr_invl = (M_CFG1_I1A1_1_EPADDR << 16)|(M_CFG1_I1A1_1_ATTRIB << 8)| M_CFG1_I1A1_1_INTER;
	EndPointDscInit(&cfg1.cfg_11.stdep_1,addr_attr_invl,M_CFG1_I1A1_1_MAXP);

	/* interface2*/
	/* .. Interface 0 */
	ifs_alt_ep = (2<<16)|(0<<8)|M_CFG1_I2A0_NUMEPS;
	class_type = (M_CFG1_I2A0_IFCLASS<<24)|(M_CFG1_I2A0_IFSUBCLASS<<16)|(M_CFG1_I2A0_IFPROTOCOL<<8)| M_CFG1_I2A0_IFI;
	InterfaceDscInit(&cfg1.cfg_20.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG1_I2A0_0_EPADDR << 16)|(M_CFG1_I2A0_0_ATTRIB << 8)| M_CFG1_I2A0_0_INTER;
	EndPointDscInit(&cfg1.cfg_20.stdep_0,addr_attr_invl,M_CFG1_I2A0_0_MAXP);
	addr_attr_invl = (M_CFG1_I2A0_1_EPADDR << 16)|(M_CFG1_I2A0_1_ATTRIB << 8)| M_CFG1_I2A0_1_INTER;
	EndPointDscInit(&cfg1.cfg_20.stdep_1,addr_attr_invl,M_CFG1_I2A0_1_MAXP);
	
	
	
	/* Initialise the descriptors for configuration 2 */
	ConfigDscInit(&cfg2.stdcfg,INIT_CFG_2);
	/* .. Interface 0 */
	ifs_alt_ep = (0<<16)|(0<<8)|M_CFG2_I0A0_NUMEPS;
	class_type = (M_CFG2_I0A0_IFCLASS<<24)|(M_CFG2_I0A0_IFSUBCLASS<<16)|(M_CFG2_I0A0_IFPROTOCOL<<8)| M_CFG2_I0A0_IFI;
	InterfaceDscInit(&cfg2.cfg_00.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG2_I0A0_0_EPADDR << 16)|(M_CFG2_I0A0_0_ATTRIB << 8)| M_CFG2_I0A0_0_INTER;
	EndPointDscInit(&cfg2.cfg_00.stdep_0,addr_attr_invl,M_CFG2_I0A0_0_MAXP);
	addr_attr_invl = (M_CFG2_I0A0_1_EPADDR << 16)|(M_CFG2_I0A0_1_ATTRIB << 8)| M_CFG2_I0A0_1_INTER;
	EndPointDscInit(&cfg2.cfg_00.stdep_1,addr_attr_invl,M_CFG2_I0A0_1_MAXP);
	
	/* .. Interface 1, Alternate 0 */
	ifs_alt_ep = (1<<16)|(0<<8)|M_CFG2_I1A0_NUMEPS;
	class_type = (M_CFG2_I1A0_IFCLASS<<24)|(M_CFG2_I1A0_IFSUBCLASS<<16)|(M_CFG2_I1A0_IFPROTOCOL<<8)| M_CFG2_I1A0_IFI;
	InterfaceDscInit(&cfg2.cfg_10.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG2_I1A0_0_EPADDR << 16)|(M_CFG2_I1A0_0_ATTRIB << 8)| M_CFG2_I1A0_0_INTER;
	EndPointDscInit(&cfg2.cfg_10.stdep_0,addr_attr_invl,M_CFG2_I1A0_0_MAXP);
	addr_attr_invl = (M_CFG2_I1A0_1_EPADDR << 16)|(M_CFG2_I1A0_1_ATTRIB << 8)| M_CFG2_I1A0_1_INTER;
	EndPointDscInit(&cfg2.cfg_10.stdep_1,addr_attr_invl,M_CFG2_I1A0_1_MAXP);
	
	/* .. Interface 2 */
	ifs_alt_ep = (2<<16)|(0<<8)|M_CFG2_I2A0_NUMEPS;
	class_type = (M_CFG2_I2A0_IFCLASS<<24)|(M_CFG2_I2A0_IFSUBCLASS<<16)|(M_CFG2_I2A0_IFPROTOCOL<<8)| M_CFG2_I2A0_IFI;
	InterfaceDscInit(&cfg2.cfg_20.stdif,ifs_alt_ep,class_type);
	
	/* .. Endpoints */
	addr_attr_invl = (M_CFG2_I2A0_0_EPADDR << 16)|(M_CFG2_I2A0_0_ATTRIB << 8)| M_CFG2_I2A0_0_INTER;
	EndPointDscInit(&cfg2.cfg_20.stdep_0,addr_attr_invl,M_CFG2_I2A0_0_MAXP);
	addr_attr_invl = (M_CFG2_I2A0_1_EPADDR << 16)|(M_CFG2_I2A0_1_ATTRIB << 8)| M_CFG2_I2A0_1_INTER;
	EndPointDscInit(&cfg2.cfg_20.stdep_1,addr_attr_invl,M_CFG2_I2A0_1_MAXP);	
}
