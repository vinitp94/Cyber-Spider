# Cyber Spider

Cyber Spider is an algorithm heavy project that simulates crawling through telemetry log data from government computers in order to predict future cyberattacks. The thing that makes this project so interesting is the size of the input; we are given trillions of lines of log data, so any ordinary implementation wouldn't do. Below, the technology and implementation details are covered. The spec for the project can be viewed [here][spec].

## Technologies

This project is written in pure C++. The files are split up as they typically are; headers and source.

## Implementation

The two main classes written to implement this program were [DiskMultiMap][multimap] and [IntelWeb][intelweb].
The first is in charge of storing all of the input data. Because the input was so large, all of the data had to be stored on disk using a binary file, which can be found [here][binaryfile]. To do so, I built a hashmap on disk by creating a series of buckets that all store linked lists full of data in the DiskMultiMap class.

The class that does the heavy lifting in this program is IntelWeb. This class stores all of the input data from a given file in an instance of DiskMultiMap. The functions to note here are:

- IntelWeb::ingest()

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

- IntelWeb::purge()

- IntelWeb::crawl()

[spec]: ./spec.pdf
[multimap]: ./DiskMultiMap.cpp
[intelweb]: ./IntelWeb.cpp
[binaryfile]: ./BinaryFile.h
