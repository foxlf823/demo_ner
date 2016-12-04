#ifndef _FOX_UTIL_H_
#define _FOX_UTIL_H_

#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <map>
#include <boost/serialization/vector.hpp>


using namespace std;

namespace fox {

double precision(int correct, int predict) {
	return 1.0*correct/predict;
}

double recall(int correct, int gold) {
	return 1.0*correct/gold;
}

double f1(int correct, int gold, int predict) {
	double p = precision(correct, predict);
	double r = recall(correct, gold);

	return 2*p*r/(p+r);
}

double f1(double p, double r) {

	return 2*p*r/(p+r);
}


template<typename T>
void dumpVector(const vector<T> & v, const string& name, int size) {
	cout<< name <<": ";
	for(int i=0;i<size;i++) {
	  cout<<v[i]<<" ";

	}
	cout<<endl;
}

template<typename T>
void dumpVectorP(const vector<T*> & v, const string& name, int size) {
	cout<< name <<": ";
	for(int i=0;i<size;i++) {
	  cout<<*v[i]<<" ";

	}
	cout<<endl;
}

string toLowercase(string str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

char toLowercase(char c) {
	if(c>=65 && c<=90)
		return c+32;
	else
		return c;
}

string lexicalPattern(string str) {
	for(uint i=0;i<str.length();i++) {
		if(isupper(str[i]))
			str[i] = 'A';
		else if(islower(str[i]))
			str[i] = 'a';
		else if(isdigit(str[i]))
			str[i] = '0';
		else
			str[i] = '*';
	}

	return str;
}

void split_bystring(const string& str, vector<string>& vec, const string separator) {
	//assert(vec.empty());
	vec.clear();
	string::size_type pos1 = 0, pos2 = 0;
	string word;
	while ((pos2 = str.find_first_of(separator, pos1)) != string::npos) {
	  word = str.substr(pos1, pos2 - pos1);
	  pos1 = pos2 + separator.size();
	  //if (!word.empty())
		vec.push_back(word);
	}
	word = str.substr(pos1);
	//if (!word.empty())
	  vec.push_back(word);
}

void split_bychar(const string& str, vector<string>& vec, const char separator = ' ') {
	//assert(vec.empty());
	vec.clear();
	string::size_type pos1 = 0, pos2 = 0;
	string word;
	while ((pos2 = str.find_first_of(separator, pos1)) != string::npos) {
	  word = str.substr(pos1, pos2 - pos1);
	  pos1 = pos2 + 1;
	  //if (!word.empty())
		vec.push_back(word);
	}
	word = str.substr(pos1);
	//if (!word.empty())
	  vec.push_back(word);
}

void split(const string& str, vector<string>& vec, const string& separatorSet) {
	vector<string> separator;
	fox::split_bychar(separatorSet, separator, '|');

	vec.clear();
	string::size_type pos1 = 0, pos2 = 0;
	string word;
	bool separatorFind = true;
	while(separatorFind) {
		separatorFind = false;
		int temp = 99999;
		for(int i=0;i<separator.size();i++) {
			if((pos2 = str.find_first_of(separator[i].at(0), pos1)) != string::npos && pos2<temp) {
				temp = pos2;
				separatorFind = true;
			}
		}
		pos2 = temp;
		  word = str.substr(pos1, pos2 - pos1);
		  pos1 = pos2 + 1;
			vec.push_back(word);
	}


}

class BrownClusterUtil {
public:
	map<string, string> word2cluster;

	BrownClusterUtil() {

	}

	BrownClusterUtil(const string& file) {
		load(file);
	}

	void load(const string& file) {
		ifstream ifs;
		ifs.open(file.c_str());

		string line;
		while(getline(ifs, line)) {
			//cout<<line<<endl;

			if(!line.empty()) {
				vector<string> splitted;
				fox::split_bychar(line, splitted, '\t');
				word2cluster.insert(map<string, string>::value_type(splitted[1], splitted[0]));


			}
		}

		ifs.close();
	}

	string get(const string& word) {
		map<string, string>::iterator it = word2cluster.find(word);
		if (it != word2cluster.end())
			return it->second;
		else
			return "";
	}
};



template<typename Type>
void writeObject(const string& file, const Type& object) {
	ofstream ofs(file.c_str());
	boost::archive::text_oarchive oa(ofs);
	oa << object;

}

template<typename Type>
void readObject(const string & file, Type& new_object) {
	ifstream ifs(file.c_str(), ios::binary);
	boost::archive::text_iarchive ia(ifs);
	ia >> new_object;
}


bool isNumber(char c) {
	if(c>=0x30 && c<=0x39)
		return true;
	else
		return false;
}



void randomInitEmb(double* emb, int embSize, const vector<string>& known, const string& unknownKey,
		const map<string, int>& IDs, bool bNorm, double initRange, int seed) {
	srand((uint)seed);
	int unknownID = -1;
	double sum[embSize];
	for(int i=0;i<embSize;i++)
		sum[i] = 0;

	int count = 0;
	for(int i=0;i<known.size();i++) {
		if(known[i] == unknownKey) {
			unknownID = IDs.find(known[i])->second;
			continue;
		}

		int id = IDs.find(known[i])->second;
		double norm = 0;
		for(int j=0;j<embSize;j++) {
			*(emb+id*embSize+j) = (1.0 * rand() / RAND_MAX) *initRange*2-initRange;
			norm += (*(emb+id*embSize+j)) * (*(emb+id*embSize+j));
		}

		if(bNorm) {
			norm = sqrt(norm);
			for(int j=0;j<embSize;j++) {
				*(emb+id*embSize+j) = *(emb+id*embSize+j)/norm;
				sum[j] += *(emb+id*embSize+j);
			}
		} else {
			for(int j=0;j<embSize;j++) {
				sum[j] += *(emb+id*embSize+j);
			}
		}

		count++;
	}

	if(count==0)
		count = 1;
	assert(unknownID != -1);
	for(int j=0;j<embSize;j++) {
		*(emb+unknownID*embSize+j) = sum[j]/count;
	}

}

template<typename T>
void initArray1(T *array, int size, T value) {
	for(int i=0;i<size;i++) {
		*(array+i) = value;
	}
}

template<typename T>
void initArray2(T *array, int sizeX, int sizeY, T value) {
	for(int i=0;i<sizeX;i++) {
		for(int j=0;j<sizeY;j++) {
			*(array+i*sizeY+j) = value;
		}
	}
}

template<typename T>
void dumpArray1(T *array, int size) {
	for(int i=0;i<size;i++) {
		cout<<*(array+i)<<" ";
	}
	cout<<endl;
}

template<typename T>
void dumpArray2(T *array, int sizeX, int sizeY) {
	for(int i=0;i<sizeX;i++) {
		for(int j=0;j<sizeY;j++) {
			cout<<*(array+i*sizeY+j)<<" ";
		}
		cout<<endl;
	}
}

template<typename T>
void releaseVector(vector<T *>& vector) {
	for(int i=0;i<vector.size();i++) {
		delete vector[i];
	}
}




} // end fox



#endif
