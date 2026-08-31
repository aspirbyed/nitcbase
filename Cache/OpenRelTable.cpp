#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table ****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry attrRelCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &attrRelCacheEntry.relCatEntry);
  attrRelCacheEntry.recId.block = RELCAT_BLOCK;
  attrRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrRelCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  for(int i=0; i < RELCAT_NO_ATTRS; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);

    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    attrCacheEntry->next = nullptr;

    // link the entries together
    if(i == 0) {
      AttrCacheTable::attrCache[RELCAT_RELID] = attrCacheEntry; // head of the linked list
    } else {
      struct AttrCacheEntry* prevEntry = AttrCacheTable::attrCache[RELCAT_RELID];
      while(prevEntry->next != nullptr) {
        prevEntry = prevEntry->next;
      }
      prevEntry->next = attrCacheEntry;
    }
  }

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  for(int i=6; i < 12; i++){
    attrCatBlock.getRecord(attrCatRecord, i);

    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    attrCacheEntry->next = nullptr;

    // link the entries together
    if(i == 6) {
      AttrCacheTable::attrCache[ATTRCAT_RELID] = attrCacheEntry; // head of the linked list
    } else {
      struct AttrCacheEntry* prevEntry = AttrCacheTable::attrCache[ATTRCAT_RELID];
      while(prevEntry->next != nullptr) {
        prevEntry = prevEntry->next;
      }
      prevEntry->next = attrCacheEntry;
    }
  }

  /* 
   * Following code is only for Exercise 1 in the Stage 3, 
   * where you've to add entries for Student relation into the cache.
   */

  /**** setting up Student relation in the Relation Cache Table ****/
  RecBuffer studentRelCatBlock(RELCAT_BLOCK);
  Attribute studentRelCatRecord[RELCAT_NO_ATTRS];
  studentRelCatBlock.getRecord(studentRelCatRecord, 2); // slot 3 has the Student relation

  struct RelCacheEntry studentRelCacheEntry;
  RelCacheTable::recordToRelCatEntry(studentRelCatRecord, &studentRelCacheEntry.relCatEntry);
  studentRelCacheEntry.recId.block = RELCAT_BLOCK;
  studentRelCacheEntry.recId.slot = 2;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[2] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[2]) = studentRelCacheEntry;

  /**** setting up Student relation in the Attribute Cache Table ****/
  // set up the attributes of the Student relation similarly.
  RecBuffer studentAttrCatBlock(ATTRCAT_BLOCK);
  Attribute studentAttrCatRecord[ATTRCAT_NO_ATTRS];
  for(int i=12; i < 16; i++){ // slots 12-15 have the attributes of Student relation
    studentAttrCatBlock.getRecord(studentAttrCatRecord, i);

    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(studentAttrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    attrCacheEntry->next = nullptr;

    // link the entries together
    if(i == 12) {
      AttrCacheTable::attrCache[2] = attrCacheEntry; // head of the linked list for Student relation
    } else {
      struct AttrCacheEntry* prevEntry = AttrCacheTable::attrCache[2];
      while(prevEntry->next != nullptr) {
        prevEntry = prevEntry->next;
      }
      prevEntry->next = attrCacheEntry;
    }
  }
}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
  for (int i = 0; i < MAX_OPEN; ++i) {
    if (RelCacheTable::relCache[i] != nullptr) {
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i] = nullptr;
    }

    struct AttrCacheEntry* currentEntry = AttrCacheTable::attrCache[i];
    while (currentEntry != nullptr) {
      struct AttrCacheEntry* nextEntry = currentEntry->next;
      free(currentEntry);
      currentEntry = nextEntry;
    }
    AttrCacheTable::attrCache[i] = nullptr;
  }
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  if (strcmp(relName, RELCAT_RELNAME) == 0) {
    return RELCAT_RELID;
  }
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return ATTRCAT_RELID;
  }

  if (strcmp(relName, "Students") == 0) {
    return 2; // Student relation has relId 2
  }

  return E_RELNOTOPEN;
}