# NITCbase C++ Function Pseudocode

This document covers every `.cpp` implementation under `mynitcbase`, except
`FrontendInterface/FrontendInterface.cpp` and `main.cpp`. Each declaration is
transcribed from its corresponding header; the pseudocode follows the
implementation's control flow and data handling.

## Buffer/BlockBuffer.cpp

### `resetComparisonCount`

```cpp
void resetComparisonCount();
```

**Pseudocode**

1. Set the shared attribute-comparison counter to zero.
2. Return without producing a value.

### `getComparisonCount`

```cpp
unsigned long long getComparisonCount();
```

**Pseudocode**

1. Read the shared attribute-comparison counter.
2. Return its current value without modifying it.

### `BlockBuffer::BlockBuffer`

```cpp
BlockBuffer(char blockType);
```

**Pseudocode**

1. Translate the character block kind into the internal block-type constant:
   `'R'` becomes a record block, `'L'` becomes an index leaf, and any other
   value becomes an internal index block.
2. Request a free disk block of that type through `getFreeBlock`.
3. If allocation returns an invalid block number or an error value, print an
   allocation diagnostic.
4. Store the returned block number or error value in the object so the caller
   can determine whether construction succeeded.

### `BlockBuffer::BlockBuffer`

```cpp
BlockBuffer(int blockNum);
```

**Pseudocode**

1. Store the supplied existing block number in the object.
2. Do not allocate a block or validate its type at construction time.

### `RecBuffer::RecBuffer`

```cpp
RecBuffer();
```

**Pseudocode**

1. Invoke the base constructor with record-block type `'R'`.
2. The base constructor performs the block allocation and records either the
   new block number or an allocation error.

### `RecBuffer::RecBuffer`

```cpp
RecBuffer(int blockNum);
```

**Pseudocode**

1. Forward the existing block number to the base constructor.
2. This creates a wrapper for accessing a block, without allocating storage.

### `IndBuffer::IndBuffer`

```cpp
IndBuffer(char blockType);
```

**Pseudocode**

1. Forward the requested index-block kind to `BlockBuffer`.
2. Let the base class allocate the block and initialize the object's block
   number.

### `IndBuffer::IndBuffer`

```cpp
IndBuffer(int blockNum);
```

**Pseudocode**

1. Forward the existing block number to `BlockBuffer`.
2. Do not allocate a new block.

### `IndInternal::IndInternal`

```cpp
IndInternal();
```

**Pseudocode**

1. Construct the `IndBuffer` base with `'I'`, identifying an internal index
   block.
2. The base class performs allocation.

### `IndInternal::IndInternal`

```cpp
IndInternal(int blockNum);
```

**Pseudocode**

1. Construct the `IndBuffer` base around the supplied existing block.

### `IndLeaf::IndLeaf`

```cpp
IndLeaf();
```

**Pseudocode**

1. Construct the `IndBuffer` base with `'L'`, identifying an index leaf.
2. The base class performs allocation.

### `IndLeaf::IndLeaf`

```cpp
IndLeaf(int blockNum);
```

**Pseudocode**

1. Construct the `IndBuffer` base around the supplied existing block.

### `BlockBuffer::getHeader`

```cpp
int getHeader(struct HeadInfo *head);
```

**Pseudocode**

1. Ask `loadBlockAndGetBufferPtr` to locate or load this object's block.
2. If loading fails, return that error unchanged.
3. Copy the block header fields from their fixed byte offsets into `head`:
   block type, parent, left/right sibling, entry count, attribute count, and
   slot count.
4. Leave reserved header bytes untouched and return `SUCCESS`.

### `RecBuffer::getRecord`

```cpp
int getRecord(union Attribute *rec, int slotNum);
```

**Pseudocode**

1. Read the block header to determine the record's attribute count and the
   block's slot count.
2. Load or locate the block's in-memory buffer; return the loading error if
   one occurs.
3. Compute a record's byte length as `numAttrs * ATTR_SIZE`.
4. Locate the requested record after the header and slot map, offsetting by
   `slotNum * recordSize`.
5. Copy that record's bytes into the caller-provided array and return
   `SUCCESS`. This implementation does not validate `slotNum` against the
   header's slot count.

### `BlockBuffer::loadBlockAndGetBufferPtr`

```cpp
int loadBlockAndGetBufferPtr(unsigned char **buffPtr);
```

**Pseudocode**

1. Look up this block number in the static buffer pool.
2. If already cached, increment the timestamps of all occupied buffers and
   reset this block's timestamp to zero, marking it most recently used.
3. Otherwise request a free or replaceable buffer slot.
4. If the request indicates that the block number is out of bounds, return
   `E_OUTOFBOUND`.
5. Read the disk block into the selected buffer slot.
6. Store the selected buffer's address through `buffPtr` and return `SUCCESS`.

### `RecBuffer::getSlotMap`

```cpp
int getSlotMap(unsigned char *slotMap);
```

**Pseudocode**

1. Load or locate the block buffer; return an error if loading fails.
2. Read the header to obtain the number of slots.
3. Treat the bytes immediately after the fixed-size header as the slot map.
4. Copy `numSlots` bytes into the caller's array and return `SUCCESS`.

### `compareAttrs`

```cpp
int compareAttrs(Attribute attr1, Attribute attr2, int attrType);
```

**Pseudocode**

1. Increment the shared comparison counter once for this comparison.
2. If the type is `STRING`, compare the two string members with `strcmp`.
3. Otherwise subtract the numeric members.
4. Normalize the comparison result to `1` when the first value is larger,
   `-1` when smaller, or `0` when equal.
5. Return the normalized result.

### `RecBuffer::setRecord`

```cpp
int setRecord(union Attribute *rec, int slotNum);
```

**Pseudocode**

1. Load or locate the block buffer; return immediately if loading fails.
2. Read the header and obtain attribute and slot counts.
3. If `slotNum` is negative or outside the block's slot range, return
   `E_OUTOFBOUND`.
4. Compute the record's byte length and its position after the header and slot
   map.
5. Copy the caller's record bytes into that position.
6. Mark the block dirty in the static buffer metadata.
7. If dirty marking reports an error, print the implementation's diagnostic;
   this function still returns `SUCCESS` after the copy.

### `BlockBuffer::setHeader`

```cpp
int setHeader(struct HeadInfo *head);
```

**Pseudocode**

1. Load or locate the block buffer; return any load error.
2. Interpret the beginning of the buffer as a block header.
3. Copy each defined header field from `head` into the buffer, excluding the
   reserved bytes.
4. Mark the block dirty.
5. Return a dirty-marking error if present; otherwise return `SUCCESS`.

### `BlockBuffer::setBlockType`

```cpp
int setBlockType(int blockType);
```

**Pseudocode**

1. Load or locate the block buffer; return any load error.
2. Write the new block-type integer into the first header field.
3. Update the global allocation map entry for this block to the same type.
4. Mark the block dirty and return a dirty-marking error if it fails.
5. Otherwise return `SUCCESS`.

### `BlockBuffer::getFreeBlock`

```cpp
int getFreeBlock(int blockType);
```

**Pseudocode**

1. Scan the static block-allocation map from the first disk block onward for
   the first `UNUSED_BLK`.
2. If no free block exists, return `E_DISKFULL`.
3. Store the found block number in the object and obtain a buffer slot for it.
4. Prepare an empty header with the requested type, all links set to `-1`, and
   entry, attribute, and slot counts set to zero.
5. Write the header into the block; return an error if that operation fails.
6. Set the block type in the buffer and allocation map; return any error.
7. Return the allocated block number.

### `RecBuffer::setSlotMap`

```cpp
int setSlotMap(unsigned char *slotMap);
```

**Pseudocode**

1. Load or locate the block buffer, returning any load error.
2. Read its header and return any header-read error.
3. Copy exactly `numSlots` bytes from the supplied map to the bytes following
   the header.
4. Mark the block dirty and return any dirty-marking error.
5. Return `SUCCESS`.

### `BlockBuffer::getBlockNum`

```cpp
int getBlockNum();
```

**Pseudocode**

1. Return the block number currently stored in the object, including an error
   value when allocation did not succeed.

### `BlockBuffer::releaseBlock`

```cpp
void releaseBlock();
```

**Pseudocode**

1. If the stored number is invalid or outside the disk range, do nothing.
2. Look for the block in the static buffer pool.
3. If cached, mark its buffer slot free.
4. Mark the disk block unused in the allocation map.
5. Replace the object's block number with `INVALID_BLOCKNUM`.

### `IndInternal::getEntry`

```cpp
int getEntry(void *ptr, int indexNum);
```

**Pseudocode**

1. Reject an index outside `[0, MAX_KEYS_INTERNAL)` with `E_OUTOFBOUND`.
2. Load or locate the index block and return any loading error.
3. Compute the entry address after the block header using the fixed internal
   entry stride.
4. Copy the left child, attribute value, and right child separately into the
   caller's `InternalEntry`.
5. Return `SUCCESS`.

### `IndLeaf::getEntry`

```cpp
int getEntry(void *ptr, int indexNum);
```

**Pseudocode**

1. Reject an index outside `[0, MAX_KEYS_LEAF)` with `E_OUTOFBOUND`.
2. Load or locate the leaf and return any loading error.
3. Compute the entry position after the header using `LEAF_ENTRY_SIZE` as
   the stride.
4. Copy one complete `Index` entry to `ptr`.
5. Return `SUCCESS`.

### `IndInternal::setEntry`

```cpp
int setEntry(void *ptr, int indexNum);
```

**Pseudocode**

1. Reject an index outside `[0, MAX_KEYS_INTERNAL)` with `E_OUTOFBOUND`.
2. Load or locate the block and return any loading error.
3. Compute the entry position after the header.
4. Copy the left child, attribute value, and right child from the supplied
   `InternalEntry` into the block.
5. Request that the block be marked dirty.
6. Return `SUCCESS` after the copy. The implementation does not propagate a
   failure from the dirty-bit call.

### `IndLeaf::setEntry`

```cpp
int setEntry(void *ptr, int indexNum);
```

**Pseudocode**

1. Reject an index outside `[0, MAX_KEYS_LEAF)` with `E_OUTOFBOUND`.
2. Load or locate the leaf and return any loading error.
3. Compute the entry position after the header and copy one leaf entry into
   the block.
4. Request that the block be marked dirty.
5. Return `SUCCESS` after the copy. The implementation does not propagate a
   failure from the dirty-bit call.

## Buffer/StaticBuffer.cpp

### `StaticBuffer::StaticBuffer`

```cpp
StaticBuffer();
```

**Pseudocode**

1. For each of the first four reserved disk blocks, read its bytes from disk.
2. Copy those bytes into the in-memory block-allocation map.
3. Initialize every buffer metadata slot as free and clean, with no associated
   block and timestamp `-1`.

### `StaticBuffer::~StaticBuffer`

```cpp
~StaticBuffer();
```

**Pseudocode**

1. Serialize the first four allocation-map blocks back to disk.
2. Scan all buffer slots.
3. For each occupied, dirty slot, write its block contents back to the
   corresponding disk block.

### `StaticBuffer::getFreeBuffer`

```cpp
static int getFreeBuffer(int blockNum);
```

**Pseudocode**

1. Validate that the requested block number is in disk bounds; otherwise
   return `E_OUTOFBOUND`.
2. Increment timestamps for all currently occupied buffer slots.
3. Search the pool for a free slot, retaining the last free slot encountered.
4. If a free slot exists, select it. Otherwise find the occupied slot with
   the greatest timestamp (the least recently used slot).
5. If the selected replacement slot is dirty, flush its contents to its
   recorded disk block.
6. Update the selected metadata to occupied, clean, associated with
   `blockNum`, and timestamp zero.
7. Return the selected buffer index.

### `StaticBuffer::getBufferNum`

```cpp
static int getBufferNum(int blockNum);
```

**Pseudocode**

1. Validate the block number against disk bounds; return `E_OUTOFBOUND` when
   invalid.
2. Scan metadata for an occupied slot associated with the requested block.
3. Return its index if found.
4. If no slot contains the block, return `E_BLOCKNOTINBUFFER`.

### `StaticBuffer::setDirtyBit`

```cpp
static int setDirtyBit(int blockNum);
```

**Pseudocode**

1. Locate the block's buffer slot using `getBufferNum`.
2. If the block is absent, return `E_BLOCKNOTINBUFFER`.
3. If its number is out of bounds, return `E_OUTOFBOUND`.
4. Mark the slot dirty and return `SUCCESS`.

### `StaticBuffer::getStaticBlockType`

```cpp
static int getStaticBlockType(int blockNum);
```

**Pseudocode**

1. Validate that the block number is in disk bounds; otherwise return
   `E_OUTOFBOUND`.
2. Read its allocation-map byte, convert it to an integer, and return it.

## Disk_Class/Disk.cpp

### `Disk::Disk`

```cpp
Disk();
```

**Pseudocode**

1. Open the persistent disk image as a binary input stream.
2. Open the run-copy image as a binary output stream.
3. Copy the complete input stream buffer to the run-copy stream.
4. Close both streams. Runtime changes are thereby isolated from the
   persistent image until normal shutdown.

### `Disk::~Disk`

```cpp
~Disk();
```

**Pseudocode**

1. Open the run-copy image as binary input.
2. Open the persistent disk image as binary output.
3. Copy the complete run-copy stream into the persistent image.
4. Close both streams, persisting changes made during this session.

### `Disk::readBlock`

```cpp
static int readBlock(unsigned char *block, int blockNum);
```

**Pseudocode**

1. Open the run-copy image for binary reading.
2. If the block number is outside `[0, DISK_BLOCKS)`, return
   `E_OUTOFBOUND`.
3. Calculate the byte offset as `blockNum * BLOCK_SIZE`.
4. Seek to that position and read one complete block into the caller's buffer.
5. Close the file and return `SUCCESS`.

### `Disk::writeBlock`

```cpp
static int writeBlock(unsigned char *block, int blockNum);
```

**Pseudocode**

1. Open the run-copy image for binary read/write.
2. If the block number is outside `[0, DISK_BLOCKS)`, return
   `E_OUTOFBOUND`.
3. Calculate the byte offset as `blockNum * BLOCK_SIZE`.
4. Seek to that position and write one complete block from the caller's
   buffer.
5. Close the file and return `SUCCESS`.

## Cache/RelCacheTable.cpp

### `RelCacheTable::getRelCatEntry`

```cpp
static int getRelCatEntry(int relId, RelCatEntry *relCatBuf);
```

**Pseudocode**

1. Validate `relId`; return `E_OUTOFBOUND` if outside the open-relation range.
2. If its cache pointer is null, return `E_RELNOTOPEN`.
3. Copy the cached relation-catalog structure into the caller's buffer.
4. Return `SUCCESS`.

### `RelCacheTable::recordToRelCatEntry`

```cpp
static void recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS],
                                RelCatEntry *relCatEntry);
```

**Pseudocode**

1. Copy the relation name from the record's name field.
2. Convert the numeric record fields into the relation structure's attribute
   count, record count, first and last block numbers, and slots-per-block.
3. Return after populating the supplied structure.

### `RelCacheTable::getSearchIndex`

```cpp
static int getSearchIndex(int relId, RecId *searchIndex);
```

**Pseudocode**

1. Reject an out-of-range relation ID with `E_OUTOFBOUND`.
2. If no cache entry exists for that ID, return `E_RELNOTOPEN`.
3. Copy the cached relation search position into `searchIndex`.
4. Return `SUCCESS`.

### `RelCacheTable::setSearchIndex`

```cpp
static int setSearchIndex(int relId, RecId *searchIndex);
```

**Pseudocode**

1. Reject an out-of-range relation ID with `E_OUTOFBOUND`.
2. If no cache entry exists for that ID, return `E_RELNOTOPEN`.
3. Copy the supplied record ID into the relation cache's search position.
4. Return `SUCCESS`.

### `RelCacheTable::resetSearchIndex`

```cpp
static int resetSearchIndex(int relId);
```

**Pseudocode**

1. Create a record ID with both block and slot set to `-1`.
2. Pass it to `setSearchIndex`.
3. Return `SUCCESS`. The implementation does not propagate a failure from
   `setSearchIndex`.

### `RelCacheTable::setRelCatEntry`

```cpp
static int setRelCatEntry(int relId, RelCatEntry *relCatBuf);
```

**Pseudocode**

1. Validate the relation ID and return `E_OUTOFBOUND` when invalid.
2. If the relation has no cache entry, return `E_RELNOTOPEN`.
3. Copy the supplied relation structure into the cache entry.
4. Mark that cache entry dirty so it will be written back when closed.
5. Return `SUCCESS`.

### `RelCacheTable::relCatEntryToRecord`

```cpp
static void relCatEntryToRecord(RelCatEntry *relCatEntry,
                                union Attribute record[RELCAT_NO_ATTRS]);
```

**Pseudocode**

1. Copy the relation name into the relation-name record field.
2. Place the attribute count, record count, first and last block numbers, and
   slots-per-block into their corresponding numeric record fields.
3. Return after populating the caller's record array.

## Cache/AttrCacheTable.cpp

### `AttrCacheTable::getAttrCatEntry` (by offset)

```cpp
static int getAttrCatEntry(int relId, int attrOffset,
                           AttrCatEntry *attrCatBuf);
```

**Pseudocode**

1. Validate the relation ID; return `E_OUTOFBOUND` if invalid.
2. If the relation's attribute-cache list is null, return `E_RELNOTOPEN`.
3. Traverse the linked list from its head.
4. When an entry's stored offset matches `attrOffset`, copy its catalog
   structure to `attrCatBuf` and return `SUCCESS`.
5. Return `E_ATTRNOTEXIST` if the list has no matching offset.

### `AttrCacheTable::recordToAttrCatEntry`

```cpp
static void recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                 AttrCatEntry *attrCatEntry);
```

**Pseudocode**

1. Copy the relation and attribute names from their string fields.
2. Convert the type, primary flag, index-root block, and attribute offset
   fields into the matching structure members.
3. Return after populating the supplied structure.

### `AttrCacheTable::getAttrCatEntry` (by name)

```cpp
static int getAttrCatEntry(int relId, char attrName[ATTR_SIZE],
                           AttrCatEntry *attrCatBuf);
```

**Pseudocode**

1. Validate the relation ID; return `E_OUTOFBOUND` if invalid.
2. If its attribute-cache list is null, return `E_RELNOTOPEN`.
3. Traverse the list and compare each cached attribute name to `attrName`.
4. Copy the matching catalog structure to the caller and return `SUCCESS`.
5. If no name matches, return `E_ATTRNOTEXIST`.

### `AttrCacheTable::getSearchIndex` (by name)

```cpp
static int getSearchIndex(int relId, char attrName[ATTR_SIZE],
                          IndexId *searchIndex);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute cache exists.
2. Traverse the attribute list looking for an exact name match.
3. Copy the matched entry's index block and entry number into `searchIndex`.
4. Return `SUCCESS` on a match, `E_ATTRNOTEXIST` if no attribute matches, or
   the validation error otherwise.

### `AttrCacheTable::getSearchIndex` (by offset)

```cpp
static int getSearchIndex(int relId, int attrOffset, IndexId *searchIndex);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute cache exists.
2. Traverse entries until the stored attribute offset equals `attrOffset`.
3. Copy that entry's index block and entry number to the output.
4. Return `SUCCESS` if found, `E_ATTRNOTEXIST` otherwise.

### `AttrCacheTable::setSearchIndex` (by name)

```cpp
static int setSearchIndex(int relId, char attrName[ATTR_SIZE],
                          IndexId *searchIndex);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute cache exists.
2. Find the attribute by name.
3. Copy the supplied block and index values into its cached search position.
4. Return `SUCCESS` when updated, or `E_ATTRNOTEXIST` if there is no name
   match.

### `AttrCacheTable::setSearchIndex` (by offset)

```cpp
static int setSearchIndex(int relId, int attrOffset, IndexId *searchIndex);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute cache exists.
2. Find the attribute whose stored offset matches `attrOffset`.
3. Copy the supplied block and index values into its cached search position.
4. Return `SUCCESS` when updated, or `E_ATTRNOTEXIST` when no offset matches.

### `AttrCacheTable::resetSearchIndex` (by name)

```cpp
static int resetSearchIndex(int relId, char attrName[ATTR_SIZE]);
```

**Pseudocode**

1. Construct an index ID whose block and index are both `-1`.
2. Pass it to the name-based `setSearchIndex`.
3. Return that setter's result.

### `AttrCacheTable::resetSearchIndex` (by offset)

```cpp
static int resetSearchIndex(int relId, int attrOffset);
```

**Pseudocode**

1. Construct an index ID whose block and index are both `-1`.
2. Pass it to the offset-based `setSearchIndex`.
3. Return that setter's result.

### `AttrCacheTable::setAttrCatEntry` (by name)

```cpp
static int setAttrCatEntry(int relId, char attrName[ATTR_SIZE],
                           AttrCatEntry *attrCatBuf);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute-cache list exists.
2. Traverse the list for an entry with the supplied attribute name.
3. Replace its cached catalog structure with the supplied structure.
4. Mark that entry dirty for later catalog write-back.
5. Return `SUCCESS` on update or `E_ATTRNOTEXIST` if no name matches.

### `AttrCacheTable::setAttrCatEntry` (by offset)

```cpp
static int setAttrCatEntry(int relId, int attrOffset,
                           AttrCatEntry *attrCatBuf);
```

**Pseudocode**

1. Validate the relation ID and confirm that its attribute-cache list exists.
2. Traverse the list for an entry with the supplied offset.
3. Replace its cached catalog structure and mark it dirty.
4. Return `SUCCESS` on update or `E_ATTRNOTEXIST` if no offset matches.

### `AttrCacheTable::attrCatEntryToRecord`

```cpp
static void attrCatEntryToRecord(AttrCatEntry *attrCatEntry,
                                 union Attribute record[ATTRCAT_NO_ATTRS]);
```

**Pseudocode**

1. Copy relation and attribute names into the corresponding string fields.
2. Copy the attribute type, primary flag, root block, and offset into their
   numeric fields.
3. Return after the catalog record has been filled.

## Cache/OpenRelTable.cpp

### `OpenRelTable::OpenRelTable`

```cpp
OpenRelTable();
```

**Pseudocode**

1. For every open-relation slot, clear both cache pointers and mark the table
   metadata as free.
2. Read the relation-catalog records for the built-in relation catalog and
   attribute catalog.
3. Convert each record into a `RelCatEntry`, attach its catalog record ID, and
   allocate a persistent relation-cache entry for the reserved catalog ID.
4. Read the attribute-catalog records describing the relation catalog's
   attributes; convert each to an attribute-cache node and link the nodes into
   the reserved relation's linked list.
5. Repeat for the attribute catalog's own attribute records and link those
   nodes under its reserved relation ID.
6. Mark the two reserved catalog slots occupied and store their fixed names.

### `OpenRelTable::~OpenRelTable`

```cpp
~OpenRelTable();
```

**Pseudocode**

1. Starting at relation ID 2, close each user relation still marked open.
2. For the attribute-catalog cache entry, if dirty, convert its relation
   structure back into a record and write that record to its stored catalog
   location.
3. Free the attribute-catalog relation-cache entry and clear its pointer.
4. Repeat the dirty write-back and release for the relation-catalog cache
   entry.
5. Walk and free every linked attribute-cache node belonging to the two
   built-in catalogs, then clear both list heads.

### `OpenRelTable::getRelId`

```cpp
static int getRelId(char relName[ATTR_SIZE]);
```

**Pseudocode**

1. Scan all open-relation metadata entries.
2. Ignore free entries; compare the name of each occupied entry to `relName`.
3. Return the matching relation ID.
4. If no open relation has that name, return `E_RELNOTOPEN`.

### `OpenRelTable::getFreeOpenRelTableEntry`

```cpp
static int getFreeOpenRelTableEntry();
```

**Pseudocode**

1. Scan user-relation slots beginning at ID 2, preserving the reserved catalog
   entries.
2. Return the first slot marked free.
3. Return `E_CACHEFULL` if every user-relation slot is occupied.

### `OpenRelTable::openRel`

```cpp
static int openRel(char relName[ATTR_SIZE]);
```

**Pseudocode**

1. If the relation is already open, return its existing ID.
2. Find a free user-relation slot; return `E_CACHEFULL` if none exists.
3. Reset the relation catalog's search cursor and search it for `relName`.
4. If no catalog record matches, return `E_RELNOTEXIST`.
5. Read the matching catalog record, convert it to a relation-cache entry,
   retain its record ID, allocate cache memory, and store it at the chosen
   relation ID.
6. Reset the attribute catalog's search cursor.
7. Repeatedly search the attribute catalog for entries belonging to this
   relation. For each match, read and convert the record, retain its catalog
   record ID, allocate a cache node, and append it to the local linked list.
8. Stop when no further matching attribute record is found and install the
   linked-list head in the attribute cache.
9. Mark the open-relation metadata occupied, copy the relation name into it,
   and return the assigned ID.

### `OpenRelTable::closeRel`

```cpp
static int closeRel(int relId);
```

**Pseudocode**

1. Reject attempts to close either reserved catalog with `E_NOTPERMITTED`.
2. Reject an invalid ID with `E_OUTOFBOUND`, then reject a free slot with
   `E_RELNOTOPEN`.
3. If the relation cache entry is dirty, convert its relation metadata back to
   a catalog record and write it to the stored relation-catalog record ID.
4. Free the relation-cache object and clear its pointer.
5. Traverse the relation's attribute-cache list. For each dirty node, convert
   the cached attribute metadata back into a record and write it to the
   stored attribute-catalog location.
6. Free every attribute-cache node and clear the list head.
7. Mark the open-relation slot free and clear both cache pointers.
8. Return `SUCCESS`.

## Schema/Schema.cpp

### `Schema::openRel`

```cpp
static int openRel(char relName[ATTR_SIZE]);
```

**Pseudocode**

1. Call `OpenRelTable::openRel` with the relation name.
2. If the result is a valid nonnegative relation ID, convert it to `SUCCESS`.
3. Otherwise pass the error code back unchanged.

### `Schema::closeRel`

```cpp
static int closeRel(char relName[ATTR_SIZE]);
```

**Pseudocode**

1. If the name refers to either system catalog, return `E_NOTPERMITTED`.
2. Look up the relation ID by name.
3. If it is not open, return `E_RELNOTOPEN`.
4. Delegate closing to `OpenRelTable::closeRel` and return its result.

### `Schema::renameRel`

```cpp
static int renameRel(char oldRelName[ATTR_SIZE],
                     char newRelName[ATTR_SIZE]);
```

**Pseudocode**

1. Reject a system-catalog name as either the old or new relation name with
   `E_NOTPERMITTED`.
2. Check whether the old relation is open; if so, return `E_RELOPEN`.
3. Delegate catalog updates and duplicate/nonexistent-name checks to
   `BlockAccess::renameRelation`.
4. Return the delegated result.

### `Schema::renameAttr`

```cpp
static int renameAttr(char relName[ATTR_SIZE],
                      char oldAttrName[ATTR_SIZE],
                      char newAttrName[ATTR_SIZE]);
```

**Pseudocode**

1. Reject attempts to modify attributes of a system catalog.
2. If the relation is open, return `E_RELOPEN`.
3. Delegate the attribute-catalog lookup and rename to
   `BlockAccess::renameAttribute`.
4. Return the delegated result.

### `Schema::createRel`

```cpp
static int createRel(char relName[], int numOfAttributes,
                     char attrNames[][ATTR_SIZE], int attrType[]);
```

**Pseudocode**

1. Build a string-valued attribute containing the requested relation name.
2. Reset the relation-catalog cursor and search for that name.
3. If a matching relation already exists, return `E_RELEXIST`.
4. Compare every pair of requested attribute names; return
   `E_DUPLICATEATTR` if any pair is identical.
5. Assemble a relation-catalog record containing the name, attribute count,
   zero records, unset first/last block links, and the computed slots-per-block
   value based on the record layout.
6. Insert that record into the relation catalog. Return immediately if the
   insertion fails.
7. For each requested attribute, assemble an attribute-catalog record with
   relation name, attribute name, type, unset primary/index metadata, and its
   ordinal offset.
8. Insert each attribute record into the attribute catalog.
9. If any attribute insertion fails, delete the partially created relation
   through `Schema::deleteRel` and return `E_DISKFULL`.
10. Return `SUCCESS` after all catalog records are inserted.

### `Schema::deleteRel`

```cpp
static int deleteRel(char relName[ATTR_SIZE]);
```

**Pseudocode**

1. Refuse deletion of the relation or attribute catalog with
   `E_NOTPERMITTED`.
2. Check whether the relation is open; return `E_RELOPEN` if it is.
3. Delegate record-block, catalog-entry, and index cleanup to
   `BlockAccess::deleteRelation`.
4. Return the delegated status.

### `Schema::createIndex`

```cpp
static int createIndex(char relName[ATTR_SIZE],
                       char attrName[ATTR_SIZE]);
```

**Pseudocode**

1. Reject either system catalog with `E_NOTPERMITTED`.
2. Find the relation's open ID; return `E_RELNOTOPEN` if it is not open.
3. Delegate index creation and population to `BPlusTree::bPlusCreate`.
4. Return its result.

### `Schema::dropIndex`

```cpp
static int dropIndex(char relName[ATTR_SIZE], char attrName[ATTR_SIZE]);
```

**Pseudocode**

1. Reject a system catalog with `E_NOTPERMITTED`.
2. Look up the relation ID; return `E_RELNOTOPEN` if the relation is not open.
3. Fetch the named attribute's catalog entry; return `E_ATTRNOTEXIST` if it
   cannot be found.
4. If the attribute has no root block, return `E_NOINDEX`.
5. Destroy the B+ tree rooted at the recorded block.
6. Set the cached root block to `-1` and save the updated attribute entry.
7. Return `SUCCESS`.

## Frontend/Frontend.cpp

### `Frontend::create_table`

```cpp
static int create_table(char relname[ATTR_SIZE], int no_attrs,
                        char attributes[][ATTR_SIZE], int type_attrs[]);
```

**Pseudocode**

1. Forward the relation name, attribute count, attribute names, and types to
   `Schema::createRel`.
2. Return its status unchanged.

### `Frontend::drop_table`

```cpp
static int drop_table(char relname[ATTR_SIZE]);
```

**Pseudocode**

1. Forward the relation name to `Schema::deleteRel`.
2. Return its status unchanged.

### `Frontend::open_table`

```cpp
static int open_table(char relname[ATTR_SIZE]);
```

**Pseudocode**

1. Forward the name to `Schema::openRel`.
2. Return its status unchanged.

### `Frontend::close_table`

```cpp
static int close_table(char relname[ATTR_SIZE]);
```

**Pseudocode**

1. Forward the name to `Schema::closeRel`.
2. Return its status unchanged.

### `Frontend::create_index`

```cpp
static int create_index(char relname[ATTR_SIZE],
                        char attrname[ATTR_SIZE]);
```

**Pseudocode**

1. Forward relation and attribute names to `Schema::createIndex`.
2. Return its status unchanged.

### `Frontend::drop_index`

```cpp
static int drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]);
```

**Pseudocode**

1. Forward relation and attribute names to `Schema::dropIndex`.
2. Return its status unchanged.

### `Frontend::alter_table_rename`

```cpp
static int alter_table_rename(char relname_from[ATTR_SIZE],
                              char relname_to[ATTR_SIZE]);
```

**Pseudocode**

1. Pass the old and new names to `Schema::renameRel`.
2. Return its status unchanged.

### `Frontend::alter_table_rename_column`

```cpp
static int alter_table_rename_column(char relname[ATTR_SIZE],
                                     char attrname_from[16],
                                     char attrname_to[16]);
```

**Pseudocode**

1. Pass the relation name and old/new attribute names to `Schema::renameAttr`.
2. Return its status unchanged.

### `Frontend::insert_into_table_values`

```cpp
static int insert_into_table_values(char relname[ATTR_SIZE], int attr_count,
                                    char attr_values[][ATTR_SIZE]);
```

**Pseudocode**

1. Forward the relation, value count, and textual values to `Algebra::insert`.
2. Return its status unchanged.

### `Frontend::select_from_table`

```cpp
static int select_from_table(char relname_source[ATTR_SIZE],
                             char relname_target[ATTR_SIZE]);
```

**Pseudocode**

1. Request an all-attribute projection from the source relation to the target
   through `Algebra::project`.
2. Return its status unchanged.

### `Frontend::select_attrlist_from_table`

```cpp
static int select_attrlist_from_table(
    char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
    int attr_count, char attr_list[][ATTR_SIZE]);
```

**Pseudocode**

1. Pass the source, target, attribute count, and requested names to the
   attribute-list overload of `Algebra::project`.
2. Return its status unchanged.

### `Frontend::select_from_table_where`

```cpp
static int select_from_table_where(char relname_source[ATTR_SIZE],
                                   char relname_target[ATTR_SIZE],
                                   char attribute[ATTR_SIZE], int op,
                                   char value[ATTR_SIZE]);
```

**Pseudocode**

1. Pass the relation names and predicate attribute, operator, and textual
   value to `Algebra::select`.
2. Return its status unchanged.

### `Frontend::select_attrlist_from_table_where`

```cpp
static int select_attrlist_from_table_where(
    char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
    int attr_count, char attr_list[][ATTR_SIZE],
    char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]);
```

**Pseudocode**

1. Run `Algebra::select` from the source into the temporary relation `TEMP`.
2. If selection fails, return its error without opening the temporary
   relation.
3. Open `TEMP`. If opening fails, delete the temporary relation and return
   the open error.
4. Project the requested attributes from `TEMP` into the real target relation.
5. Close the temporary relation, delete it, and return the projection result.

### `Frontend::select_from_join_where`

```cpp
static int select_from_join_where(
    char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
    char relname_target[ATTR_SIZE], char join_attr_one[ATTR_SIZE],
    char join_attr_two[ATTR_SIZE]);
```

**Pseudocode**

1. Forward both source relations, target, and join-attribute names to
   `Algebra::join`.
2. Return its status unchanged.

### `Frontend::select_attrlist_from_join_where`

```cpp
static int select_attrlist_from_join_where(
    char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
    char relname_target[ATTR_SIZE], char join_attr_one[ATTR_SIZE],
    char join_attr_two[ATTR_SIZE], int attr_count,
    char attr_list[][ATTR_SIZE]);
```

**Pseudocode**

1. Join the two source relations into `TEMP`.
2. If the join fails, return its error.
3. Open `TEMP`. If opening fails, delete it and return the open error.
4. Project the requested attributes from `TEMP` into the requested target.
5. Close and delete `TEMP`, then return the projection status.

### `Frontend::custom_function`

```cpp
static int custom_function(int argc, char argv[][ATTR_SIZE]);
```

**Pseudocode**

1. Accept the token count and token array.
2. Perform no additional work in the current implementation.
3. Return `SUCCESS`.

## Algebra/Algebra.cpp

### `isNumber`

```cpp
bool isNumber(char *str);
```

**Pseudocode**

1. Use `sscanf` to attempt to parse a floating-point number and record how many
   characters were consumed, including trailing whitespace.
2. Return true only if one number was parsed and the consumed character count
   equals the full string length.
3. Return false for an invalid number or for extra non-whitespace characters.

### `Algebra::insert`

```cpp
static int insert(char relName[ATTR_SIZE], int numberOfAttributes,
                  char record[][ATTR_SIZE]);
```

**Pseudocode**

1. Reject direct inserts into either system catalog with `E_NOTPERMITTED`.
2. Look up the relation ID and return `E_RELNOTOPEN` if the relation is not
   open.
3. Read the relation metadata from the relation cache.
4. If the supplied value count differs from the relation's attribute count,
   return `E_NATTRMISMATCH`.
5. Allocate an `Attribute` array sized to the relation's attributes.
6. For each attribute position, fetch its catalog entry to determine its type.
7. For numeric attributes, validate the textual value with `isNumber`; return
   `E_ATTRTYPEMISMATCH` for invalid input, otherwise convert it to a number.
8. For string attributes, copy the supplied text into the string member.
9. Insert the converted record through `BlockAccess::insert`.
10. Return the block-access result.

### `Algebra::select`

```cpp
static int select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],
                  char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]);
```

**Pseudocode**

1. Resolve the open source relation ID; return `E_RELNOTOPEN` if absent.
2. Fetch the predicate attribute's catalog entry; return
   `E_ATTRNOTEXIST` when the attribute is missing.
3. Convert the comparison value using the attribute's type. For numeric
   attributes, validate and parse the text or return `E_ATTRTYPEMISMATCH`; for
   strings, copy it into a string attribute value.
4. Read source-relation metadata and collect every source attribute's name and
   type from the attribute cache.
5. Create the target relation with that schema. If creation fails, return its
   status.
6. Open the target. If opening fails, delete the newly created target and
   return the open error.
7. Reset both the source relation's record cursor and the predicate
   attribute's index cursor so the scan starts at the beginning.
8. Reset the comparison counter and repeatedly call `BlockAccess::search` for
   matching records.
9. Insert each returned record into the target. If insertion fails, close and
   delete the target, then return the insertion error.
10. Continue searching until no more record is returned.
11. Print the comparison count, close the target relation, and return
    `SUCCESS`.

### `Algebra::project` (selected attributes)

```cpp
static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE],
                   int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]);
```

**Pseudocode**

1. Find the source relation ID; return `E_RELNOTOPEN` if it is not open.
2. Read source relation metadata to determine its attribute count.
3. For each requested target attribute, retrieve its source catalog entry.
   Return `E_ATTRNOTEXIST` if any requested name is absent.
4. Save each requested attribute's source-record offset and data type.
5. Create the target relation using the requested names and types. Return a
   creation error if one occurs.
6. Open the target; if opening fails, delete it and return the open error.
7. Reset the source relation cursor and repeatedly request the next source
   record with `BlockAccess::project`.
8. For every record, assemble a new record by copying each requested value
   from its saved source offset, copying numeric values directly and strings
   by name.
9. Insert the projected record into the target.
10. If insertion fails, close and delete the target, then return the error.
11. When source records are exhausted, close the target and return `SUCCESS`.

### `Algebra::project` (all attributes)

```cpp
static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]);
```

**Pseudocode**

1. Find the source relation ID and return `E_RELNOTOPEN` if it is not open.
2. Read the source relation's attribute count.
3. Fetch each attribute-cache entry and collect its name and type in source
   order.
4. Create a target relation with the same schema; return a creation error if
   necessary.
5. Open the target. If opening fails, delete it and return the open error.
6. Reset the source scan cursor.
7. Repeatedly fetch the next source record with `BlockAccess::project` and
   insert the complete record into the target.
8. On insertion failure, close and delete the target and return the error.
9. Close the target after exhausting source records and return `SUCCESS`.

### `Algebra::join`

```cpp
static int join(char srcRelOne[ATTR_SIZE], char srcRelTwo[ATTR_SIZE],
                char targetRel[ATTR_SIZE], char attrOne[ATTR_SIZE],
                char attrTwo[ATTR_SIZE]);
```

**Pseudocode**

1. Resolve both open source relation IDs; return `E_RELNOTOPEN` if either is
   absent.
2. Fetch the two join-attribute catalog entries. Return `E_ATTRNOTEXIST` if
   either name is absent.
3. If the join attributes have different types, return
   `E_ATTRTYPEMISMATCH`.
4. Read metadata for both source relations.
5. Compare attributes from the first relation against names in the second.
   If a same-named pair is not precisely the pair of join attributes, return
   `E_DUPLICATEATTR`.
6. If the second relation has no index on its join attribute, create one;
   return the index-creation error if that fails.
7. Form the output schema from every first-relation attribute followed by all
   second-relation attributes except the second join attribute.
8. Create the target relation. Return any creation error.
9. Open the target. If opening fails, delete it and return the open error.
10. Reset the first relation scan and iterate over its records.
11. For each first-side record, reset the second relation's record cursor and
    the second join attribute's index cursor.
12. Search the second relation for records whose join value equals the
    current first-side join value.
13. For every match, compose a target record from the complete first record
    and the second record with its join attribute omitted.
14. Insert the combined record. On failure, close and delete the target and
    return the error.
15. After all matches and source records are processed, close the target and
    return `SUCCESS`.

## BlockAccess/BlockAccess.cpp

### `BlockAccess::linearSearch`

```cpp
static RecId linearSearch(int relId, char *attrName, Attribute attrVal,
                          int op);
```

**Pseudocode**

1. Read the relation's previous search position from the relation cache.
2. If the cursor is `{-1, -1}`, start at the relation's first data block and
   slot zero. Otherwise resume at the slot after the previous hit.
3. While a data block remains, load its header, current record, and slot map.
4. If the slot is beyond the block's slot count, advance to the block's right
   neighbor and reset the slot to zero.
5. If the slot is unoccupied, advance to the next slot.
6. For an occupied slot, fetch the predicate attribute metadata and compare
   its value with `attrVal`.
7. Evaluate the comparison result against the requested operator (`NE`, `LT`,
   `LE`, `EQ`, `GT`, or `GE`).
8. On a match, store the matching record ID as the relation cursor and return
   that ID.
9. Otherwise advance the slot and continue.
10. Return `{-1, -1}` when every record block has been exhausted.

### `BlockAccess::renameRelation`

```cpp
static int renameRelation(char *oldName, char *newName);
```

**Pseudocode**

1. Reset the relation-catalog cursor and search for `newName`.
2. If the new name is already present, return `E_RELEXIST`.
3. Reset the catalog cursor again and search for `oldName`.
4. If the old name is absent, return `E_RELNOTEXIST`.
5. Read the old relation-catalog record, replace its relation-name field with
   `newName`, and write the record back.
6. Reset the attribute-catalog cursor.
7. Repeat once for each attribute recorded for the relation: find the next
   attribute-catalog row belonging to `oldName`, replace its relation-name
   field, and write it back.
8. Return `SUCCESS`.

### `BlockAccess::renameAttribute`

```cpp
static int renameAttribute(char *relName, char *oldName, char *newName);
```

**Pseudocode**

1. Reset the relation-catalog cursor and verify that `relName` exists.
   Return `E_RELNOTEXIST` if it does not.
2. Reset the attribute-catalog cursor and initialize the remembered old-name
   record ID to `{-1, -1}`.
3. Repeatedly search for attribute-catalog rows belonging to the relation.
4. Read each row. Remember the row ID when its attribute name equals
   `oldName`.
5. If any row already has `newName`, return `E_ATTREXIST`.
6. After scanning, return `E_ATTRNOTEXIST` if the old name was never found.
7. Read the remembered row, replace its attribute-name field with `newName`,
   write it back, and return `SUCCESS`.

### `BlockAccess::insert`

```cpp
static int insert(int relId, union Attribute *record);
```

**Pseudocode**

1. Fetch the relation metadata and initialize the prospective record ID to
   `{-1, -1}`.
2. Walk the relation's linked data blocks, reading each header and slot map.
3. In each block, find the first unoccupied slot. If found, save that block
   and slot and stop scanning.
4. If a block has no free slot, remember it as the previous block and advance
   to its right neighbor.
5. If no existing block has room, reject allocation for the relation catalog
   with `E_MAXRELATIONS`; otherwise allocate a new record block.
6. Initialize the new block header with record type, relation schema counts,
   empty entry count, no right neighbor, and the previous block link. Mark all
   its slots unoccupied.
7. If a previous block exists, set its right link to the new block. Otherwise
   set the relation's first block in the relation cache. In either case,
   update the relation's last block.
8. Write the record into the selected slot.
9. Mark that slot occupied in the block's slot map and increment the block's
   entry count.
10. Increment the relation's cached record count.
11. For each relation attribute, fetch its metadata. If an index exists,
    insert the attribute value and record ID into that B+ tree.
12. If an index insertion reports `E_DISKFULL`, record that index blocks were
    released; continue checking the remaining attributes.
13. Return `E_INDEX_BLOCKS_RELEASED` if an index was destroyed due to space
    exhaustion; otherwise return `SUCCESS`.

### `BlockAccess::search`

```cpp
static int search(int relId, Attribute *record, char *attrName,
                  Attribute attrVal, int op);
```

**Pseudocode**

1. Fetch the predicate attribute's catalog entry; return its error if lookup
   fails.
2. If the attribute has no index, find the next matching record using
   `linearSearch`.
3. Otherwise find the next matching record using `BPlusTree::bPlusSearch`.
4. If the result is `{-1, -1}`, return `E_NOTFOUND`.
5. Load the matching record block and copy the record into the caller's
   buffer.
6. Return `SUCCESS`.

### `BlockAccess::deleteRelation`

```cpp
static int deleteRelation(char *relName);
```

**Pseudocode**

1. Reject deletion of either system catalog with `E_NOTPERMITTED`.
2. Reset the relation-catalog cursor and search for the named relation.
3. Return `E_RELNOTEXIST` if its catalog row is absent.
4. Read the relation-catalog row to obtain the first data block and attribute
   count.
5. Traverse the relation's data-block chain. Save each right-neighbor number
   before releasing its current block.
6. Reset the attribute-catalog cursor and repeatedly find rows belonging to
   the relation.
7. For each matching attribute row, read its root block and slot map, mark
   the catalog slot unoccupied, and decrement the block's entry count.
8. When an attribute-catalog block becomes empty, reconnect its left and right
   neighbors. If it was the last block, update the attribute catalog's last
   block. Release the emptied block.
9. If the attribute row refers to an index root, recursively destroy that
   B+ tree.
10. Decrement the relation-catalog block's entry count and mark the deleted
    relation slot unoccupied.
11. Decrement the cached record count of the relation catalog and reduce the
    attribute catalog's cached record count by the number of removed
    attributes.
12. Return `SUCCESS`.

### `BlockAccess::project`

```cpp
static int project(int relId, Attribute *record);
```

**Pseudocode**

1. Read the relation's previous scan cursor.
2. If it is `{-1, -1}`, start at the relation's first data block and slot
   zero; otherwise resume one slot after the previous record.
3. While a data block remains, read its header and slot map.
4. If the slot is past the block's slot count, advance to its right neighbor
   and reset the slot to zero.
5. If the slot is unoccupied, advance within the block.
6. Stop once an occupied slot is reached.
7. If no block remains, return `E_NOTFOUND`.
8. Save the found record ID as the relation's new cursor.
9. Read that record into the caller-provided buffer and return `SUCCESS`.

## BPlusTree/BPlusTree.cpp

### `BPlusTree::bPlusSearch`

```cpp
static RecId bPlusSearch(int relId, char attrName[ATTR_SIZE],
                         union Attribute attrVal, int op);
```

**Pseudocode**

1. Fetch the attribute's saved B+ tree search position and catalog metadata.
   If either lookup fails, return `{-1, -1}`.
2. If the saved position is `{-1, -1}`, start at the attribute's root and
   entry zero. If there is no root, return `{-1, -1}`.
3. Otherwise resume at the entry immediately following the saved leaf entry.
   If that entry is beyond the leaf, move to its right-linked leaf at entry
   zero. If no next leaf exists, return `{-1, -1}`.
4. When starting from the root, descend internal nodes. For `NE`, `LT`, and
   `LE`, repeatedly follow the leftmost child. For `EQ`, `GT`, and `GE`, scan
   separators to select the left child of the first separator meeting the
   operator-specific boundary; if none qualifies, follow the last entry's
   right child.
5. Scan leaf entries from the current position in ascending order and compare
   each value with `attrVal`.
6. When an entry satisfies the operator, save its leaf block and entry index
   in the attribute cache and return its referenced record ID.
7. For `EQ`, `LE`, or `LT`, stop early when the leaf value has passed the
   possible matching range.
8. For operators other than `NE`, stop after exhausting the current candidate
   leaf. For `NE`, continue across right-linked leaves until a match or end.
9. Return `{-1, -1}` if no qualifying entry exists.

### `BPlusTree::bPlusCreate`

```cpp
static int bPlusCreate(int relId, char attrName[ATTR_SIZE]);
```

**Pseudocode**

1. Reject relation- or attribute-catalog IDs with `E_NOTPERMITTED`.
2. Fetch the attribute metadata; return its lookup error if it fails.
3. If a root already exists, return `SUCCESS` without rebuilding the index.
4. Allocate an empty leaf and return `E_DISKFULL` if allocation fails.
5. Save the new root block in the cached attribute entry.
6. Read the relation metadata and begin at its first record block.
7. For each record block, load its slot map and inspect all relation slots.
8. For each occupied slot, read the record, form its record ID, and call
   `bPlusInsert` with the indexed attribute value.
9. Return immediately if an insertion fails.
10. Advance through the record-block chain using each block's right link.
11. Return `SUCCESS` when every existing record has been indexed.

### `BPlusTree::bPlusDestroy`

```cpp
static int bPlusDestroy(int rootBlockNum);
```

**Pseudocode**

1. Validate the root block number; return `E_OUTOFBOUND` if invalid.
2. Read its block type from the allocation map.
3. For a leaf, wrap the block in `IndLeaf`, release it, and return
   `SUCCESS`.
4. For an internal node, read its header and each entry.
5. Recursively destroy the left child of the first entry and the right child
   of every entry, ensuring each child is visited once.
6. Release the internal block and return `SUCCESS`.
7. If the block is neither an index leaf nor an internal node, return
   `E_INVALIDBLOCK`.

### `BPlusTree::bPlusInsert`

```cpp
static int bPlusInsert(int relId, char attrName[ATTR_SIZE],
                       union Attribute attrVal, RecId recordId);
```

**Pseudocode**

1. Fetch the attribute metadata; return its error if lookup fails.
2. If the attribute has no root, return `E_NOINDEX`.
3. Descend the tree using `findLeafToInsert` to find the leaf for `attrVal`.
4. Build an index entry containing the value and record block/slot.
5. Insert it into the leaf, allowing any required split to propagate upward.
6. If insertion reports `E_DISKFULL`, destroy the existing tree, set the
   attribute's root to `-1`, and save the updated metadata.
7. Return `E_DISKFULL` after cleanup; otherwise return `SUCCESS`.

### `BPlusTree::findLeafToInsert`

```cpp
static int findLeafToInsert(int rootBlock, Attribute attrVal, int attrType);
```

**Pseudocode**

1. Set the current block to the root.
2. While the current block is not a leaf, read its internal-node header.
3. Scan separator entries in order until finding the first key greater than or
   equal to `attrVal`.
4. If such a separator exists, descend through its left child.
5. Otherwise descend through the right child of the final separator.
6. Return the block number once a leaf is reached.

### `BPlusTree::insertIntoLeaf`

```cpp
static int insertIntoLeaf(int relId, char attrName[ATTR_SIZE],
                          int blockNum, Index entry);
```

**Pseudocode**

1. Fetch attribute metadata to determine the key type.
2. Read the target leaf header and allocate a temporary array for its current
   entries plus the new one.
3. Copy existing entries into the temporary array in sorted order, placing the
   new entry before the first strictly greater key.
4. If the new key belongs after all existing keys, append it.
5. If the leaf has room, increment its entry count, write the updated header,
   write all temporary entries back, and return `SUCCESS`.
6. If full, split the combined entries with `splitLeaf`.
7. If a right leaf cannot be allocated, return `E_DISKFULL`.
8. Create a separator from the designated middle entry. If the old leaf had a
   parent, insert the separator into that parent; otherwise create a new root.
9. Return the result of the parent insertion or root creation.

### `BPlusTree::splitLeaf`

```cpp
static int splitLeaf(int leafBlockNum, Index indices[]);
```

**Pseudocode**

1. Allocate a new right leaf and wrap the existing leaf as the left leaf.
2. If allocation fails, return `E_DISKFULL`.
3. Read both headers.
4. Set the right leaf's entry count to half the combined entries, copy the
   left leaf's parent, and connect the new leaf between the left and its
   former right neighbor.
5. Set the left leaf's entry count to the same split size and point its right
   link at the new leaf.
6. Write the first half of the sorted temporary entries into the left leaf and
   the remaining half into the right leaf.
7. Return the new right leaf's block number.

### `BPlusTree::insertIntoInternal`

```cpp
static int insertIntoInternal(int relId, char attrName[ATTR_SIZE],
                              int intBlockNum, InternalEntry entry);
```

**Pseudocode**

1. Fetch the attribute metadata and return its error if it fails.
2. Read the target internal node and allocate a temporary array for its
   current separators plus the new separator.
3. Copy separators in sorted order, inserting the new entry before the first
   strictly greater key or at the end if no key is greater.
4. Repair adjacent child pointers around the insertion so the new separator's
   left and right children occupy the correct positions.
5. If the node has room, increment its entry count, write the header and all
   temporary entries, then return `SUCCESS`.
6. Otherwise split the combined separators with `splitInternal`.
7. If splitting fails because the disk is full, destroy the newly detached
   right subtree and return `E_DISKFULL`.
8. Promote the designated middle separator to the parent using a recursive
   `insertIntoInternal` call, or create a new root if this node had no parent.
9. Return the propagation result.

### `BPlusTree::splitInternal`

```cpp
static int splitInternal(int intBlockNum, InternalEntry internalEntries[]);
```

**Pseudocode**

1. Allocate a new right internal node and wrap the existing node as the left.
2. If allocation fails, return `E_DISKFULL`.
3. Read both headers and assign half of the entries to each node, leaving the
   designated middle separator to be promoted.
4. Write the left half to the existing node and the entries after the
   promoted separator to the new right node.
5. Determine the child block type from the first entry's left child.
6. For each child moved to the right node, update its parent pointer to the
   new right node. Handle the first moved left child, then update each moved
   entry's right child.
7. Return the new right node's block number.

### `BPlusTree::createNewRoot`

```cpp
static int createNewRoot(int relId, char attrName[ATTR_SIZE],
                         Attribute attrVal, int lChild, int rChild);
```

**Pseudocode**

1. Fetch the indexed attribute metadata.
2. Allocate a new internal node.
3. If allocation fails, destroy the newly detached right subtree and return
   `E_DISKFULL`.
4. Set the new root's entry count to one.
5. Write a separator entry containing `attrVal`, `lChild`, and `rChild`.
6. Read both child headers, set their parent pointers to the new root, and
   write the headers back.
7. Store the new root block in the attribute metadata cache.
8. Return `SUCCESS`.