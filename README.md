# Cyber Spider

Cyber Spider is an algorithm heavy project that simulates crawling through telemetry log data from government computers in order to predict future cyberattacks. The thing that makes this project so interesting is the size of the input; we are given trillions of lines of log data, so any ordinary implementation wouldn't do. The input is in the following form:

{ file/website # 1, file/website # 2, relationship between # 1 and # 2 }

Below, the technology and implementation details are covered. The spec for the project can be viewed [here][spec].

## Technologies

This project is written in pure C++. The files are split up as they typically are; headers and source.

## Implementation

The two main classes written to implement this program were [DiskMultiMap][multimap] and [IntelWeb][intelweb].
The first is in charge of storing all of the input data. Because the input was so large, all of the data had to be stored on disk using a binary file, which can be found [here][binaryfile]. To do so, I built a hashmap on disk by creating a series of buckets that all store linked lists full of data in the DiskMultiMap class. The data is stored in two different maps because the input data has two types of specified relationships between the files/websites; "to" and "from".

The class that does the heavy lifting in this program is IntelWeb. This class stores all of the input data from a given file in an instance of DiskMultiMap. The functions to note here are:

- IntelWeb::ingest(string&)

  This function had to read in all the data and store it in the DiskMultiMap in O(N) time

  ```c++
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
  ```

- IntelWeb::purge(string)

  This function was meant to clear all entities that match the passed in string in O(M) time where M is the number of entities that match. Because we have information stored in two different maps, both maps must be checked, and we cannot simply iterate through all the elements because that would be in O(N) time.

  ```c++
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
  ```

- IntelWeb::crawl()

[spec]: ./spec.pdf
[multimap]: ./DiskMultiMap.cpp
[intelweb]: ./IntelWeb.cpp
[binaryfile]: ./BinaryFile.h
