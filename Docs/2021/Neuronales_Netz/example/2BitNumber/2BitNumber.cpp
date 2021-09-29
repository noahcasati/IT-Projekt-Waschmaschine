/*
Simple neuronal network to detect a 2 Bit binary number.

Based on
David Miller, http://millermattson.com/dave

2018	Friedrich Haase		modified
2021	F.Haase				updated for IT-project
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cmath>
#include "neuron1D.h"
using namespace std;

//! number of samples to train the network
const int				nTrainingSamples = 2000;

//#define VERBOSE
#define SAVE_ERROR

#if defined (SAVE_ERROR)
ofstream				ErrorFile("errors.txt");
#endif

//! vote for 0/false or 1/true depending on 20% ranges
double Vote(double x)
{
	return x > .8 ? 1 : x < .2 ? 0 : x;
}

//! Output to stdout of a label and values in a vector as a single line
void ShowVectorValues(string label, vector<double>& v)
{
	cout << label << " ";
	for ( size_t i = 0; i < v.size(); ++i )
	{
		cout << v[i] << " ";
	}
	cout << endl;
}

// Output to stdout of a label and voted values in a vector as a single line
void ShowVectorValuesVoted(string label, vector<double>& v)
{
	cout << label << " ";
	for ( size_t i = 0; i < v.size(); ++i )
	{
		cout << Vote(v[i]) << " ";
	}
	cout << endl;
}

int main()
{
	cout << "Neuronal network to detect a 2 Bit binary number" << endl;

	// Create a neural network
	// topology
	// 2 Neurons in the input layer + BIAS
	// 4 Neurons in the hidden layer + BIAS
	// 4 Neurons in the output layer
	vector<size_t>		topology = {2, 8, 4};
	NeuralNetwork		ANN;
	ANN.CreateLayers1D(topology, true/*BIAS*/);
	ANN.ConnectFullyConnected();

	// create training data and pass to the neural network
	int					nInput;
	vector<double>		inputVals;
	vector<double>		targetVals, resultVals;
	int					trainingPass = 0;
	cout << "training with " << nTrainingSamples << " test samples" << endl;

	while ( ++trainingPass <= nTrainingSamples )
	{
		// calculate new input data sample and expected result
		nInput = rand() & 3;							// 2 bits as input
		inputVals.push_back((nInput & 2) != 0);			// fill input and target vectors with doubles, 0.0 or 1.0
		inputVals.push_back((nInput & 1) != 0);
		targetVals.push_back(nInput == 0);
		targetVals.push_back(nInput == 1);
		targetVals.push_back(nInput == 2);
		targetVals.push_back(nInput == 3);

		assert(inputVals.size() == topology.front());	// vector size check
		assert(targetVals.size() == topology.back());

#if defined VERBOSE
		cout << trainingPass;
		ShowVectorValues(": Inputs:", inputVals);
#endif

		// let the neural network do it's normal job
		ANN.FeedForward(inputVals);						// feed forward

#if defined VERBOSE
		ANN.GetResults(resultVals);						// get and show result from the neural network
		ShowVectorValues("results:", resultVals);
		ShowVectorValues("Targets:", targetVals);		// show correct target values
#endif

		// train the net what the outputs should have been
		ANN.BackPropagation(targetVals);				// back propagation

#if defined SAVE_ERROR
		ANN.CalcError(targetVals);
//		ErrorFile << ANN.GetRecentAverageError() << endl;
		ErrorFile << ANN.GetError() << endl;
#endif

		inputVals.clear();								// cleanup for next training sample
		targetVals.clear();
		resultVals.clear();
	}

	cout << "training done with " << nTrainingSamples << " samples" << endl << endl;

	cout << "manual input new test value (0..3),\nempty input to terminate" << endl;
	string			line;
	unsigned int	oneValue;
	while ( true )
	{
		getline(cin, line);								// read a line
		stringstream ss(line);							// use it as C++ stream
		inputVals.clear();
		if ( ss >> oneValue )
		{
			if ( oneValue <= 3 )
			{											// fill input and target vectors with doubles, 0.0 or 1.0
				inputVals.push_back(( oneValue & 2 ) != 0);
				inputVals.push_back(( oneValue & 1 ) != 0);
			}
			else
				inputVals.push_back(oneValue);			// dummy for else below
		}

		if ( inputVals.size() == topology.front() )		// check number of input values
		{
			ANN.FeedForward(inputVals);					// feed forward
			ANN.GetResults(resultVals);
			cout << inputVals[0] << " " << inputVals[1];
			ShowVectorValuesVoted(": ", resultVals);	// show result
//			ShowVectorValues(": ", resultVals);			// show result
		}
		else if ( inputVals.size() != 0 )
			cout << "need 1 input value 0..3" << endl;
		else
			break;
	}

	return 0;
}
