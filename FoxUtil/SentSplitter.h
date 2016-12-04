/*
 * SentSplitter.h
 *
 *  Created on: Dec 20, 2015
 *      Author: fox
 */

#ifndef SENTSPLITTER_H_
#define SENTSPLITTER_H_

#include <set>
#include <string>
#include <fstream>
#include "FoxUtil.h"
#include "Punctuation.h"

using namespace std;

namespace fox {

class SentSplitter {
public:
	set<char> delimiters;
	set<string> abbr;

	SentSplitter(const set<char>* otherDelimiters=NULL, const string* abbrFilePath=NULL) {
		delimiters.insert('.');
		delimiters.insert(';');
		delimiters.insert('!');
		delimiters.insert('?');
		if(NULL != otherDelimiters) {
			set<char>::iterator iter;
			for(iter=otherDelimiters->begin(); iter!=otherDelimiters->end();iter++) {
				delimiters.insert(*iter);
			}
		}

		if(NULL != abbrFilePath && !(*abbrFilePath).empty()) {
			ifstream ifs;
			ifs.open(abbrFilePath->c_str());
			string line;
			while(getline(ifs, line)) {
				if(!line.empty()) {
					abbr.insert(line);
				}
			}

			ifs.close();
		}
	}
/*	virtual ~SentSplitter() {

	}*/

	void splitWithFilters(const string& s, vector<string>& sentences) {
		int sBeginIndex = s.length()-1;
		int sEndIndex = s.length();

		for(;sBeginIndex>=0;sBeginIndex--) {
			char c = s.at(sBeginIndex);

			if(delimiters.find(c) != delimiters.end()) {
				if(c == '.') {
					bool split = true;
					int step = 0;
					int matchedAbbrLength = 0;
					if(filterFullStopWithNumbers(sBeginIndex, s)) {
						split = false;
						step = 2;
					} else if(filterOneUpperWord(sBeginIndex, s)) {
						split = false;
						step = 1;
					}
					else if(0 != (matchedAbbrLength= filterAbbr(sBeginIndex, s))) {
						split = false;
						step = matchedAbbrLength;
					}

					if(split) {
						if(sBeginIndex+1 < sEndIndex) {
							//sentences.insert(sentences.begin(), new string(s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1))));
							sentences.insert(sentences.begin(), s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1)));
						}
						sEndIndex = sBeginIndex+1;
					} else {
						sBeginIndex = sBeginIndex-step+1;
					}
				} else {
					if(sBeginIndex+1<sEndIndex) {
						//sentences.insert(sentences.begin(), new string(s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1))));
						sentences.insert(sentences.begin(), s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1)));
					}
					sEndIndex = sBeginIndex+1;
				}
			}
		}
		//sentences.insert(sentences.begin(), new string(s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1))));
		sentences.insert(sentences.begin(), s.substr(sBeginIndex+1, sEndIndex-(sBeginIndex+1)));
		return;
	}

private:
	 bool filterFullStopWithNumbers(uint currentIndex, const string& s) {
		if(0 == currentIndex || s.length()-1==currentIndex)
			return false;
		char previous = s.at(currentIndex-1);
		char next = s.at(currentIndex+1);
		if(isNumber(previous) && isNumber(next))
			return true;
		else
			return false;
	 }

	 bool filterOneUpperWord(uint currentIndex, const string& s) {
		 if(currentIndex >=2) {
			 char previous = s.at(currentIndex-1);
			 char pp = s.at(currentIndex-2);
			 if(isupper(previous) && pp==' ')
				 return true;
			 else
				 return false;
		 } else if(currentIndex == 1){
			 char previous = s.at(currentIndex-1);
			 if(isupper(previous))
				 return true;
			 else
				 return false;
		 } else
			 return false;
	 }

	 uint filterAbbr(int currentIndex, const string& s) {
		 uint matchedAbbrLength = 0;
		 for(set<string>::iterator iter=abbr.begin(); iter!=abbr.end(); iter++) {
			 int tempIndex = currentIndex;
			 int j = iter->length()-1;
			 for(;j>=0;j--) {
				 if(tempIndex<0)
					 break;
				 //if(toLowercase(iter->at(j)) != toLowercase(s.at(tempIndex)))
				 if(iter->at(j) != s.at(tempIndex))
					 break;
				 tempIndex--;
			 }
			 if(j<0 && (tempIndex<0 || (tempIndex>=0 && (isEmptyChar(s.at(tempIndex)) || fox::Punctuation::isEnglishPunc(s.at(tempIndex)))))) {
				 matchedAbbrLength = iter->length();
				 break;
			 }
		 }
		 return matchedAbbrLength;
	 }

	 bool isEmptyChar(char pp) {
		 if(pp==' ' || pp=='\n' || pp=='\r' || pp=='\t')
			 return true;
		 else
			 return false;
	 }
};


}
#endif /* SENTSPLITTER_H_ */
