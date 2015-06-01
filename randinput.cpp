#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

int main (int argc, char * const argv[]) {

	int num = 20;

	if (argc == 2) {
		num = atoi(argv[1]);
	}

	srand(time(NULL));
	
	for (int i = 0; i < num; i++) {
		cout << ((double) rand() / (RAND_MAX/100.0)) << "\t" << ((double) rand() / (RAND_MAX/100.0)) << "\n";
	}

	return 0;
}