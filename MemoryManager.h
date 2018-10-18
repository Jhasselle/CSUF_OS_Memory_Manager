// Jason Hasselle


#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H


#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Process
{
	int id;
	int arrivalTime;
	int allocationTime;
	int lifeTime;
	int memRequirement;


	// Still deciding whether these, acting as flags, will,-> 
	// -> make calculating the next time instant easier.
	bool running = false;
	bool queued = false;
	bool completed = false;


	// For debugging purposes
	void print() {
		cout << "id: " << id << endl;
		cout << "arrival time: " << arrivalTime << endl;
		cout << "life time: " << lifeTime << endl;
		cout << "memory requirement: " << memRequirement << endl << endl;
	}
};

class MemoryManager {
public:

	// Parameterized Constructor. There is no implemented default constructor
	MemoryManager(int newMemorySize, int newPageSize, int newTimeLimit) {

		memorySize = newMemorySize;
		pageSize = newPageSize;

		// Because we check these values on user input, ->
		// -> they can be assumed to be perfectly divisible.
		numOfPages = memorySize / pageSize;
		freeMapSpace = numOfPages;
		timeLimit = newTimeLimit;
		currentInstant = 0;

		iFile.open("in1.txt");
		if (iFile.eof()) {
			// TODO: error; self destruct.
		}

		// The input file has a simple header.
		// First character within the input file is the totaly number of processes.
		iFile >> numOfProcesses;

		// We must dynamically allocate the arrays
		ProcessList = new Process[numOfProcesses];
		MemoryMap = new int*[numOfPages];

		// Initialize the values of ProcessQueue and MemoryMap
		initializeMap();

		// Reading the values of each process and storing them into a list
		for (int i = 0; i < numOfProcesses; ++i) {
			iFile >> ProcessList[i].id;
			iFile >> ProcessList[i].arrivalTime;
			iFile >> ProcessList[i].lifeTime;

			int numOfSpaces;
			iFile >> numOfSpaces;

			// Temporary variables
			int chunkRequirement = 0;
			int totalRequirement = 0;

			// In case we have multiple memory sections, 
			// we need to compute the total needed memory for the process.
			for (int j = 0; j < numOfSpaces; ++j) {
				iFile >> chunkRequirement;
				totalRequirement += chunkRequirement;
			}
			ProcessList[i].memRequirement = totalRequirement;
			//ProcessList[i].print();
		}

		// We have no use for the input file as of this point.
		iFile.close();

	}

	// Set the initial values within the map and the queue to 0 at each index.
	// because, why not?
	void initializeMap() {

		for (int i = 0; i < numOfPages; ++i) {
			MemoryMap[i] = new int[2];
			MemoryMap[i][0] = -1;
			MemoryMap[i][1] = -1;
		}
	}

	// Returns whether there are still processes that need to execute
	bool isComplete() {
		bool result = true;
		for (int i = 0; i < numOfProcesses; ++i) {
			if (!ProcessList[i].completed) {
				result = false;
			}
		}
		return result;
	}

	// Returns the next time instant
	int getNextInstant() {

		// Starting off with the highest possible value
		int result = timeLimit + 1;

		// Searching amongst arrivals times
		for (int i = 0; i < numOfProcesses; ++i) {
			if (!ProcessList[i].queued && !ProcessList[i].completed && !ProcessList[i].running)
			{
				if (result > ProcessList[i].arrivalTime) {
					result = ProcessList[i].arrivalTime;
				}
			}
		}

		// Searching amongst completion times
		for (int i = 0; i < numOfProcesses; ++i) {
			if (ProcessList[i].running)
			{
				if (result >(ProcessList[i].allocationTime + ProcessList[i].lifeTime)) {
					result = (ProcessList[i].allocationTime + ProcessList[i].lifeTime);
				}
			}
		}

		currentInstant = result;
		return currentInstant;
	}


	//
	//  The Big Kahuna of this class.  //
	void computeCurrentInstant(int instant) {


		// Lastly, print processes that complete
		for (int i = 0; i < numOfProcesses; ++i) {
			if ((ProcessList[i].allocationTime + ProcessList[i].lifeTime) == currentInstant) {
				cout << "\tProcess " << ProcessList[i].id << " completes" << endl;
				if (memoryMapDealloc(i)) {
					// success
					ProcessList[i].running = false;
					ProcessList[i].completed = true;
				}

				memoryMapPrint();
			}
		}


		// First loop, compute the processes arriving
		for (int i = 0; i < numOfProcesses; ++i) {
			if (ProcessList[i].arrivalTime == currentInstant) {
				cout << "\tProcess " << ProcessList[i].id << " arrives" << endl;
				ProcessQueue.push_back(i);
				ProcessList[i].queued = true;
				queuePrint();
			}
		}


		// Second loop, move processes into memory map
		// change process state to running
		// set allocation time

		// Looping through the process queue to see what we can allocate with the memory map
		int j = 0;
		for (vector<int>::iterator i = ProcessQueue.begin(); i != ProcessQueue.end(); ++i) {
			if (ProcessQueue.at(j) != -1)
			{
				if (memoryMapAlloc(*i)) {
					ProcessList[*i].allocationTime = instant;
					ProcessList[*i].running = true;
					ProcessList[*i].queued = false;
					// remove from queue
					cout << "\tMM moves Process  " << ProcessList[*i].id << " to memory" << endl;
					//ProcessQueue.erase(&ProcessQueue[j]);
					ProcessQueue.at(j) = -1;
					// print input queue
					queuePrint();
					// print memory map
					memoryMapPrint();
				}
			}
			++j;

		}
	}

	// 
	bool memoryMapAlloc(int processIndex) {

		bool result = false;

		int neededPages = (ProcessList[processIndex].memRequirement / pageSize);

		if (ProcessList[processIndex].memRequirement % pageSize) {
			++neededPages;
		}
		if (neededPages <= freeMapSpace) {
			result = true;


			int pageNum = 1;
			// Sequentially parse the MemoryMap array
			// 
			for (int i = 0; i < numOfPages; ++i) {
				if ((MemoryMap[i][0] == -1) && (neededPages > 0)) {
					--neededPages;
					--freeMapSpace;
					MemoryMap[i][0] = processIndex;
					MemoryMap[i][1] = pageNum;
					++pageNum;
				}
			}
		}

		return result;
	}

	bool memoryMapDealloc(int processIndex) {
		bool result = false;

		for (int i = 0; i < numOfPages; ++i) {
			if (MemoryMap[i][0] == processIndex) {
				result = true;
				++freeMapSpace;
				MemoryMap[i][0] = -1;
				MemoryMap[i][1] = -1;
			}
		}

		return result;
	}


	//
	void memoryMapPrint() {
		bool free = false;
		int firstStartPage = 0;
		int numOfFree = 0;

		cout << "\tMemory Map: \n";

		for (int i = 0; i < numOfPages; ++i) {

			// Free Space
			if (MemoryMap[i][0] == -1) {

				if (!free) firstStartPage = i * pageSize;
				free = true;
				++numOfFree;
				if (i + 1 == numOfPages) {
					cout << "\t\t" << firstStartPage << "-" << firstStartPage + (((numOfFree)* pageSize) - 1) << ": Free frame(s)" << endl;
				}
			}
			else {
				if (free) {
					cout << "\t\t" << firstStartPage << "-" << firstStartPage + (((numOfFree)* pageSize) - 1) << ": Free frame(s)" << endl;
					numOfFree = 0;
				}
				free = false;
				cout << "\t\t" << i * pageSize << "-" << ((i + 1) * pageSize) - 1 << ": ";
				cout << "Process: " << ProcessList[MemoryMap[i][0]].id << ", Page " << MemoryMap[i][1] << endl;
			}
		}
	}



	// Prints the processes currently queued
	void queuePrint() {

		cout << "\tInput Queue:[ ";
		for (vector<int>::iterator i = ProcessQueue.begin(); i != ProcessQueue.end(); ++i) {
			if (*i != -1) {
				cout << ProcessList[*i].id << " ";
			}
		}
		cout << "]" << endl;
	}

	// For debugging purposes
	void debugPrint() {
		for (int i = 0; i < numOfProcesses; ++i) {
			ProcessList[i].print();
		}
	}

	//Calulates the average turnaround time of the processes
	void calcTurnaroundTime() {
		double turnaroundTime = 0;
		//check for completion
		for (int i = 0; i < numOfProcesses; ++i) {
			if (ProcessList[i].completed) {
				turnaroundTime += (ProcessList[i].allocationTime + ProcessList[i].lifeTime - ProcessList[i].arrivalTime);
			}
		}
		turnaroundTime /= numOfProcesses;

		cout << "Average Turnaround Time: ";
		cout << fixed;
		cout << setprecision(2);
		cout << turnaroundTime << " ";
		turnaroundTime *= numOfProcesses;
		cout << "(" << turnaroundTime << "/" << numOfProcesses << ")\n\n";
	}

	// Cleaning up our heap
	~MemoryManager() {
		delete[] ProcessList;
		delete[] MemoryMap;

	}

private:
	ifstream iFile;
	Process * ProcessList;

	// ProcessQueue stores the index of processes currently queued.
	vector<int> ProcessQueue;

	// MemoryMap stores the index of processes within it.
	int ** MemoryMap;

	int numOfProcesses;
	int numOfPages;
	int memorySize;
	int pageSize;
	int currentInstant;
	int timeLimit;
	int freeMapSpace;
};

#endif // !MEMORY_MANAGER_CPP
