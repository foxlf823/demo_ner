/*
 * test.cpp
 *
 *  Created on: Dec 20, 2015
 *      Author: fox
 */



#include <set>
#include <iostream>
#include <assert.h>
#include "FoxUtil.h"
#include <fstream>
#include <map>
#include <algorithm>



using namespace std;
using namespace fox;

vector< set<int> > getCompleteGraph(vector< vector<int> > & table) {

	map<int, set<int> > relationMap;
	for(int i=0;i<table.size();i++) {
		for(int j=0;j<i;j++) {
			if(table[i][j]==1) {
				// add relations based on i
				map<int, set<int> >::iterator it = relationMap.find(i);
				if (it != relationMap.end()) {
					it->second.insert(j);
				} else {
					set<int> relationNode;
					relationNode.insert(j);
					relationMap.insert(map<int, set<int> >::value_type(i, relationNode));
				}
				// add relations based on j
				it = relationMap.find(j);
				if (it != relationMap.end()) {
					it->second.insert(i);
				} else {
					set<int> relationNode;
					relationNode.insert(i);
					relationMap.insert(map<int, set<int> >::value_type(j, relationNode));
				}
			}
		}
	}

	vector< set<int> > graphs; // each set<int> denotes a complete graph
	// in the initial state, each node is a complete graph
	for(int i=0;i<table.size();i++) {
		set<int> s;
		s.insert(i);
		graphs.push_back(s);
	}

	bool changed = false;
	do {
		changed = false;

		vector< set<int> >::iterator graph = graphs.begin();
		for(; graph!=graphs.end();graph++) {
			// graph e.g. {0,1}
			set<int> intersect; // record the nodes connected with all the nodes in this graph
			bool isolateNode = false;

			set<int>::iterator iterTargetNode = graph->begin();
			for(; iterTargetNode!=graph->end();iterTargetNode++) {
				int node = *iterTargetNode;

				map<int, set<int> >::iterator iterRelationMap = relationMap.find(node);
				if (iterRelationMap == relationMap.end()) { // no nodes connected with this node
					isolateNode = true;
					break;
				}

				set<int> & relationMapOfThisNode = iterRelationMap->second;

				if(iterTargetNode == graph->begin()) { // if first, add it directly
					set<int>::iterator iterTemp=relationMapOfThisNode.begin();
					for(;iterTemp!=relationMapOfThisNode.end();iterTemp++)
						intersect.insert(*iterTemp);
				} else {
					vector<int> temp(table.size(), -1);
					set_intersection(intersect.begin(), intersect.end(), relationMapOfThisNode.begin(), relationMapOfThisNode.end(), temp.begin());
					intersect.clear();
					for(int i=0;i<temp.size();i++) {
						if(temp[i] == -1)
							continue;
						intersect.insert(temp[i]);
					}

					if(intersect.empty()) // we don't need to consider the subsequent nodes
						break;
				}
			}

			if(isolateNode) {
				// do nothing
			} else {
				if(intersect.empty()) {
					// do nothing
				} else {
					// add one node to this graph
					graph->insert(*intersect.begin());
					changed = true;
				}
			}

		} // graph

		if(changed == true) {
			// removed the same graph and get ready for the next iteration
			vector< set<int> > temp;

			for(int i=0;i<graphs.size();i++) {
				set<int> & old = graphs[i];

				bool same = false;
				for(int j=0;j<temp.size();j++) {
					set<int> & new_ = temp[j];

					if(old == new_) {
						same = true;
						break;
					}
				}

				if(same==false) {
					temp.push_back(old);
				}
			}

			graphs.clear();
			for(int i=0;i<temp.size();i++)
				graphs.push_back(temp[i]);

		}


	} while(changed);



	return graphs;
}

int main(int argc, char **argv)
{
	const int size = 0;
	//const int size = 6;
	vector< vector<int> > relationTable;
	relationTable.resize(size);
	for (int idx = 0; idx < size; idx++){
		for (int i = 0; i < size; i++)
			relationTable[idx].push_back(0);
	}



/*	relationTable[1][0] = 1;
	relationTable[2][1] = 1;*/

/*	relationTable[1][0] = 1;
	relationTable[2][0] = 1;
	relationTable[2][1] = 1;*/

/*	relationTable[1][0] = 1;
		relationTable[2][0] = 1;
		relationTable[3][0] = 1;
		relationTable[3][1] = 1;
		relationTable[3][2] = 1;
		relationTable[4][2] = 1;*/

	vector< set<int> > graphs = getCompleteGraph(relationTable);

	set<int > a;
	for(int i=13;i>=0;i--) {
		a.insert(i);
	}

	return 0;
}




