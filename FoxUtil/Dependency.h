/*
 * Dependency.h
 *
 *  Created on: Apr 2, 2016
 *      Author: fox
 */

#ifndef DEPENDENCY_H_
#define DEPENDENCY_H_

#include <vector>

using namespace std;

namespace fox {

/*
 * This is a tool class for processing the projective dependency tree (PDT).
 */
class Dependency {
public:
	Dependency() {

	}

	/*
	 * dep denotes the parent index of each word in the PDT.
	 * We follow the rules of Stanford Dependency whose indexes start from 1 (ROOT is 0);
	 * level denotes the node depths in the PDT. ROOT is 0, so the son of ROOT is 1.
	 * Some words may not exist in the PDT, we force their levels as 1.
	 */
	static void getLevelForEachNode(const vector<int> &dep, vector<int> &level) {
		int nodeNum = dep.size();
		for(int i=0;i<nodeNum;i++) {
			int node = i;
			int l = 1;
			while(dep[node]>0) {
				l++;
				node = dep[node]-1;
			}
			level.push_back(l);
		}
	}

	// paths should be deleted by caller
	static void deepFirstSearch(int current, const vector<Token>& sent, vector<vector<int>*>& paths,
			vector<int>& temp) {
		if(current==-1) {
			temp.push_back(0);
			vector<int>* path = new vector<int>(temp);
			paths.push_back(path);
			return;
		}

		if(sent[current].depGov == -1)
			return;

		for(int i=temp.size()-1;i>=0;i--) {
			if(temp[i]==current+1) {
				temp.push_back(current+1);
				return;
			}
		}

		temp.push_back(current+1);


			deepFirstSearch(sent[current].depGov-1, sent, paths, temp);
			temp.erase(temp.end()-1);


	}

	// a and b is the token idx, and they must be in the same sentence
	// return the token idx of common ancestor, sdpA is the node idx of sdp from a to common ancestor
	// if return -1, it should be root, if return -2, no common ancestor
	static int getCommonAncestor(const vector<Token>& sent, int a, int b, vector<int>& sdpA, vector<int>& sdpB) {
		if(a==b) {
			sdpA.push_back(a+1);
			sdpB.push_back(b+1);
			return a;
		}

		if(sent[a].depGov == -1 || sent[b].depGov == -1)
			return -2;

		vector<vector<int>*> pathsA;
		vector<int> temp;
		Dependency::deepFirstSearch(a, sent, pathsA, temp);

		temp.clear();
		vector<vector<int>*> pathsB;
		Dependency::deepFirstSearch(b, sent, pathsB, temp);


		int size = pathsA.size()*pathsB.size();
		int* dist = new int[size];
		memset(dist, 0x7fffffff, size*sizeof(int));
		int* commonAncestor = new int[size];
		memset(commonAncestor, -1, size*sizeof(int));

		for(int i=0;i<pathsA.size();i++) {
			vector<int>* path1 = pathsA[i];
			int j=0;
OUTER:		for(;j<pathsB.size();) {
				vector<int>* path2 = pathsB[j];

				for(int k=0;k<path1->size();k++) {
					for(int m=0;m<path2->size();m++) {
						if((*path1)[k] == (*path2)[m]) {
							*(dist+(i*pathsB.size()+j)) = k+m;
							*(commonAncestor+(i*pathsB.size()+j)) = (*path1)[k];
							j++;
							goto OUTER;
						}
					}
				}

				j++;
			}
		}

		int shortest = 0x7fffffff;
		int k = -1;
		for(int i=0;i<size;i++) {
			if(*(dist+i)<shortest) {
				shortest = *(dist+i);
				k = i;
			}
		}

		if(k==-1)
			return -2;

		int aPathIdx = k/pathsB.size();
		for(int i=0;i<pathsA[aPathIdx]->size();i++) {
			sdpA.push_back((*pathsA[aPathIdx])[i]);

			if((*pathsA[aPathIdx])[i] == *(commonAncestor+k))
				break;
		}

		int bPathIdx = k%pathsB.size();
		for(int i=0;i<pathsB[bPathIdx]->size();i++) {
			sdpB.push_back((*pathsB[bPathIdx])[i]);

			if((*pathsB[bPathIdx])[i] == *(commonAncestor+k))
				break;
		}

		int ret = *(commonAncestor+k)-1;

		// free all the pointers
		for(int i=0;i<pathsA.size();i++) {
			delete pathsA[i];
		}
		for(int i=0;i<pathsB.size();i++) {
			delete pathsB[i];
		}

		delete dist;
		delete commonAncestor;

		return ret;
	}
};

}




#endif /* DEPENDENCY_H_ */
