#include "IntelWeb.h"
#include "DiskMultiMap.h"
#include "InteractionTuple.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <set>

using namespace std;

IntelWeb::IntelWeb()
{}

IntelWeb::~IntelWeb()		// FREE ANY DYNAMICALLY ALLOCATED MEMORY
{
	close();
}

bool IntelWeb::createNew(const string& filePrefix, unsigned int maxDataItems)		// DONE
{
	close();
	int bucknum = int(maxDataItems / 0.7);

	bool success = m_from.createNew(filePrefix + "-from-hash-table.dat", bucknum);
	bool success2 = m_to.createNew(filePrefix + "-to-hash-table.dat", bucknum);
	bool success3 = prevalence.createNew(filePrefix + "-prevalence-hash-table.dat", 2 * bucknum);
	
	if (success && success2 && success3)
		return true;
	else
	{
		close();
		return false;
	}
}

bool IntelWeb::openExisting(const string& filePrefix)			// DONE
{
	close();

	bool success = m_from.openExisting(filePrefix + "-from-hash-table.dat");
	bool success2 = m_to.openExisting(filePrefix + "-to-hash-table.dat");
	bool success3 = prevalence.openExisting(filePrefix + "-prevalence-hash-table.dat");
	
	if (success && success2 && success3)
		return true;
	else
	{
		close();
		return false;
	}
}

void IntelWeb::close()		// DONE
{
	m_from.close();
	m_to.close();
	prevalence.close();
}

bool IntelWeb::ingest(const string& telemetryFile)		// DONE
{
	ifstream inf(telemetryFile);
	if (!inf)
		return false;

	string line;
	while (getline(inf, line))
	{
		istringstream iss(line);
		string context;
		string from;
		string to;
		if (!(iss >> context >> from >> to))
			continue;

		if (context.size() > 120 || from.size() > 120 || to.size() > 120)
			continue;

		m_from.insert(from, to, context);
		m_to.insert(to, from, context);
		prevalence.insert(from, "", "");
		prevalence.insert(to, "", "");
	}
	return true;
}

unsigned int IntelWeb::crawl(const vector<string>& indicators, unsigned int minPrevalenceToBeGood, 
	vector<string>& badEntitiesFound, vector<InteractionTuple>& badInteractions)
{	
	unordered_map<string, DiskMultiMap::Iterator> fromIts;
	unordered_map<string, DiskMultiMap::Iterator> toIts;
	unordered_map<string, bool> isGood;
	queue<string> badEntities;
	set<string> o_BadEntities;
	set<InteractionTuple> o_InteractionTuples;

	badEntitiesFound.clear();
	badInteractions.clear();

	for (int i = 0; i < indicators.size(); i++)
		badEntities.push(indicators[i]);

	while (!badEntities.empty())
	{
		DiskMultiMap::Iterator fromIter;
		DiskMultiMap::Iterator toIter;
		auto it = fromIts.find(badEntities.front());
		auto it2 = toIts.find(badEntities.front());
		fromIter = m_from.search(badEntities.front());
		toIter = m_to.search(badEntities.front());

		if (fromIter.isValid())
		{
			if (it == fromIts.end())
				fromIts.insert(make_pair(badEntities.front(), fromIter));
			else
				fromIter = it->second;
			
			if (fromIter.isValid())
			{
				o_BadEntities.insert(badEntities.front());
				MultiMapTuple mmt1 = *fromIter;
				auto ip = isGood.find(mmt1.value);
				bool prevIsGood;
				
				if (ip == isGood.end())
				{
					prevIsGood = countPrevalence(mmt1.value, minPrevalenceToBeGood);
					isGood.insert(std::make_pair(mmt1.value, prevIsGood));
				}
				else
					prevIsGood = ip->second;

				if (!prevIsGood)
				{
					o_BadEntities.insert(mmt1.value);
					badEntities.push(mmt1.value);
				}

				InteractionTuple it1;
				it1.context = mmt1.context;
				it1.from = mmt1.key;
				it1.to = mmt1.value;
				o_InteractionTuples.insert(it1);
				++fromIter;
				fromIts[badEntities.front()] = fromIter;
			}
		}

		if (toIter.isValid())
		{
			if (it2 == toIts.end())
				toIts.insert(make_pair(badEntities.front(), toIter));
			else
				toIter = it2->second;

			if (toIter.isValid())
			{
				o_BadEntities.insert(badEntities.front());
				MultiMapTuple mmt2 = *toIter;
				auto ip2 = isGood.find(mmt2.value);
				bool prevIsGood;

				if (ip2 == isGood.end())
				{
					prevIsGood = countPrevalence(mmt2.value, minPrevalenceToBeGood);
					isGood.insert(make_pair(mmt2.value, prevIsGood));
				}
				else
					prevIsGood = ip2->second;

				if (!prevIsGood)
				{
					o_BadEntities.insert(mmt2.value);
					badEntities.push(mmt2.value);
				}

				InteractionTuple it2;
				it2.context = mmt2.context;
				it2.from = mmt2.value;
				it2.to = mmt2.key;
				o_InteractionTuples.insert(it2);
				++toIter;
				toIts[badEntities.front()] = toIter;
			}
		}
		badEntities.pop();
	}

	for (auto it = o_BadEntities.begin(); it != o_BadEntities.end(); it++)
		badEntitiesFound.push_back(*it);

	for (auto it = o_InteractionTuples.begin(); it != o_InteractionTuples.end(); it++)
		badInteractions.push_back(*it);

	return (unsigned int)badEntitiesFound.size();
}

bool IntelWeb::purge(const string& entity)		// DONE
{
	bool purged = false;
	DiskMultiMap::Iterator it = m_from.search(entity);
	
	while (it.isValid())
	{
		MultiMapTuple m = *it;
		string k = m.key;
		string v = m.value;
		string c = m.context;
		++it;
		m_from.erase(k, v, c);
		m_to.erase(v, k, c);
		purged = true;
	}

	DiskMultiMap::Iterator it2 = m_to.search(entity);

	while (it2.isValid())
	{
		MultiMapTuple m = *it2;
		string k = m.key;
		string v = m.value;
		string c = m.context;
		++it2;
		m_to.erase(k, v, c);
		m_from.erase(v, k, c);
		purged = true;
	}

	prevalence.erase(entity, "", "");
	return purged;
}