
#ifndef UTILS_H_
#define UTILS_H_


#include <stdio.h>
#include <vector>
#include "Word2Vec.h"
#include "Utf.h"
#include "Entity.h"
#include "Token.h"
#include "FoxUtil.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Document.h"
#include <list>
#include <sstream>

using namespace std;



// schema BIO, one entity types (Location)
#define MAX_ENTITY 3
#define B_Location "B_Location"
#define I_Location "I_Location"
#define O "O"

string getShemaLabel(const fox::Token& tok, const Entity& entity) {

	if(tok.begin==entity.begin)
		return "B";
	else if(tok.begin>=entity.begin && tok.end<=entity.end)
		return "I";
	else
		return "O";

}


void appendEntity(const fox::Token& token, Entity& entity) {
	int whitespacetoAdd = token.begin-entity.end;
	for(int j=0;j<whitespacetoAdd;j++)
		entity.text += " ";
	entity.text += token.word;
	entity.end = token.end;
}

void newEntity(const fox::Token& token, const string& labelName, Entity& entity, int entityId) {
	stringstream ss;
	ss<<"T"<<entityId;
	entity.id = ss.str();
	entity.type = labelName.substr(labelName.find("_")+1);
	entity.begin = token.begin;
	entity.end = token.end;
	entity.text = token.word;
}

int NERlabelName2labelID(const string& labelName) {
	if(labelName == B_Location) {
		return 0;
	} else if(labelName == I_Location) {
		return 1;
	} else
		return 2;
}

string NERlabelID2labelName(const int labelID) {
	if(labelID == 0) {
		return B_Location;
	} else if(labelID == 1) {
		return I_Location;
	} else
		return O;
}



void loadNlpFile(const string& dirPath, vector<Document>& docs) {

	struct dirent** namelist = NULL;
	int total = scandir(dirPath.c_str(), &namelist, 0, alphasort);
	int count = 0;

	for(int i=0;i<total;i++) {

		if (namelist[i]->d_type == 8) {
			//file
			if(namelist[i]->d_name[0]=='.')
				continue;

			string filePath = dirPath;
			filePath += "/";
			filePath += namelist[i]->d_name;
			string fileName = namelist[i]->d_name;

			ifstream ifs;
			ifs.open(filePath.c_str());
			fox::Sent sent;
			string line;


			while(getline(ifs, line)) {
				if(line.empty()){
					// new line
					if(sent.tokens.size()!=0) {
						docs[count].sents.push_back(sent);
						docs[count].sents[docs[count].sents.size()-1].begin = sent.tokens[0].begin;
						docs[count].sents[docs[count].sents.size()-1].end = sent.tokens[sent.tokens.size()-1].end;
						sent.tokens.clear();
					}
				} else {
					vector<string> splitted;
					fox::split_bychar(line, splitted, '\t');
					fox::Token token;
					token.word = splitted[0];
					token.begin = atoi(splitted[1].c_str());
					token.end = atoi(splitted[2].c_str());
					sent.tokens.push_back(token);
				}



			}

			ifs.close();
			count++;
		}
	}

}


int loadAnnotatedFile(const string& dirPath, vector<Document>& documents)
{
	struct dirent** namelist = NULL;
	int total = scandir(dirPath.c_str(), &namelist, 0, alphasort);


	for(int i=0;i<total;i++) {

		if (namelist[i]->d_type == 8) {
			//file
			if(namelist[i]->d_name[0]=='.')
				continue;

			string filePath = dirPath;
			filePath += "/";
			filePath += namelist[i]->d_name;
			string fileName = namelist[i]->d_name;


			ifstream ifs;
			ifs.open(filePath.c_str());

			Document doc;
			doc.id = fileName.substr(0, fileName.find("."));


			string line;
			while(getline(ifs, line)) {

				if(!line.empty()) {

					if(line[0] == 'T') { // entity
						vector<string> splitted;
						fox::split_bychar(line, splitted, '\t');
						Entity entity;
						entity.id = splitted[0];
						entity.text = splitted[2];

						vector<string> temp1;
						fox::split(splitted[1], temp1, " |;");

						entity.type = temp1[0];
						entity.begin = atoi(temp1[1].c_str());
						entity.end = atoi(temp1[2].c_str());

						doc.entities.push_back(entity);

					}

				}
			}
			ifs.close();

			documents.push_back(doc);

		}
	}


    return 0;

}








#endif /* UTILS_H_ */
