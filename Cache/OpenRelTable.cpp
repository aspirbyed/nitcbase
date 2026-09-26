#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialise all values in relCache and attrCache to be nullptr and all entries
  // in tableMetaInfo to be free
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    tableMetaInfo[i].free = true;
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

  /************ Setting up tableMetaInfo entries ************/
  tableMetaInfo[RELCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

OpenRelTable::~OpenRelTable() {
  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      closeRel(i); // we will implement this function later
    }
  }

  // free the memory allocated for rel-id 0 and 1 in both the relation and attribute caches

  // releasing the relation cache entry of the attribute catalog
  free(RelCacheTable::relCache[ATTRCAT_RELID]);
  RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;

  // releasing the relation cache entry of the relation catalog
  free(RelCacheTable::relCache[RELCAT_RELID]);
  RelCacheTable::relCache[RELCAT_RELID] = nullptr;

  // free the memory allocated for the attribute cache entries of the
  // relation catalog and the attribute catalog
  struct AttrCacheEntry* attrCacheEntry = AttrCacheTable::attrCache[RELCAT_RELID];
  while (attrCacheEntry != nullptr) {
    struct AttrCacheEntry* nextEntry = attrCacheEntry->next;
    free(attrCacheEntry);
    attrCacheEntry = nextEntry;
  }
  AttrCacheTable::attrCache[RELCAT_RELID] = nullptr; 

  attrCacheEntry = AttrCacheTable::attrCache[ATTRCAT_RELID];
  while (attrCacheEntry != nullptr) {
    struct AttrCacheEntry* nextEntry = attrCacheEntry->next;
    free(attrCacheEntry);
    attrCacheEntry = nextEntry;
  }
  AttrCacheTable::attrCache[ATTRCAT_RELID] = nullptr; 
}

/* This function will return the relation id of the relation with name `relName`. */
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  /* Traverse through the tableMetaInfo array and 
     find the entry with the relation name `relName`. */
  for(int i=0; i < MAX_OPEN; i++) {
    if(!tableMetaInfo[i].free && strcmp((char*)relName, tableMetaInfo[i].relName) == 0) {
      return i;
    }
  }

  return E_RELNOTOPEN;
}

/*This function will return a free entry in the Open Relation Table.*/
int OpenRelTable::getFreeOpenRelTableEntry() {
  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for(int i=2; i < MAX_OPEN; i++) {
    if(tableMetaInfo[i].free) {
      return i;
    }
  }

  return E_CACHEFULL;
}

/*This function will open a relation having name `relName`.*/
int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  if(OpenRelTable::getRelId(relName) != E_RELNOTOPEN) {
    return OpenRelTable::getRelId(relName);
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int slot = OpenRelTable::getFreeOpenRelTableEntry();

  if (slot == E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId;
  relId = slot;

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute relNameAttr;
  strcpy(relNameAttr.sVal, relName);

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId;
  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char *)RELCAT_ATTR_RELNAME, relNameAttr, EQ);

  if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, relcatRecId.slot);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = relcatRecId.block;
  relCacheEntry.recId.slot = relcatRecId.slot;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[relId] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[relId]) = relCacheEntry;


  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listHead = nullptr;

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  while (true) {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char *)ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);

      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      if (attrcatRecId.block == -1 && attrcatRecId.slot == -1) {
          // (no more attributes found in the Attribute Catalog for the relation.)
          break;
      }

      RecBuffer attrCatBlock(attrcatRecId.block);
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));

      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
      attrCacheEntry->recId.block = attrcatRecId.block;
      attrCacheEntry->recId.slot = attrcatRecId.slot;
      attrCacheEntry->next = nullptr;

      if (listHead == nullptr) {
          listHead = attrCacheEntry;
      } else {
          AttrCacheEntry* tail = listHead;
          while (tail->next != nullptr) {
              tail = tail->next;
          }
          tail->next = attrCacheEntry;
      }
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId] = listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }

  /****** Releasing the Relation Cache entry of the relation ******/

  if (!tableMetaInfo[relId].free && RelCacheTable::relCache[relId] != nullptr) {
    /* Get the Relation Catalog entry from RelCacheTable::relCache
    Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
    union Attribute record[RELCAT_NO_ATTRS];
    RelCatEntry relCatEntry = RelCacheTable::relCache[relId]->relCatEntry;
    RelCacheTable::relCatEntryToRecord(&(relCatEntry), record);

    RecId recId = RelCacheTable::relCache[relId]->recId;

    // declaring an object of RecBuffer class to write back to the buffer
    RecBuffer relCatBlock(recId.block);

    // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    relCatBlock.setRecord(record, recId.slot);
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId] = nullptr;

  /****** Releasing the Attribute Cache entry of the relation ******/

  // free the memory allocated in the attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  struct AttrCacheEntry* attrCacheEntry = AttrCacheTable::attrCache[relId];
  while (attrCacheEntry != nullptr) {
    struct AttrCacheEntry* nextEntry = attrCacheEntry->next;
    free(attrCacheEntry);
    attrCacheEntry = nextEntry;
  }
  AttrCacheTable::attrCache[relId] = nullptr;

  // (because we are not modifying the attribute cache at this stage,
  // write-back is not required. We will do it in subsequent
  // stages when it becomes needed)

  /****** Set the Open Relation Table entry of the relation as free ******/

  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  tableMetaInfo[relId].free = true;
  RelCacheTable::relCache[relId] = nullptr;
  AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}
