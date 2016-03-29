#include <iostream>
#include <vector>

#include "PLL.h"

using namespace std;

int main()
{
	ios_base::sync_with_stdio(false);

	uint32_t source, destination;
	vector<pair<uint32_t, uint32_t>> es;
	while (cin >> source >> destination){
		es.push_back(make_pair(source, destination));
	}
	PLL g(es);
	g.construct_index();

	cout << "R" << endl;

	char operation;
	cin.clear();
	cin >> operation;
	while (cin >> operation){
		if (operation == 'F'){
			cout.flush();
		}
		else{
			cin >> source >> destination;
			switch (operation){
			case 'Q':
				cout << g.query_distance(source, destination) << endl;
				break;
			case 'A':
				g.insert_edge(source, destination);
				break;
			case 'D':
				g.remove_edge(source, destination);
				break;
			}
		}
	}

	return 0;
}