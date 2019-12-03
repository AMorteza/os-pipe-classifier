EnsembleClassifier.out:
	g++ -std=c++11 EnsembleClassifier.cpp -o EnsembleClassifier.out
	
clean:
	rm -f *.out
	rm -f pipe_*
