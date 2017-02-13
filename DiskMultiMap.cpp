#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include <iostream>
#include <cstring>
#include <functional>

using namespace std;

DiskMultiMap::DiskMultiMap()		// DONE
{
	sizenode = sizeof(DiskNode);				// initialize int variables for size of node, bucket, and offset
	sizebucket = sizeof(Bucket);
	sizeoffset = sizeof(BinaryFile::Offset);
	head = 0;								
	r_head = 0;
}

DiskMultiMap::~DiskMultiMap()		// DONE
{
	close();
}

bool DiskMultiMap::createNew(const string& filename, unsigned int numBuckets) // DONE
{
	if (bf.isOpen())
		bf.close();

	bool success = bf.createNew(filename);
	if (!success)
		return false;

	head = numBuckets;				// head holds number of buckets
	r_head = sizeoffset;			// r_head is head of "removed" linked list, initially points to itself
	bf.write(head, 0);
	bf.write(r_head, sizeoffset);

	for (int i = 0; i < head; i++)
	{
		Bucket b(0);										// create specified number of buckets
		bf.write(b, (2 * sizeoffset) + (i * sizebucket));
	}
	return true;
}

bool DiskMultiMap::openExisting(const string& filename)			// DONE
{
	if (bf.isOpen())
		bf.close();

	bool success = bf.openExisting(filename);
	if (!success)
		return false;

	bf.read(head, 0);
	bf.read(r_head, sizeoffset);
	return true;
}

void DiskMultiMap::close()				// DONE
{
	if (bf.isOpen())
		bf.close();
}

bool DiskMultiMap::insert(const string& key, const string& value, const string& context)	// DONE
{
	const char* ckey = key.c_str();				// cnnvert to cstrings
	const char* cval = value.c_str();
	const char* ccont = context.c_str();

	if (strlen(ckey) > 120 || strlen(cval) > 120 || strlen(ccont) > 120)	// check for valid size
		return false;

	DiskNode n;						// create node and copy passed in strings
	strcpy(n.key, ckey);
	strcpy(n.value, cval);
	strcpy(n.context, ccont);

	hash<string> str_hash;					// hash function assigns int value to a string
	unsigned int hashValue = str_hash(key);
	unsigned int buck = hashValue%head;
	int buckaddr = (2 * sizeoffset) + (sizebucket * buck);	// find address of specified bucket
	Bucket b(0);
	bf.read(b, buckaddr);

	if (r_head == sizeoffset)			// if there are no reusable nodes
	{
		BinaryFile::Offset length = bf.fileLength();	// measure length of file

		if (b.nodeaddr == 0)		// if bucket is empty
		{
			b.nodeaddr = length;
			n.next = buckaddr;
		}
		else
		{							// if bucket already contains node(s)
			n.next = b.nodeaddr;
			b.nodeaddr = length;
		}

		bf.write(n, length);
		bf.write(b, buckaddr);
	}
	else
	{						// if there are reusable nodes
		DiskNode test;
		bf.read(test, r_head);

		if (b.nodeaddr == 0)			// if bucket is empty
		{
			b.nodeaddr = r_head;
			r_head = test.next;
			n.next = buckaddr;
		}
		else
		{							// if bucket already contains node(s)
			BinaryFile::Offset temp = test.next;
			n.next = b.nodeaddr;
			b.nodeaddr = r_head;
			r_head = temp;
		}

		bf.write(n, b.nodeaddr);
		bf.write(b, buckaddr);
		bf.write(r_head, sizeoffset);
	}
	return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key)		// DONE
{
	hash<string> str_hash;					// hash function assigns int to string
	unsigned int hashValue = str_hash(key);
	unsigned int buck = hashValue%head;
	int buckaddr = (2 * sizeoffset) + (sizebucket * buck);	// find address of specified bucket

	Bucket b(0);
	bf.read(b, buckaddr);

	if (b.nodeaddr == 0)			// if bucket is empty
	{
		DiskMultiMap::Iterator it;	// return invalid iterator
		return it;
	}
	else
	{
		DiskNode n;
		bf.read(n, b.nodeaddr);
		BinaryFile::Offset curraddr = b.nodeaddr;
		bool flag = false;
		int setoff = 0;

		while (flag == false)		// search for desired key at specified bucket
		{ 
			if (key == n.key)		// for first matching key that is found
			{
				DiskMultiMap::Iterator it(key, curraddr, n.next, buckaddr, &bf);
				return it;			// return iterator pointing to that node
			}

			if (setoff == 0)
			{
				curraddr = n.next;		// move to next node
				bf.read(n, curraddr);
			}

			if (setoff != 0)			// condition to exit loop
				flag = true;

			if (n.next == buckaddr)		// if reached end of linked list
				setoff++;
		}
		DiskMultiMap::Iterator it;		// return invalid iterator if key not found
		return it;
	}
}

DiskMultiMap::Iterator::Iterator()		// DONE
{
	valid = false;		// invalid iterator constructor
}

DiskMultiMap::Iterator::Iterator(const string& key, BinaryFile::Offset c, BinaryFile::Offset n, BinaryFile::Offset a, BinaryFile* bfptr)	// DONE
	: cur(c), next(n), buckaddress(a), ptr(bfptr)
{
	valid = true;				// valid iterator constructor
	strcpy(hold, key.c_str());
}

bool DiskMultiMap::Iterator::isValid() const	// DONE
{
	return valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()	// DONE
{
	if (!isValid())		// return itself if invalid
		return *this;

	DiskNode n;
	ptr->read(n, cur);
	
	while (1)
	{
		cur = n.next;

		if (n.next == buckaddress)
		{
			valid = false;			// if at end of linked list
			return *this;
		}

		ptr->read(n, cur);
		next = n.next;				// increment iterator to point to next node in list

		if (strcmp(n.key, hold) == 0)
			return *this;
	}
}

MultiMapTuple DiskMultiMap::Iterator::operator*()	// DONE
{
	if (!isValid())
	{
		MultiMapTuple empty;	// return empty multimaptuple if invalid
		empty.key = "";
		empty.value = "";
		empty.context = "";
		return empty;
	}

	DiskNode n;
	ptr->read(n, cur);

	MultiMapTuple m;		// retrun multimaptuple with data at that node
	m.key = n.key;
	m.value = n.value;
	m.context = n.context;

	return m;
}

int DiskMultiMap::erase(const string& key, const string& value, const string& context)	// DONE
{
	const char* akey = key.c_str();
	const char* aval = value.c_str();
	const char* acont = context.c_str();		// convert to cstrings
	int removed = 0;

	hash<string> str_hash;
	unsigned int hashValue = str_hash(key);			// same hash function as in insert
	unsigned int buck = hashValue%head;
	int buckaddr = (2 * sizeoffset) + (sizebucket * buck);
	Bucket b(0);
	bf.read(b, buckaddr);

	if (b.nodeaddr == 0)		// if bucket is empty, return 
		return 0;

	DiskNode temp;
	bf.read(temp, b.nodeaddr);
	BinaryFile::Offset prev = 0;
	BinaryFile::Offset tempaddr = temp.next;

	while (tempaddr != b.nodeaddr)
	{
		if (strcmp(akey, temp.key) == 0 && strcmp(aval, temp.value) == 0 && strcmp(acont, temp.context) == 0)
		{								// if data matches 
			BinaryFile::Offset rem;

			if (prev == 0)				// if first node in list
			{
				rem = b.nodeaddr;		// remember node that is deleted
				if (tempaddr == buckaddr)
					b.nodeaddr = 0;
				else
					b.nodeaddr = tempaddr;

				bf.write(b, buckaddr);
			}
			else
			{							// if middle node in list
				DiskNode holder;
				bf.read(holder, prev);
				rem = holder.next;
				holder.next = tempaddr;
				bf.write(holder, prev);
			}

			if (r_head == sizeoffset)		// if "removed" list is empty
			{
				r_head = rem;				// add deletd node to "removed" list
				bf.write(r_head, sizeoffset);
				DiskNode r_temp;
				bf.read(r_temp, r_head);
				r_temp.next = sizeoffset;
				bf.write(r_temp, r_head);
			}
			else
			{								// if "removed" already contains nodes
				DiskNode r_temp;
				bf.read(r_temp, rem);		// add newly removed node to front of list
				r_temp.next = r_head;
				bf.write(r_temp, rem);
				r_head = rem;
				bf.write(r_head, sizeoffset);
			}
			removed++;		// count number of nodes deleted
		}
		else
		{							// if data does not match
			if (prev == 0)
				prev = b.nodeaddr;	
			else
			{
				DiskNode holder;
				bf.read(holder, prev);
				prev = holder.next;
			}
		}

		if (tempaddr == buckaddr)		// if at end of list
			tempaddr = b.nodeaddr;
		else
		{
			bf.read(temp, tempaddr);		// otherwise check next node and go to top of loop
			tempaddr = temp.next;
		}
	}
	return removed;			// return number of nodes removed
}