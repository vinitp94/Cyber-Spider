#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		Iterator(const std::string& key, BinaryFile::Offset c, BinaryFile::Offset n, BinaryFile::Offset a, BinaryFile* bfptr);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();
		
	private:
		bool valid;
		BinaryFile::Offset cur, next, buckaddress;
		BinaryFile* ptr;
		char hold[121];
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

private:
	BinaryFile bf;

	struct DiskNode
	{
		char key[121];
		char value[121];
		char context[121];
		BinaryFile::Offset next;
	};

	struct Bucket
	{
		Bucket(BinaryFile::Offset n)
			: nodeaddr(n) {}
		BinaryFile::Offset nodeaddr;
	};

	BinaryFile::Offset head, r_head;
	int sizenode, sizebucket, sizeoffset;
};

#endif // DISKMULTIMAP_H_
