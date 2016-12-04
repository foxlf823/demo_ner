/*
 * Tokenizer.h
 *
 *  Created on: Dec 21, 2015
 *      Author: fox
 */

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <set>
#include <fstream>
#include "Token.h"
#include <sstream>

using namespace std;

namespace fox {

class Tokenizer {
public:
	Tokenizer(const string* otherSplitters=NULL) {
		splitters.insert(' '); // default splitter
		if(NULL!=otherSplitters  && !(*otherSplitters).empty()) {
			ifstream ifs;
			ifs.open(otherSplitters->c_str());
			string line;
			while(getline(ifs, line)) {
				if(!line.empty()) {
					splitters.insert(line.at(0));
				}
			}

			ifs.close();
		}

	}
/*	virtual ~Tokenizer() {

	}*/

	set<char> splitters;

	void tokenize(int offset, const string& s, vector<Token>& tokens) {
		const char* chs = s.c_str();
		string sb;

		for(int i=0;i<s.length();) {
			char ch = *(chs+i);

			if(splitters.find(ch) != splitters.end()) {
				if(sb.length()!=0) {
					/*Token* pToken = new Token();
					pToken->word = sb;
					pToken->begin = offset-sb.length();
					pToken->end = offset;
					tokens.push_back(pToken);*/
					Token token;
					token.word = sb;
					token.begin = offset-sb.length();
					token.end = offset;
					tokens.push_back(token);
					sb.clear();
				}
				if(ch != ' ') {
					/*Token* pToken = new Token();
					pToken->word = ch;
					pToken->begin = offset;
					pToken->end = offset+1;
					tokens.push_back(pToken);*/
					Token token;
					token.word = ch;
					token.begin = offset;
					token.end = offset+1;
					tokens.push_back(token);
				}
			} else {
				sb += ch;
			}

			offset++;
			i++;
		}

		if(sb.length()!=0) {
			/*Token* pToken = new Token();
			pToken->word = sb;
			pToken->begin = offset-sb.length();
			pToken->end = offset;
			tokens.push_back(pToken);*/
			Token token;
			token.word = sb;
			token.begin = offset-sb.length();
			token.end = offset;
			tokens.push_back(token);
			sb.clear();
		}

	}

};




}



#endif /* TOKENIZER_H_ */
