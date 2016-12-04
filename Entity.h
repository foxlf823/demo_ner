
#ifndef ENTITY_H_
#define ENTITY_H_

#include <string>
#include <sstream>

using namespace std;

class Entity {
public:
	  string id; // id is unique in a document
	  string type;
	  int begin; // the first character offset at "doc level"
	  string text;
	  int end; // the last character offset+1 at "doc level"

	  int tkStart; // the token index that this segment starts
	  int tkEnd; // the token index that this segment ends.

	  int sentIdx; // the sentence index which this entity belongs to

	Entity() {
		id = "-1";
		type = "";
		begin = -1;
		text = "";
		end = -1;
		tkStart = -1;
		tkEnd = -1;


		sentIdx = -1;

	}

	bool equals(const Entity& another) const {
		if(type == another.type && begin == another.begin && end == another.end)
			return true;
		else
			return false;
	}


	bool equalsBoundary(const Entity& another) const {
		if(begin == another.begin && end == another.end)
			return true;
		else
			return false;
	}

	bool equalsType(const Entity& another) const {
		if(type == another.type)
			return true;
		else
			return false;
	}
};



#endif /* ENTITY_H_ */
