/*
 * Punctuation.h
 *
 *  Created on: Mar 1, 2016
 *      Author: fox
 */

#ifndef PUNCTUATION_H_
#define PUNCTUATION_H_
#include <set>

using namespace std;

namespace fox {

class Punctuation {
public:
	Punctuation() {

	}

	static set<char> english;

	static bool isEnglishPunc(char ch) {
		if(english.find(ch) != english.end())
			return true;
		else
			return false;
	}
private:
	static int a;
	static int _a() {
		english.insert('`');english.insert('~');english.insert('!');english.insert('@');english.insert('#');english.insert('$');
		english.insert('%');english.insert('&');english.insert('*');english.insert('(');english.insert(')');english.insert('-');
		english.insert('_');english.insert('+');english.insert('=');english.insert('{');english.insert('}');english.insert('|');
		english.insert('[');english.insert(']');english.insert('\\');english.insert(':');english.insert(';');english.insert('\'');
		english.insert('"');english.insert('<');english.insert('>');english.insert(',');english.insert('.');english.insert('?');
		english.insert('/');
		return 1;
	}
};

set<char> Punctuation::english;
int Punctuation::a = Punctuation::_a();


} /* namespace fox */

#endif /* PUNCTUATION_H_ */
