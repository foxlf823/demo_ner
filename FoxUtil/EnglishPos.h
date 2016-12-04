/*
 * EnglishPos.h
 *
 *  Created on: Feb 29, 2016
 *      Author: fox
 */

#ifndef ENGLISHPOS_H_
#define ENGLISHPOS_H_

#include <string>

using namespace std;

namespace fox {

enum EnglishPosType {
	FOX_NOUN = 1, FOX_VERB, FOX_ADJ, FOX_ADV, FOX_PREP, FOX_CD, FOX_OTHER
};

class EnglishPos {
public:
	EnglishPos() {

	}
/*	virtual ~EnglishPos() {

	}*/



	static EnglishPosType getType(const string& pos) {

		if(pos=="NNP" || pos=="NNS" || pos=="NN" || pos=="NNPS")
			return FOX_NOUN;
		else if(pos=="VB" || pos=="VBZ" || pos=="VBG" || pos=="VBD" || pos=="VBN" || pos=="VBP")
			return FOX_VERB;
		else if(pos=="JJ" || pos=="JJS"  || pos=="JJR")
			return FOX_ADJ;
		else if(pos=="RB" || pos=="RBR"  || pos=="RBS")
			return FOX_ADV;
		else if(pos=="IN" || pos=="TO")
			return FOX_PREP;
		else if(pos=="CD")
			return FOX_CD; // ordinal number are tagged as JJ
		else
			return FOX_OTHER;


	}
};

} /* namespace fox */

#endif /* ENGLISHPOS_H_ */
