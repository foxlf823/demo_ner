/*
 * Sent.h
 *
 *  Created on: Dec 28, 2015
 *      Author: fox
 */

#ifndef SENT_H_
#define SENT_H_

#include "Token.h"

namespace fox {

class Sent {
public:
	vector<Token> tokens;
	int begin;
	int end;

	Sent() {

	}

/*	virtual ~Sent() {

	}*/
};

}



#endif /* SENT_H_ */
