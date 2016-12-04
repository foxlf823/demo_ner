
#ifndef Classifier_H_
#define Classifier_H_

#include <iostream>

#include <assert.h>
#include "N3L.h"
#include "Example.h"

using namespace nr;
using namespace std;
using namespace mshadow;
using namespace mshadow::expr;
using namespace mshadow::utils;


template<typename xpu>
class Classifier {


public:
  LookupTable<xpu> _words;


  int _wordDim;
  int _otherDim;


  int _nerlabelSize;
  int _nerrnn_inputsize;
  int _nerrnn_outputsize;
  int _ner_hidden_inputsize;
  int _ner_hidden_outputsize;
  LSTM<xpu> _nerrnn_left;
  LSTM<xpu> _nerrnn_right;
  UniLayer<xpu> _ner_hidden_layer;
  UniLayer<xpu> _ner_olayer_linear;


  Metric _eval;

  Options options;


public:

  inline void init(Options options) {

	  this->options = options;

	  _wordDim = options.wordEmbSize;
	  _otherDim = options.otherEmbSize;

	  _nerrnn_inputsize = _wordDim; // word
	  _nerrnn_outputsize = options.rnnHiddenSize;
	  _ner_hidden_inputsize = _nerrnn_outputsize*2 ; // birnn
	  _ner_hidden_outputsize = options.hiddenSize;
	  _nerlabelSize = MAX_ENTITY;


	  _nerrnn_left.initial(_nerrnn_outputsize, _nerrnn_inputsize, true, 10);
	  _nerrnn_right.initial(_nerrnn_outputsize, _nerrnn_inputsize, false, 20);
	  _ner_hidden_layer.initial(_ner_hidden_outputsize, _ner_hidden_inputsize, true, 30, 0);
	  _ner_olayer_linear.initial(_nerlabelSize, _ner_hidden_outputsize, true, 40, 2);


	  cout<<"Classifier initial"<<endl;
  }

  inline void release() {
    _words.release();

    _nerrnn_left.release();
    _nerrnn_left.release();
    _ner_hidden_layer.release();
    _ner_olayer_linear.release();

  }


  inline dtype processNer(const vector<Example>& examples, int iter) {
    _eval.reset();
    int example_num = examples.size();
    dtype cost = 0.0;
    for (int count = 0; count < example_num; count++) {
    	const Example& example = examples[count];

      int seq_size = example._words.size();

      Tensor<xpu, 3, dtype> wordprime, wordprimeLoss, wordprimeMask;

      Tensor<xpu, 3, dtype> nerrnn_hidden_left, nerrnn_hidden_leftLoss;
      Tensor<xpu, 3, dtype> nerrnn_hidden_left_iy, nerrnn_hidden_left_oy, nerrnn_hidden_left_fy,
	  	  nerrnn_hidden_left_mcy, nerrnn_hidden_left_cy, nerrnn_hidden_left_my;
      Tensor<xpu, 3, dtype> nerrnn_hidden_right, nerrnn_hidden_rightLoss;
      Tensor<xpu, 3, dtype> nerrnn_hidden_right_iy, nerrnn_hidden_right_oy, nerrnn_hidden_right_fy,
	  	  nerrnn_hidden_right_mcy, nerrnn_hidden_right_cy, nerrnn_hidden_right_my;

      Tensor<xpu, 2, dtype> hidden_input, hidden_inputLoss;

      Tensor<xpu, 2, dtype> hidden_output, hidden_outputLoss;

      Tensor<xpu, 2, dtype> output, outputLoss;


      //initialize
      wordprime = NewTensor<xpu>(Shape3(seq_size, 1, _wordDim), 0.0);
      wordprimeLoss = NewTensor<xpu>(Shape3(seq_size, 1, _wordDim), 0.0);
      wordprimeMask = NewTensor<xpu>(Shape3(seq_size, 1, _wordDim), 1.0);

      nerrnn_hidden_left_iy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left_oy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left_fy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left_mcy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left_cy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left_my = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_left = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_leftLoss = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);

      nerrnn_hidden_right_iy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right_oy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right_fy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right_mcy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right_cy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right_my = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_right = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
      nerrnn_hidden_rightLoss = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);

      hidden_input = NewTensor<xpu>(Shape2(1, _ner_hidden_inputsize), 0.0);
      hidden_inputLoss = NewTensor<xpu>(Shape2(1, _ner_hidden_inputsize), 0.0);

      hidden_output = NewTensor<xpu>(Shape2(1, _ner_hidden_outputsize), 0.0);
      hidden_outputLoss = NewTensor<xpu>(Shape2(1, _ner_hidden_outputsize), 0.0);

      output = NewTensor<xpu>(Shape2(1, _nerlabelSize), 0.0);
      outputLoss = NewTensor<xpu>(Shape2(1, _nerlabelSize), 0.0);

      // forward
      for (int idx = 0; idx < seq_size; idx++) {
			if(options.dropProb != 0) {
				srand(iter * example_num + count * seq_size + idx);
			}

			_words.GetEmb(example._words[idx], wordprime[idx]);
			if(options.dropProb != 0) {
				dropoutcol(wordprimeMask[idx], options.dropProb);
				wordprime[idx] = wordprime[idx] * wordprimeMask[idx];
			}

      }

      _nerrnn_left.ComputeForwardScore(wordprime, nerrnn_hidden_left_iy, nerrnn_hidden_left_oy, nerrnn_hidden_left_fy,
    		  nerrnn_hidden_left_mcy, nerrnn_hidden_left_cy, nerrnn_hidden_left_my, nerrnn_hidden_left);
      _nerrnn_right.ComputeForwardScore(wordprime, nerrnn_hidden_right_iy, nerrnn_hidden_right_oy, nerrnn_hidden_right_fy,
          		  nerrnn_hidden_right_mcy, nerrnn_hidden_right_cy, nerrnn_hidden_right_my, nerrnn_hidden_right);


      concat(nerrnn_hidden_left[example._current_idx], nerrnn_hidden_right[example._current_idx], hidden_input);

      _ner_hidden_layer.ComputeForwardScore(hidden_input, hidden_output);

      _ner_olayer_linear.ComputeForwardScore(hidden_output, output);


      // get delta for each output
      cost += softmax_loss(output, example._nerLabels, outputLoss, _eval, example_num);

      // loss backward propagation
      _ner_olayer_linear.ComputeBackwardLoss(hidden_output, output, outputLoss, hidden_outputLoss);

      _ner_hidden_layer.ComputeBackwardLoss(hidden_input, hidden_output, hidden_outputLoss, hidden_inputLoss);

      unconcat(nerrnn_hidden_leftLoss[example._current_idx], nerrnn_hidden_rightLoss[example._current_idx], hidden_inputLoss);

      _nerrnn_left.ComputeBackwardLoss(wordprime, nerrnn_hidden_left_iy, nerrnn_hidden_left_oy, nerrnn_hidden_left_fy,
    		  nerrnn_hidden_left_mcy, nerrnn_hidden_left_cy, nerrnn_hidden_left_my, nerrnn_hidden_left,
			  nerrnn_hidden_leftLoss, wordprimeLoss);
      _nerrnn_right.ComputeBackwardLoss(wordprime, nerrnn_hidden_right_iy, nerrnn_hidden_right_oy, nerrnn_hidden_right_fy,
          		  nerrnn_hidden_right_mcy, nerrnn_hidden_right_cy, nerrnn_hidden_right_my, nerrnn_hidden_right,
				  nerrnn_hidden_rightLoss, wordprimeLoss);


        for (int idx = 0; idx < seq_size; idx++) {

        	if(options.dropProb != 0)
        		wordprimeLoss[idx] = wordprimeLoss[idx] * wordprimeMask[idx];
        	_words.EmbLoss(example._words[idx], wordprimeLoss[idx]);

        }


      //release
      FreeSpace(&wordprime);
      FreeSpace(&wordprimeLoss);
      FreeSpace(&wordprimeMask);


      FreeSpace(&nerrnn_hidden_left_iy);
      FreeSpace(&nerrnn_hidden_left_oy);
      FreeSpace(&nerrnn_hidden_left_fy);
      FreeSpace(&nerrnn_hidden_left_mcy);
      FreeSpace(&nerrnn_hidden_left_cy);
      FreeSpace(&nerrnn_hidden_left_my);
      FreeSpace(&nerrnn_hidden_left);
      FreeSpace(&nerrnn_hidden_leftLoss);

      FreeSpace(&nerrnn_hidden_right_iy);
      FreeSpace(&nerrnn_hidden_right_oy);
      FreeSpace(&nerrnn_hidden_right_fy);
      FreeSpace(&nerrnn_hidden_right_mcy);
      FreeSpace(&nerrnn_hidden_right_cy);
      FreeSpace(&nerrnn_hidden_right_my);
      FreeSpace(&nerrnn_hidden_right);
      FreeSpace(&nerrnn_hidden_rightLoss);

      FreeSpace(&hidden_input);
      FreeSpace(&hidden_inputLoss);

      FreeSpace(&hidden_output);
      FreeSpace(&hidden_outputLoss);

      FreeSpace(&output);
      FreeSpace(&outputLoss);

    }

    return cost;
  }

  int predictNer(const Example& example, vector<dtype>& results) {

		int seq_size = example._words.size();

		Tensor<xpu, 3, dtype> wordprime;

		Tensor<xpu, 3, dtype> nerrnn_hidden_left;
		Tensor<xpu, 3, dtype> nerrnn_hidden_left_iy, nerrnn_hidden_left_oy, nerrnn_hidden_left_fy,
		  nerrnn_hidden_left_mcy, nerrnn_hidden_left_cy, nerrnn_hidden_left_my;
		Tensor<xpu, 3, dtype> nerrnn_hidden_right;
		Tensor<xpu, 3, dtype> nerrnn_hidden_right_iy, nerrnn_hidden_right_oy, nerrnn_hidden_right_fy,
		  nerrnn_hidden_right_mcy, nerrnn_hidden_right_cy, nerrnn_hidden_right_my;

		Tensor<xpu, 2, dtype> hidden_input;

		Tensor<xpu, 2, dtype> hidden_output;

		Tensor<xpu, 2, dtype> output;


	      //initialize
	      wordprime = NewTensor<xpu>(Shape3(seq_size, 1, _wordDim), 0.0);

	      nerrnn_hidden_left_iy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left_oy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left_fy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left_mcy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left_cy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left_my = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_left = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);

	      nerrnn_hidden_right_iy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right_oy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right_fy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right_mcy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right_cy = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right_my = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);
	      nerrnn_hidden_right = NewTensor<xpu>(Shape3(seq_size, 1, _nerrnn_outputsize), 0.0);

	      hidden_input = NewTensor<xpu>(Shape2(1, _ner_hidden_inputsize), 0.0);

	      hidden_output = NewTensor<xpu>(Shape2(1, _ner_hidden_outputsize), 0.0);

	      output = NewTensor<xpu>(Shape2(1, _nerlabelSize), 0.0);


	      // forward
	      for (int idx = 0; idx < seq_size; idx++) {

				_words.GetEmb(example._words[idx], wordprime[idx]);

	      }

	      _nerrnn_left.ComputeForwardScore(wordprime, nerrnn_hidden_left_iy, nerrnn_hidden_left_oy, nerrnn_hidden_left_fy,
	    		  nerrnn_hidden_left_mcy, nerrnn_hidden_left_cy, nerrnn_hidden_left_my, nerrnn_hidden_left);
	      _nerrnn_right.ComputeForwardScore(wordprime, nerrnn_hidden_right_iy, nerrnn_hidden_right_oy, nerrnn_hidden_right_fy,
	          		  nerrnn_hidden_right_mcy, nerrnn_hidden_right_cy, nerrnn_hidden_right_my, nerrnn_hidden_right);

	      concat(nerrnn_hidden_left[example._current_idx], nerrnn_hidden_right[example._current_idx], hidden_input);

	      _ner_hidden_layer.ComputeForwardScore(hidden_input, hidden_output);

	      _ner_olayer_linear.ComputeForwardScore(hidden_output, output);

	      int optLabel = -1;
	      softmax_predict(output, optLabel);

	      //release
	      FreeSpace(&wordprime);

	      FreeSpace(&nerrnn_hidden_left_iy);
	      FreeSpace(&nerrnn_hidden_left_oy);
	      FreeSpace(&nerrnn_hidden_left_fy);
	      FreeSpace(&nerrnn_hidden_left_mcy);
	      FreeSpace(&nerrnn_hidden_left_cy);
	      FreeSpace(&nerrnn_hidden_left_my);
	      FreeSpace(&nerrnn_hidden_left);

	      FreeSpace(&nerrnn_hidden_right_iy);
	      FreeSpace(&nerrnn_hidden_right_oy);
	      FreeSpace(&nerrnn_hidden_right_fy);
	      FreeSpace(&nerrnn_hidden_right_mcy);
	      FreeSpace(&nerrnn_hidden_right_cy);
	      FreeSpace(&nerrnn_hidden_right_my);
	      FreeSpace(&nerrnn_hidden_right);

	      FreeSpace(&hidden_input);

	      FreeSpace(&hidden_output);

	      FreeSpace(&output);

		return optLabel;

}


  void updateParams(dtype nnRegular, dtype adaAlpha, dtype adaEps) {
	  _ner_olayer_linear.updateAdaGrad(nnRegular, adaAlpha, adaEps);
    _ner_hidden_layer.updateAdaGrad(nnRegular, adaAlpha, adaEps);

    _nerrnn_left.updateAdaGrad(nnRegular, adaAlpha, adaEps);
    _nerrnn_right.updateAdaGrad(nnRegular, adaAlpha, adaEps);


    _words.updateAdaGrad(nnRegular, adaAlpha, adaEps);

  }



};

#endif /* SRC_PoolGRNNClassifier_H_ */
