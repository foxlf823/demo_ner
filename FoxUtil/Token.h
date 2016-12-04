/*
 * Token.h
 *
 *  Created on: Dec 20, 2015
 *      Author: fox
 */

#ifndef TOKEN_H_
#define TOKEN_H_
#include <string>

using namespace std;

namespace fox {

class Token {
public:

	string word;
	string pos;
	int begin;
	int end; // begin+length
	string sst;
	string lemma;
	int depGov;
	string depType;
	int sentIdx;

	Token() {
		sentIdx = -1;
	}
/*	virtual ~Token() {

	}*/
};

}

#endif /* TOKEN_H_ */
