/*
 * Word2Vec.h
 *
 *  Created on: Dec 21, 2015
 *      Author: fox
 */

#ifndef WORD2VEC_H_
#define WORD2VEC_H_
#include <ctime>
#include <map>
#include "FoxUtil.h"
#include "Utf.h"

using namespace std;

namespace fox {

typedef union {
	char bytes[4];
	float value;
} FloatUnion;

class Word2Vec {
public:
	map<string, float*>* wordMap;
	int dimension;

	Word2Vec() {
		wordMap = new map<string, float*>();
	}
	virtual ~Word2Vec() {

		for(map<string,float*>::iterator it = wordMap->begin();it != wordMap->end();it++) {

			delete it->second;
		}

		delete wordMap;
	}

	void loadFromBinFile(const string& file, bool bNorm, bool bStringNorm) {
		ifstream ifs;
		ifs.open(file.c_str());
		unsigned long memused = 0;

		int wordNumber = atoi(readString(ifs).c_str());
		dimension = atoi(readString(ifs).c_str());
		char nouse;
		try {
			for(int i=0;i<wordNumber; i++) {
				string word = readString(ifs);

				if(bStringNorm) {
					word = normalize_to_lowerwithdigit(word);
				}
				float* vector = new float[dimension];
				memused += 4*dimension;
				double norm = 0;
				for(int j=0;j<dimension;j++) {
					double temp = readFloat(ifs);
					norm += temp*temp;
					*(vector+j) = temp;
				}

				if(bNorm) {
					norm = sqrt(norm);
					for(int j=0;j<dimension;j++) {
						*(vector+j) /= norm;
					}
				}

				wordMap->insert(map<string, float*>::value_type(word, vector));
				ifs.read(&nouse, 1);
			}
		}
		catch (std::bad_alloc) {
			cout<<"bad alloc: "<<memused<<endl;
			exit(0);
		}
		//cout<<memused<<endl;

		ifs.close();

/*		for(map<string,float*>::iterator it = wordMap.begin();it != wordMap.end();it++) {
			for(int j=0;j<dimension;j++) {
				cout<<*(it->second+j)<<" ";
			}
			cout<<endl;
		}*/
	}

	void getEmbedding(double* E, int embSize, const vector<string>& knownWords, const string& unknownKey,
			const map<string, int>& IDs) {
		assert(embSize == dimension);
		vector<int> uninitialIds;
		int unknownID = -1;
		double sum[dimension];
		for(int i=0;i<dimension;i++)
			sum[i] = 0;

		int count = 0;
		// try to use pre-trained embeddings
		for(int i=0;i<knownWords.size();i++) {
			if(knownWords[i] == unknownKey) {
				unknownID = IDs.find(unknownKey)->second; // assume that unknownID does exist
				continue;
			}

			map<string, float*>::iterator itWordMap = wordMap->find(knownWords[i]);
			int id = IDs.find(knownWords[i])->second; // assume that IDs and knownWords are compatible
			if(itWordMap != wordMap->end()) {
				for(int j=0;j<dimension;j++) {
					*(E+id*dimension+j) = *(itWordMap->second+j);
					sum[j] += *(E+id*dimension+j);
				}
				count++;
			} else {
				uninitialIds.push_back(id);
			}
		}
		// unknown is the average of all words
		if(count==0)
			count = 1;
		assert(unknownID != -1);
		for(int j=0;j<dimension;j++) {
			*(E+unknownID*dimension+j) = sum[j]/count;
		}
		// word not in the pre-trained will use the unknown
		for(int i=0;i<uninitialIds.size();i++) {
			for(int j=0;j<dimension; j++) {
				*(E+uninitialIds[i]*dimension+j) = *(E+unknownID*dimension+j);
			}
		}

		cout<<"oov from pretrained emb is "<<uninitialIds.size()*1.0/knownWords.size()<<endl;
	}

private:
	string readString(ifstream& ifs) {
		string s;
		char c = 0;
		ifs.read(&c, sizeof(char));
		while(c != 32 && c!=10) {
			s += c;
			ifs.read(&c, sizeof(char));
		}
		return s;
	}

	float readFloat(ifstream& ifs) {
		FloatUnion fu;
		ifs.read(fu.bytes, 4);

		return fu.value;
	}



};


}



#endif /* WORD2VEC_H_ */
