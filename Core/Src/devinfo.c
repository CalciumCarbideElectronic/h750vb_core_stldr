#include "devinfo.h"

#if defined (__ICCARM__)
__root struct StorageInfo const StorageInfo  =  {
#else
struct StorageInfo const StorageInfo  =  {
#endif
   "Core_STM32H750-QSPI", 	 	 // Device Name + version number
   SPI_FLASH,                  					 // Device Type
   0x90000000, // Device Start Address
   0x00200000, //2MB
   0x00000100,                 						 // Programming Page Size 16Bytes
   0xFF,                       						 // Initial Content of Erased Memory
// Specify Size and Address of Sectors (view example below)
   0x00000020, // use 32 sector
   0x00010000, // 64KB Sectors
   0x00000000, 0x00000000,

};

