
#ifndef NNentity_H_
#define NNentity_H_

#include <iosfwd>
#include "Options.h"
#include "Tool.h"
#include "FoxUtil.h"
#include "N3Lhelper.h"
#include "Utf.h"
#include "Token.h"
#include "Sent.h"
#include <sstream>
#include "Dependency.h"
#include "N3L.h"
#include "Document.h"
#include "EnglishPos.h"
#include "Punctuation.h"
#include "Word2Vec.h"
#include "utils.h"
#include "Classifier.h"
#include "Example.h"



using namespace nr;
using namespace std;




class NNentity {
public:
	Options m_options;

	Alphabet m_wordAlphabet;


	string unknownkey;
	string nullkey;

	Classifier<cpu> m_classifier;



	NNentity(const Options &options):m_options(options) {
		unknownkey = "-#unknown#-";
		nullkey = "-#null#-";
	}


	void trainAndTest(const string& trainFile, const string& devFile, const string& testFile,
			Tool& tool,
			const string& trainNlpFile, const string& devNlpFile, const string& testNlpFile) {


		// load train data
		vector<Document> trainDocuments;
		loadAnnotatedFile(trainFile, trainDocuments);
		loadNlpFile(trainNlpFile, trainDocuments);

		vector<Document> devDocuments;
		if(!devFile.empty()) {
			loadAnnotatedFile(devFile, devDocuments);
			loadNlpFile(devNlpFile, devDocuments);
		}
		vector<Document> testDocuments;
		if(!testFile.empty()) {
			loadAnnotatedFile(testFile, testDocuments);
			loadNlpFile(testNlpFile, testDocuments);
		}


		cout << "Creating Alphabet..." << endl;

		m_wordAlphabet.clear();
		m_wordAlphabet.from_string(unknownkey);
		m_wordAlphabet.from_string(nullkey);

		createAlphabet(trainDocuments, tool);

		if (!m_options.wordEmbFineTune) {
			// if not fine tune, use all the data to build alphabet
			if(!devDocuments.empty())
				createAlphabet(devDocuments, tool);
			if(!testDocuments.empty())
				createAlphabet(testDocuments, tool);
		}


		NRMat<dtype> wordEmb;
		if(m_options.embFile.empty()) {
			cout<<"random emb"<<endl;

			randomInitNrmat(wordEmb, m_wordAlphabet, m_options.wordEmbSize, 1000);
		} else {
			cout<< "load pre-trained emb"<<endl;
			tool.w2v->loadFromBinFile(m_options.embFile, false, true);

			double* emb = new double[m_wordAlphabet.size()*m_options.wordEmbSize];
			fox::initArray2((double *)emb, (int)m_wordAlphabet.size(), m_options.wordEmbSize, 0.0);
			vector<string> known;
			map<string, int> IDs;
			alphabet2vectormap(m_wordAlphabet, known, IDs);

			tool.w2v->getEmbedding((double*)emb, m_options.wordEmbSize, known, unknownkey, IDs);

			wordEmb.resize(m_wordAlphabet.size(), m_options.wordEmbSize);
			array2NRMat((double*) emb, m_wordAlphabet.size(), m_options.wordEmbSize, wordEmb);

			delete[] emb;
		}


		vector<Example> trainExamples;
		initialTrainingExamples(tool, trainDocuments, trainExamples);
		cout<<"Total train example number: "<<trainExamples.size()<<endl;

		m_classifier.init(m_options);

		m_classifier._words.initial(wordEmb);
		m_classifier._words.setEmbFineTune(m_options.wordEmbFineTune);


		static Metric metric_dev;
		static vector<Example> subExamples;

		dtype best = 0;

		// begin to train
		for (int iter = 0; iter < m_options.maxIter; ++iter) {

			cout << "##### Iteration " << iter << std::endl;

		    int exampleIdx = 0;
			for(int docIdx=0;docIdx<trainDocuments.size();docIdx++) {
				const Document& doc = trainDocuments[docIdx];


				for(int sentIdx=0;sentIdx<doc.sents.size();sentIdx++) {
					const fox::Sent & sent = doc.sents[sentIdx];

					for(int tokenIdx=0;tokenIdx<sent.tokens.size();tokenIdx++) {
						const fox::Token& token = sent.tokens[tokenIdx];

						subExamples.clear();
						subExamples.push_back(trainExamples[exampleIdx]);
						int curUpdateIter = iter * trainExamples.size() + exampleIdx;

						dtype cost = m_classifier.processNer(subExamples, curUpdateIter);


						m_classifier.updateParams(m_options.regParameter, m_options.adaAlpha, m_options.adaEps);


						exampleIdx++;
					} // token


				} // sent


			} // doc

		    // an iteration end, begin to evaluate
		    if (devDocuments.size() > 0 && (iter+1)% m_options.evalPerIter ==0) {

		    	evaluate(tool, devDocuments, metric_dev, iter);

				if (metric_dev.getAccuracy() > best) {
					cout << "Exceeds best performance of " << best << endl;
					best = metric_dev.getAccuracy();

					evaluate(tool, testDocuments, metric_dev, iter);

				}



		    } // devExamples > 0

		} // for iter




		m_classifier.release();

	}


	void initialTrainingExamples(Tool& tool, const vector<Document>& documents, vector<Example>& examples) {

		for(int docIdx=0;docIdx<documents.size();docIdx++) {
			const Document& doc = documents[docIdx];


			for(int sentIdx=0;sentIdx<doc.sents.size();sentIdx++) {
				const fox::Sent & sent = doc.sents[sentIdx];
				vector<string> labelSequence;

				for(int tokenIdx=0;tokenIdx<sent.tokens.size();tokenIdx++) {
					const fox::Token& token = sent.tokens[tokenIdx];
					int entityIdx = -1;
					string schemaLabel = O;

					for(int i=0;i<doc.entities.size();i++) {
						const Entity& entity = doc.entities[i];

						string temp = getShemaLabel(token, entity);
						if(temp != O) {
							entityIdx = i;
							schemaLabel = temp;
							break;
						}
					}

					Example eg(false);
					string labelName = entityIdx!=-1 ? schemaLabel+"_"+doc.entities[entityIdx].type : O;
					generateOneNerExample(eg, labelName, sent, tokenIdx);
					labelSequence.push_back(labelName);

					examples.push_back(eg);

				} // token


			} // sent


		} // doc

	}

	void evaluate(Tool& tool, const vector<Document>& documents, Metric& metric_dev, int iter) {
    	metric_dev.reset();

    	int ctGoldEntity = 0;
    	int ctPredictEntity = 0;
    	int ctCorrectEntity = 0;

		for(int docIdx=0;docIdx<documents.size();docIdx++) {
			const Document& doc = documents[docIdx];
			vector<Entity> anwserEntities;


			for(int sentIdx=0;sentIdx<doc.sents.size();sentIdx++) {
				const fox::Sent & sent = doc.sents[sentIdx];
				vector<string> labelSequence;

				for(int tokenIdx=0;tokenIdx<sent.tokens.size();tokenIdx++) {
					const fox::Token& token = sent.tokens[tokenIdx];

					Example eg(false);
					string fakeLabelName = "";
					generateOneNerExample(eg, fakeLabelName, sent, tokenIdx);

					vector<dtype> probs;
					int predicted = m_classifier.predictNer(eg, probs);

					string labelName = NERlabelID2labelName(predicted);
					labelSequence.push_back(labelName);

					// decode entity label
					if(labelName == B_Location) {
						Entity entity;
						newEntity(token, labelName, entity, 0);
						anwserEntities.push_back(entity);
					} else if(labelName == I_Location) {
						if(checkWrongState(labelSequence)) {
							Entity& entity = anwserEntities[anwserEntities.size()-1];
							appendEntity(token, entity);
						}
					}


				} // token


			} // sent

			// evaluate by ourselves
			ctGoldEntity += doc.entities.size();
			ctPredictEntity += anwserEntities.size();
			for(int i=0;i<anwserEntities.size();i++) {
				int k=-1;
				int j=0;
				for(;j<doc.entities.size();j++) {
					if(anwserEntities[i].equalsBoundary(doc.entities[j])) {
						if(anwserEntities[i].equalsType(doc.entities[j])) {
							ctCorrectEntity ++;
							break;
						} else {
							k = j;
							break;
						}
					}
				}

			}


		} // doc


		metric_dev.overall_label_count = ctGoldEntity;
		metric_dev.predicated_label_count = ctPredictEntity;
		metric_dev.correct_label_count = ctCorrectEntity;
		metric_dev.print();

	}


	// Only used when current label is I or L, check state from back to front
	// in case that "O I I", etc.
	bool checkWrongState(const vector<string>& labelSequence) {
		int positionNew = -1; // the latest type-consistent B
		int positionOther = -1; // other label except type-consistent I

		const string& currentLabel = labelSequence[labelSequence.size()-1];

		for(int j=labelSequence.size()-2;j>=0;j--) {
			if(currentLabel==I_Location) {
				if(positionNew==-1 && labelSequence[j]==B_Location) {
					positionNew = j;
				} else if(positionOther==-1 && labelSequence[j]!=I_Location) {
					positionOther = j;
				}
			}

			if(positionOther!=-1 && positionNew!=-1)
				break;
		}

		if(positionNew == -1)
			return false;
		else if(positionOther<positionNew)
			return true;
		else
			return false;
	}

	void generateOneNerExample(Example& eg, const string& labelName, const fox::Sent& sent, const int tokenIdx) {
		if(!labelName.empty()) {
			int labelID = NERlabelName2labelID(labelName);
			for(int i=0;i<MAX_ENTITY;i++)
				eg._nerLabels.push_back(0);
			eg._nerLabels[labelID] = 1;

		}

		for(int i=0;i<sent.tokens.size();i++) {
			eg._words.push_back(featureName2ID(m_wordAlphabet, feature_word(sent.tokens[i])));

		}

		eg._current_idx = tokenIdx;
	}


	void createAlphabet (const vector<Document>& documents, Tool& tool) {

		hash_map<string, int> word_stat;


		for(int docIdx=0;docIdx<documents.size();docIdx++) {

			for(int i=0;i<documents[docIdx].sents.size();i++) {

				for(int j=0;j<documents[docIdx].sents[i].tokens.size();j++) {

					string curword = feature_word(documents[docIdx].sents[i].tokens[j]);
					word_stat[curword]++;


				}


			}


		}

		stat2Alphabet(word_stat, m_wordAlphabet, "word");

	}

	string feature_word(const fox::Token& token) {
		string ret = normalize_to_lowerwithdigit(token.word);

		return ret;
	}


	void randomInitNrmat(NRMat<dtype>& nrmat, Alphabet& alphabet, int embSize, int seed) {
		double* emb = new double[alphabet.size()*embSize];
		fox::initArray2((double *)emb, (int)alphabet.size(), embSize, 0.0);

		vector<string> known;
		map<string, int> IDs;
		alphabet2vectormap(alphabet, known, IDs);

		fox::randomInitEmb((double*)emb, embSize, known, unknownkey,
				IDs, false, m_options.initRange, seed);

		nrmat.resize(alphabet.size(), embSize);
		array2NRMat((double*) emb, alphabet.size(), embSize, nrmat);

		delete[] emb;
	}


	void stat2Alphabet(hash_map<string, int>& stat, Alphabet& alphabet, const string& label) {

		cout << label<<" num: " << stat.size() << endl;
		alphabet.set_fixed_flag(false);
		hash_map<string, int>::iterator feat_iter;
		for (feat_iter = stat.begin(); feat_iter != stat.end(); feat_iter++) {

			if (feat_iter->second > m_options.wordCutOff) {
			  alphabet.from_string(feat_iter->first);
			}
		}
		cout << "alphabet "<< label<<" num: " << alphabet.size() << endl;
		alphabet.set_fixed_flag(true);

	}

	int featureName2ID(Alphabet& alphabet, const string& featureName) {
		int id = alphabet.from_string(featureName);
		if(id >=0)
			return id;
		else
			return 0; // assume unknownID is zero
	}

};



#endif

