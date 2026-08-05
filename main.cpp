#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <string.h>

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  // // create objects for the relation catalog and attribute catalog
  // RecBuffer relCatBuffer(RELCAT_BLOCK);
  // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  // HeadInfo relCatHeader;
  // HeadInfo attrCatHeader;

  // // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // relCatBuffer.getHeader(&relCatHeader);

  // for (int i = 0; i < relCatHeader.numEntries; i++) {
  //   attrCatBuffer = RecBuffer(ATTRCAT_BLOCK);
  //   attrCatBuffer.getHeader(&attrCatHeader);

  //   Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog
  //   relCatBuffer.getRecord(relCatRecord, i);

  //   HeadInfo currentAttrCatHeader = attrCatHeader;

  //   printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

  //   do {
  //     for (int j = 0; j < currentAttrCatHeader.numEntries; j++) {
  //       // declare attrCatRecord and load the attribute catalog entry into it
  //       Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  //       attrCatBuffer.getRecord(attrCatRecord, j);

  //       if (strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal) == 0) {
  //         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
  //         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
  //       }
  //     }

  //     if (currentAttrCatHeader.rblock != -1) {
  //       attrCatBuffer = RecBuffer(currentAttrCatHeader.rblock);
  //       attrCatBuffer.getHeader(&currentAttrCatHeader);
  //     } else {
  //       break;
  //     }
  //   } while (true);

  //   printf("\n");
  // }


  for (int i = 0; i < 3; i++) { // i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)
    RelCatEntry relCatBuf;
    AttrCatEntry attrCatBuf;
    RelCacheTable::getRelCatEntry(i, &relCatBuf);
    printf("Relation: %s\n", relCatBuf.relName);

    for(int j=0; j < relCatBuf.numAttrs; j++) {
      AttrCacheTable::getAttrCatEntry(i, j, &attrCatBuf);
      const char *attrType = attrCatBuf.attrType == NUMBER ? "NUM" : "STR";
      printf("  %s: %s\n", attrCatBuf.attrName, attrType);
    }
    printf("\n");
  }

  return 0;
}
