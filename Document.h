/*
 * BiocDocument.h
 *
 *  Created on: Dec 19, 2015
 *      Author: fox
 */

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <string>
#include "Entity.h"
#include "Sent.h"

using namespace std;


class Document {
public:
	Document() {

	}

	string id;
	vector<Entity> entities;
	vector<fox::Sent> sents;
};

#endif /* DOCUMENT_H_ */
