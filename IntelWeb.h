#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

class IntelWeb
{
public:
	IntelWeb();
	~IntelWeb();
	bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
	bool openExisting(const std::string& filePrefix);
	void close();
	bool ingest(const std::string& telemetryFile);
	unsigned int crawl(const std::vector<std::string>& indicators,
		unsigned int minPrevalenceToBeGood,
		std::vector<std::string>& badEntitiesFound,
		std::vector<InteractionTuple>& badInteractions
		);
	bool purge(const std::string& entity);

private:
	bool countPrevalence(const std::string& key, unsigned int minPrev)
	{
		DiskMultiMap::Iterator it = prevalence.search(key);
		int counter = 0;

		while (it.isValid())
		{
			counter++;
			if (counter >= minPrev)
				return true;
			++it;
		}
		return false;
	}

	DiskMultiMap m_from;
	DiskMultiMap m_to;
	DiskMultiMap prevalence;
};

inline
bool operator<(const InteractionTuple& lhs, const InteractionTuple& rhs) {

	if (lhs.context < rhs.context)
		return true;

	if (lhs.context > rhs.context)
		return false;

	if (lhs.context == rhs.context) 
	{
		if (lhs.from < rhs.from)
			return true;
		if (lhs.from > rhs.from)
			return false;

		if (lhs.from == rhs.from) 
		{
			if (lhs.to < rhs.to)
				return true;
			if (lhs.to > rhs.to)
				return false;
			else
				return false;
		}
	}
	return false;
}

#endif // INTELWEB_H