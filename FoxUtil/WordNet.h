#ifndef WORDNET_H_
#define WORDNET_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wn.h"
#include <sstream>

namespace fox {

// keep consistent with wn.h
#define WNNOUN		1
#define WNVERB		2
#define WNADJ		3
#define WNADV		4

/*
 * if level=0, return synset offset of synonym
 * if level>0, return synset offset of hypernym
 * if cannot find, return empty string
 */
string getWnID(char* lemmalower, int pos,int level=0) {


	int searchtype = 0;
	if(pos == NOUN || pos == VERB) {
		searchtype = -HYPERPTR;
	} else if(pos == ADJ)
		searchtype = SIMPTR;
	else
		searchtype = SYNS;

	stringstream ss;
	SynsetPtr ptr = findtheinfo_ds(lemmalower, pos, searchtype, 1);
	SynsetPtr head = ptr;
	do {
		if(ptr!=NULL) {
			ss.clear();
			ss.str("");
			ss<<ptr->hereiam;

			ptr = ptr->ptrlist;
			level--;
		}
	} while(ptr!=NULL && level>=0);

	if(head!=NULL)
		free_syns(head);
	return ss.str();
}

}

#endif
