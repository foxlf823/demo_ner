# demo_ner
This is a demo Bi-LSTM model for named entity recognition (NER) and it is only used for study. 
Our purpose is to help ones to understand how a neural NER system works, so we do not guarantee its performance.

We utilize LibN3L to implement our model. Please cite:<br>
https://github.com/SUTDNLP/LibN3L<br>
Zhang, M., Yang, J., Teng, Z., Zhang, Y.: Libn3l: A lightweight package for neural nlp. 
In Proceedings of the Tenth International Conference on Language Resources and Evaluation (2016)

Preparation environment:<br>
1. Ubuntu 14<br>
2. run "sudo apt-get install" with "build-essential" "libboost-dev" "libopenblas-dev".

Usage:<br>
1. download this package and decompress it.<br>
2. cd into its directory, and run "make".<br>
3. run "chmod 777 ner" and "chmod 777 command.txt".<br>
4. run "./command.txt".

You can also import this package into eclipse, so you need to:<br>
1. install a C++ version of eclipse<br>
2. jre<br>
Please note that you should select "Existing Code as Makefile Project" when you are importing this package into eclipse!!!

Hope it helpful :)
