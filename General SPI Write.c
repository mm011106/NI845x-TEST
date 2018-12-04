/*
General SPI Write.c
テスト用
This example shows a more generalized way to use the SPI basic API
to write data.

It allows the user to change the following parameters: page size, write delay,
the Endianness of the address bytes, and the number of bytes in an address.

The parameters will have the following effects:

   Page size: The memory of an EEPROM is broken apart into pages, each having
   a size of page size. Writes to an EERPOM are typically limited to one page
   at a time. The example will automatically break the bytes to write into
   pages based on this attribute.

   Write delay: After copy the bytes to the EEPROM, there is typically a
   programming time required to allow the memory to be copied to the
   nonvolatile memory.This delay allows for this operation to complete.

   Number of Address Bytes: This will allow the user to specify the starting
   address as either 1 or 2 bytes.

   Endianness: This parameter will change the ordering of the bytes in the
   address, note that this only has an effect if there are 2 bytes in the
   address.

For connection information see the NI-845x Hardware and Software manual and the
manual for your specific EEPROM.

Step 1: Search for devices and display them.
     a) Open a device handle.

Step 2: Create an SPI configuration to describe the communication requirements
        between the NI-845x device and the EEPROM.
     a) Many EEPROMs use SPI Mode 0, which is the polarity and phase
        both being set to zero, however this is not always true.
        These may need to be adjusted towork with other EEPROMs.
     b) The Clock rate and Chip Select lines are configurable by the user.

Step 3: Write enable the EEPROM.
     a) This is accomplished by writing the WREN Instruction (0x6)
        to the device.

Step 4: Generate and write the data to the EEPROM.
        The format for many EEPROMs is:
     a) The first byte is 0x2, which is the write opcode.
        For 1 Byte addressing some EEPROMs require the 4th bit of the opcode
        to be the 9th bit of the address, others don't care.
     b) The next two bytes or byte are the address to write to.
        Check the correct endianess for your device to sent in
        big endian format (MSB first) or little endian format (LSB first).
     c) The next x bytes sent to the device are the bytes that are to written
        to the EEPROM.
        For this example, we are creating an array of x sequential bytes
        (0, 1, ...) up to the write size.
        Many EEPROMs, when they exceed the page size will simply begin
        to write at the beginning of the page boundary,
        thus the write must be split into page aligned writes.

Note: A function calculates the correct EEPROM start addresses
      for multiple page writes.

Step 5: Close the configuration and check for errors.
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

/* Struct definition for CalculatePageInformation function */
typedef struct {
   uInt16   Address;
   uInt16   Length;
   uInt16   Index;
} sPageInfo;

/* Calculate Multiple page information */
void CalculatePageInformation (
   uInt16      PageSize,
   uInt16      NumberToWrite,
   uInt16      StartingOffset,
   sPageInfo * pPageInfo,
   uInt16    * pNumPages
   );

int main ()
{
   int    Error            = 0;
   char   FirstDevice[260];       // 260 characters for 845x resource name
   uInt16 ClockRate       = 1000; // clock rate in KHz (1000)
   uInt32 ChipSelect      = 0;    // chip select pin (0)
   uInt16 NumberToWrite   = 512;  // number of bytes to write (512)
   uInt16 StartingOffset  = 0;    // start address (EEPROM Target)
   uInt32 WriteSize       = 0;
   uInt8  WriteData[512];         // array for data bytes to write
   uInt8  SendData[515];          // array for total bytes to write
   uInt16 PageSize        = 32;   // page size for the EEPROM
   uInt8  Delay           = 5;    // delay time between pages, must be lognger than 5 ms
   uInt8  AddressBytes    = 1;    // 1 addressbyte (0), 2 addressbytes (1)
   uInt8  LowAddressByte  = 0;
   uInt8  HighAddressByte = 0;
   uInt8  Endianness      = 0;    // big endian (0), little endian (1)
   uInt32 ReadSize        = 0;
   uInt8  ReadData[35];           // read array for bytes to read
   uInt16 TotalNumberOfPages = 0;
   sPageInfo PageInfo[32] = {0};  // array of cluster for multiple pageinfo
   uInt16 i;
   uInt16 j;
   char   CharBuff[10];
   char   ErrorMsg[1024];

   /* calculate values for page info and the total number of pages */
   CalculatePageInformation (PageSize, NumberToWrite, StartingOffset,
      PageInfo, &TotalNumberOfPages);

   /* Create the data array to write  */
   printf ("\n\nData to write: \n\n");

   for (j = 0; j < (NumberToWrite); j++)
   {
      WriteData[j] = (1 * j);
      sprintf (&CharBuff[0], "%3X", WriteData[j]);
      printf ("%s\n", CharBuff);
   }

   printf ("\n\nSearching for Devices\n\n");

   /* find first device */
   errChk (ni845xFindDevice (FirstDevice, NULL , NULL));

   /* open device handle */
   errChk (ni845xOpen (FirstDevice, &DeviceHandle));

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

   printf ("%s", FirstDevice);
   printf (" initialized successfully\n");

   /* write number of pages to the EEPROM Target
   calculate addressbytes depending on endianness and byte number*/
   for (i = 0; i < TotalNumberOfPages; i++)
   {
      if (AddressBytes == 1)  // for 2 address bytes
      {
         if (Endianness == 0) // for big endian
         {  // convert to 8-bit
            LowAddressByte = PageInfo[i].Address & 0xFF;
            HighAddressByte = PageInfo[i].Address >> 8;
         }
         else
         {  // for little endian rotate the high andlow part
            LowAddressByte = PageInfo[i].Address >> 8;
            HighAddressByte = PageInfo[i].Address & 0xFF;//
         }
      }
      else      // for 1 address bytes
      {
         // for 1 address bytes endianess is not an issue
         HighAddressByte = PageInfo[i].Address & 0xFF;
      }

      /* Write the write enable (WREN) instruction */
      WriteSize = 4;
      SendData[0] = 0x6;
	  SendData[1] = 0x12;
	  SendData[2] = 0x34;
	  SendData[3] = 0x56;

      errChk (ni845xSpiWriteRead (DeviceHandle, SPIHandle, WriteSize,
         SendData, &ReadSize, ReadData));


 /* Write the SPI data */

/*
      WriteSize = PageInfo[i].Length + AddressBytes + 2;
      SendData[0] = 0x2;              // 0x2 write instruction byte
      SendData[1] = HighAddressByte;  // first address byte
      SendData[2] = LowAddressByte;   // second AddressByte

      // copy the write data to the SendData array
      memcpy (&SendData[AddressBytes + 2], &WriteData[PageInfo[i].Index],
         PageInfo[i].Length);

      errChk (ni845xSpiWriteRead (DeviceHandle, SPIHandle,
         WriteSize, SendData, &ReadSize, ReadData));
*/

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

/*Calculate page informations for multiple page writes*/
void CalculatePageInformation (
   uInt16      PageSize,
   uInt16      NumberToWrite,
   uInt16      StartingOffset,
   sPageInfo * pPageInfo,
   uInt16    * pNumPages
   )
{
   uInt16 x;
   uInt16 currentAddress       = 0;
   uInt16 addressForLastByte   = 0;
   uInt16 addressOfNextPage    = 0;
   uInt16 remainingBytes       = 0;
   uInt16 bytesRemainingOnPage = 0;

   /* calculate values for address, datalength and index for multiple pages */
   remainingBytes = NumberToWrite;
   currentAddress = StartingOffset;
   addressForLastByte = remainingBytes + currentAddress - 1;

   x = 0;
   do
   {
      pPageInfo[x].Address = currentAddress;
      pPageInfo[x].Index   = NumberToWrite - remainingBytes;
      addressOfNextPage = (currentAddress / PageSize + 1) * PageSize;

      // Remaining Data fits in current page
      if(addressForLastByte < addressOfNextPage)
      {
         pPageInfo[x].Length = remainingBytes;
         remainingBytes = 0;
      }
      else
      {
         // Finish current page
         bytesRemainingOnPage = addressOfNextPage - currentAddress;
         pPageInfo[x].Length = bytesRemainingOnPage;

         // Adjust variables
         remainingBytes = remainingBytes - bytesRemainingOnPage;
         currentAddress = addressOfNextPage;
      }

      x++;

   } while (remainingBytes != 0);

   *pNumPages = x;
}
