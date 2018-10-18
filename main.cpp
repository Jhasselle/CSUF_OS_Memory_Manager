// Jason Hasselle

#include "MemoryManager.h"

using namespace std;

// Keeps track of time 
// Directs the functionality of the memory mamanger
void Simulation() {

	int timeLimit = 10000;
	int memorySize, pageSize = 0, choice;
	int currentInstant = 0, elapsedTime = 0;

	// Collect memory size from user
	cout << "Memory size> ";
	cin >> memorySize;

	// Collect page size from user
	cout << "Page Size (1: 100, 2: 200, 3: 400)> ";
	cin >> choice;

	switch (choice)
	{
	case 1: pageSize = 100;
		break;
	case 2: pageSize = 200;
		break;
	case 3: pageSize = 400;
		break;
	default: cout << "Invalid entry. Program Terminating...\n";
		system("PAUSE");
		exit(1);
	}

	cout << "Memory Size: " << memorySize << '\n'
		<< "Page Size: " << pageSize << '\n';

	// Initializing our memory manager.
	MemoryManager MMananger(memorySize, pageSize, timeLimit);

	// The instant in which the first process(es) arrive.
	currentInstant = MMananger.getNextInstant();

	// Our main loop; the meat and potatoes.
	while ((currentInstant < timeLimit) && !MMananger.isComplete()) {

		cout << "\nt = " << currentInstant << ":";
		MMananger.computeCurrentInstant(currentInstant);
		currentInstant = MMananger.getNextInstant();
	}

	// The processes ran out of time. RIP.
	if (elapsedTime >= timeLimit && !MMananger.isComplete()) {
		cout << "More than 10000ms has elapsed. Shutting down." << endl;
	}
	// Or, JK, all is good.
	else
	{
		cout << "\nProgram shutdown initiated.\n\n";
		MMananger.calcTurnaroundTime();
	}
}

// You may be wondering why I didn't put all the preceeding code within main();
//  because then it wouldn't be so cute and abstracted. Look at this little guy!

int main() {
	Simulation();
	return 0;
}
