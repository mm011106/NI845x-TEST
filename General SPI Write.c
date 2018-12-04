/*
General SPI Write.c
テスト用
Based on the sample program in the driver of NI845x

2018/12 miyamoto


*/

#include <stdio.h>     // Include file for printf
#include <stdlib.h>    // Include file for strtol
#include <string.h>
#include <windows.h>   // Include file for Win32 time functions
#include "ni845x.h"    // Include file for NI-485x functions and constants

/* the NI 845x handles */
NiHandle DeviceHandle;
NiHandle SPIHandle;

/*  error Function for NI 845x */
#ifndef errChk
#define errChk(fCall) if (Error = (fCall), Error < 0) {goto Error;} else
#endif

int main ()
{
   int    Error            = 0;

   char   FirstDevice[260];       // 260 characters for 845x resource name
   uInt16 ClockRate       = 1000; // clock rate in KHz (1000)
   uInt32 ChipSelect      = 0;    // chip select pin (0)

   uInt32 WriteSize       = 0;
   uInt8  SendData[515];          // array for total bytes to write
   uInt32 ReadSize        = 0;
   uInt8  ReadData[35];           // read array for bytes to read

   uInt8  Delay           = 1;    // delay time between pages
   uInt8  i;

   char   ErrorMsg[1024];



   printf ("\n\nSearching for Devices\n\n");

   /* find first device */
   errChk (ni845xFindDevice (FirstDevice, NULL , NULL));

   /* open device handle */
   errChk (ni845xOpen (FirstDevice, &DeviceHandle));
   printf("Found following device to initialize: %s \n", FirstDevice);

   /* Set the I/O Voltage Level */
   errChk (ni845xSetIoVoltageLevel (DeviceHandle, kNi845x33Volts));

   /* create configuration reference */
   errChk (ni845xSpiConfigurationOpen (&SPIHandle));

   /* configure configuration properties */
   errChk (ni845xSpiConfigurationSetChipSelect (SPIHandle, ChipSelect));
   errChk (ni845xSpiConfigurationSetClockRate (SPIHandle,ClockRate));
   errChk (ni845xSpiConfigurationSetClockPolarity (SPIHandle,
      kNi845xSpiClockPolarityIdleLow));
   errChk (ni845xSpiConfigurationSetClockPhase (SPIHandle,
      kNi845xSpiClockPhaseFirstEdge));


   printf (" ....initialized successfully\n\n");


   for (i = 0; i < 10; i++)
   {
	  printf("counter: %d \n", i);

      WriteSize = 4;

      SendData[0] = 0x06;
	  SendData[1] = 0x12;
	  SendData[2] = 0x34;
	  SendData[3] = 0x56;

      errChk (ni845xSpiWriteRead (DeviceHandle, SPIHandle, WriteSize,
         SendData, &ReadSize, ReadData));

      Sleep(Delay); // allow the EEPROM to internaly
                    // write the data to the target
   }

   printf ("\nData written !\n\n");

   // close the configuration and device handles
   errChk (ni845xSpiConfigurationClose (SPIHandle));
   errChk (ni845xClose (DeviceHandle));

Error:
   if (Error < 0)
   {
      ni845xStatusToString(Error, 1024, ErrorMsg);
      printf ("\nError %d %s \n", Error, ErrorMsg);
      ni845xSpiConfigurationClose (SPIHandle);
      ni845xClose (DeviceHandle);
      exit(1);
   }

   return 0;
}

