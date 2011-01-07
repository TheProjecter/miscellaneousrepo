import Ports, Bochs, Panic, memory/MM

FloppyPorts: enum  {
        PORT_STATUSA    = 0x0,
        PORT_STATUSB    = 0x1,
        PORT_DIGOUTPUT  = 0x2,
        PORT_MAINSTATUS = 0x4,
        PORT_DATARATE   = 0x4,
        PORT_DATA               = 0x5,
        PORT_DIGINPUT   = 0x7,
        PORT_CONFIGCTRL = 0x7
}

FloppyCommands: enum {
        FIX_DRIVE_DATA  = 0x03,
        HECK_DRIVE_STATUS       = 0x04,
        CALIBRATE_DRIVE = 0x07,
        CHECK_INTERRUPT_STATUS = 0x08,
        SEEK_TRACK              = 0x0F,
        READ_SECTOR_ID  = 0x4A,
        FORMAT_TRACK    = 0x4D,
        READ_TRACK              = 0x42,
        READ_SECTOR             = 0x66,
        WRITE_SECTOR    = 0xC5,
        WRITE_DELETE_SECTOR     = 0xC9,
        READ_DELETE_SECTOR      = 0xCC
}

FDD: cover {
	fddTypes := static ["None",
		"360kB 5.25\"",
		"1.2MB 5.25\"",
 		"720kB 3.5\"",
		"1.44MB 3.5\"",
		"2.88MB 3.5\""]

	fddSizes := static [0,
		360*1024,
		1200*1024,
		720*1024,
		1440*1024,
		2880*1024]

	setup: static func {
		Bochs debug("addr1: 0x%08x addr2: 0x%08x" format(fddTypes, fddTypes[0]))

		// determine floppy types
        	Ports outByte(0x70, 0x10)
        	data := Ports inByte(0x71)
		Bochs debug("disk 0: %s, disk 1: %s" format(fddTypes[(data>>4)], fddTypes[(data & 0xF)]))
/*
		sensInt(0x3F0, null, null);

		// Install IRQ6 Handler
		IRQ_AddHandler(6, FDD_IRQHandler);
		// Reset Primary FDD Controller
		FDD_Reset(0);

		// Initialise Root Node
		gFDD_DriverInfo.RootNode.CTime = gFDD_DriverInfo.RootNode.MTime
		        = gFDD_DriverInfo.RootNode.ATime = now();

		// Initialise Child Nodes
		gFDD_Devices[0].Node.Inode = 0;
		gFDD_Devices[0].Node.Flags = 0;
		gFDD_Devices[0].Node.NumACLs = 0;
		gFDD_Devices[0].Node.Read = FDD_ReadFS;
		gFDD_Devices[0].Node.Write = NULL;//FDD_WriteFS;
		memcpy(&gFDD_Devices[1].Node, &gFDD_Devices[0].Node, sizeof(tVFS_Node));

		gFDD_Devices[1].Node.Inode = 1;

		// Set Lengths
		gFDD_Devices[0].Node.Size = cFDD_SIZES[data >> 4];
		gFDD_Devices[1].Node.Size = cFDD_SIZES[data & 0xF];

		// Create Sector Cache
		if( cFDD_SIZES[data >> 4] )
		{
		        gFDD_Devices[0].CacheHandle = IOCache_Create(
		                FDD_WriteSector, 0, 512,
		                gFDD_Devices[0].Node.Size / (512*4)
		                );      // Cache is 1/4 the size of the disk
		}
		if( cFDD_SIZES[data & 15] ) {
		        gFDD_Devices[1].CacheHandle = IOCache_Create(
		                FDD_WriteSector, 0, 512,
		                gFDD_Devices[1].Node.Size / (512*4)
		                );      // Cache is 1/4 the size of the disk
		}

		// Register with devfs
		DevFS_AddDevice(&gFDD_DriverInfo);
*/
	}
/*
void FDD_SensInt(int base, Uint8 *sr0, Uint8 *cyl)
{
        FDD_int_SendByte(base, CHECK_INTERRUPT_STATUS);
        if(sr0) *sr0 = FDD_int_GetByte(base);
        else    FDD_int_GetByte(base);
        if(cyl) *cyl = FDD_int_GetByte(base);
        else    FDD_int_GetByte(base);
}

void FDD_int_SendByte(int base, char byte)
{
        volatile int state;
        int timeout = 128;
        for( ; timeout--; )
        {
            state = inb(base + PORT_MAINSTATUS);
            if ((state & 0xC0) == 0x80)
            {
                outb(base + PORT_DATA, byte);
                return;
            }
            inb(0x80);  //Delay
        }

        #if WARN
        Warning("FDD_int_SendByte - Timeout sending byte 0x%x to base 0x%x\n", byte, base);
        #endif
}

int FDD_int_GetByte(int base)
{
        volatile int state;
        int timeout;
        for( timeout = 128; timeout--; )
        {
            state = inb((base + PORT_MAINSTATUS));
            if ((state & 0xd0) == 0xd0)
                    return inb(base + PORT_DATA);
            inb(0x80);
        }
        return -1;
}

*/

}
