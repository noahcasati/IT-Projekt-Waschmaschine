/*

1 dimensional neural network
partially inspired by David R. Miller, 2015 version

*/

#include <iostream>
#include <cmath>
#include <cassert>
#include <string>
//#include "../helper.h"
#include "neuron1D.h"
using namespace std;

// Get a random number between 0.0 and 1.0
static double RandomNumber_0_1(void)
{
	return rand() / double(RAND_MAX);
}

// Get a random number between -1.0 and 1.0
static double RandomNumber_m1_1(void)
{
	return ( rand() / double(RAND_MAX) ) * 2.0 - 1.0;
}

// TANH transfer function
static double TransferFunctionTANH(double x)
{
	// tanh - output range [-1.0..1.0]
	return tanh(x);
}

// TANH transfer function derivative
static double TransferFunctionTANHDerivative(double x)
{
	// tanh derivative
	return 1.0 - x * x;
}

// SIGMOID transfer function
static double TransferFunctionSIGMOID(double x)
{
	return 1.0 / ( 1.0 + exp(-x) );
}

// SIGMOID transfer function derivative
static double TransferFunctionSIGMOIDDerivative(double x)
{
	return x * ( 1.0 - x );
}

// rectified linear(ReLu) transfer function
static double TransferFunctionRELU(double x)
{
	if ( x <= 0.0 )
		return 0.0;
	return x;
}

// rectified linear(ReLu) transfer function derivative
static double TransferFunctionRELUerivative(double x)
{
	if ( x <= 0.0 )
		return 0.0;
	return 1.0;
}

//-----------------------------------------------------------------------------
// class Connection

// constructor
Connection::Connection(Neuron* from, Neuron* to)
	: m_SrcNeuron(from), m_DstNeuron(to), m_Weight(0.0), m_DeltaWeight(0.0)
{
	m_Weight = NeuralNetwork::m_pRandomFunction();
}

//-----------------------------------------------------------------------------
// class Neuron

// constructor
Neuron::Neuron()
	: m_OutputValue(RandomNumber_0_1()-0.5), m_Gradient(0)
{
}

// Feed forward function for a neuron
void Neuron::FeedForward(Layer* pLayer)
{
//?? pLayer might be useful if all layers have their own selection of transfer functions
	// Sum the previous layers outputs. Possibly including a bias neuron.
	double		sum = 0.0;
	// Sum the outputs from all inputs to this neuron
	for ( auto pConn : m_SrcConnections )
	{
		sum += pConn->m_SrcNeuron->m_OutputValue * pConn->m_Weight;
	}
	m_OutputValue = NeuralNetwork::m_pTransferFunction(sum);
}

// The error gradient of an output-layer neuron is equal to the target
// (desired) value minus the computed output value, times the derivative of
// the output-layer activation function evaluated at the computed output value.
void Neuron::CalcOutputGradients(double targetVal)
{
	double	delta = targetVal - m_OutputValue;
	m_Gradient = delta * NeuralNetwork::m_pTransferFunctionDerivative(m_OutputValue);
}

// The error gradient of a hidden-layer neuron is equal to the derivative
// of the activation function of the hidden layer evaluated at the
// local output of the neuron times the sum of the product of
// the primary outputs times their associated hidden-to-output weights.
void Neuron::CalcHiddenGradients()
{
	// Sum our contributions of the errors at the nodes we feed.
	double	sum = 0.0;
	for ( auto pConn : m_DstConnections )
	{
		sum += pConn->m_Weight * pConn->m_DstNeuron->m_Gradient;
	}
	m_Gradient = sum * NeuralNetwork::m_pTransferFunctionDerivative(m_OutputValue);
}

// update weights
void Neuron::UpdateInputWeights(double eta, double alpha)
{
	// The weights to be updated are the weights from the neurons in the
	// preceding layer (the source layer) to this neuron:

	for ( auto pConn : m_SrcConnections )
	{
//?? improve, remove temporaries
		double			oldDeltaWeight = pConn->m_DeltaWeight;

		double			newDeltaWeight =
			// Individual input, magnified by the gradient and train rate:
			eta
			* pConn->m_SrcNeuron->m_OutputValue
			* m_Gradient
			// Add momentum = a fraction of the previous delta weight;
			+ alpha
			* oldDeltaWeight;
		pConn->m_DeltaWeight = newDeltaWeight;
		pConn->m_Weight += newDeltaWeight;

assert(pConn->m_Weight != 0);
	}
}

//! get weights as string
void Neuron::GetWeights(string& sWeights)
{
	for ( auto pConn : m_SrcConnections )
	{
		sWeights += to_string(pConn->m_Weight) + " ";
	}
}

//-----------------------------------------------------------------------------
// class Layer

// constructor
Layer::Layer(size_t nSize, bool bAddBias)
	: m_nSize(nSize), m_bUseBias(bAddBias)
{
	for ( size_t j=0; j<m_nSize; ++j )
	{
		m_Neurons.push_back(new Neuron);
	}
	if ( m_bUseBias )
	{
		m_Neurons.push_back(new Neuron);
		m_Neurons.back()->SetOutputValue(1.0);
	}
};

// Feed forward function for a layer
void Layer::FeedForward()
{
	for ( size_t j=0; j<m_Neurons.size(); ++j )
	{
		m_Neurons[j]->FeedForward(this);				// for all neurons in a layer
	}
}

// calculate gradients
void Layer::CalcOutputGradients(const vector<double>& targetValues)
{
	for ( size_t j=0; j<targetValues.size(); ++j )
	{
		m_Neurons[j]->CalcOutputGradients(targetValues[j]);	// for all neurons in output layer (single row)
	}
}

// calculate gradients
void Layer::CalcHiddenGradients()
{
	for ( size_t j=0; j<m_Neurons.size(); ++j )
	{
		m_Neurons[j]->CalcHiddenGradients();
	}
}

// update weights
void Layer::UpdateWeights(double eta, double alpha)
{
	for ( size_t j=0; j<m_Neurons.size(); ++j )
	{
		m_Neurons[j]->UpdateInputWeights(eta, alpha);
	}
}

//! get weights as string
void Layer::GetWeights(string& sWeights)
{
	for ( size_t j = 0; j < m_Neurons.size(); ++j )
	{
		m_Neurons[j]->GetWeights(sWeights);
	}
}

//-----------------------------------------------------------------------------
// class NeuralNetwork

/*static*/ vector<Connection*>		NeuralNetwork::m_ConnectionsAll;												// list of all connections between neurons
/*static*/ double					NeuralNetwork::m_eta = 0.15;													// [0.0..1.0] overall net training rate, only used in backPropagation()
/*static*/ double					NeuralNetwork::m_alpha = 0.5;													// [0.0..n] multiplier of last weight change (momentum), only used in backPropagation()
/*static*/ ANNFunction_t			NeuralNetwork::m_pRandomFunction = RandomNumber_0_1;							// selected random number function for initial weights
/*static*/ ANNTransferFunction_t	NeuralNetwork::m_pTransferFunction = TransferFunctionTANH;						// transfer function
/*static*/ ANNTransferFunction_t	NeuralNetwork::m_pTransferFunctionDerivative = TransferFunctionTANHDerivative;	// transfer function derivative

// constructor
NeuralNetwork::NeuralNetwork()
	: m_Error(0.0), m_RecentAverageError(0.0), m_RecentAverageSmoothingFactor(125.0)
{
}

// destructor
NeuralNetwork::~NeuralNetwork()
{
	for ( auto pConn : m_ConnectionsAll )				// clean-up connections
		delete pConn;
	for ( auto layer : m_Layers )						// clean-up neurons
	{
		for ( size_t j=0; j<layer.Size(); ++j )
		{
			delete layer.m_Neurons[j];
		}
	}
}

// Create layers
void NeuralNetwork::CreateLayers1D(vector<size_t>& vSize, bool bAddBias)
{
	assert(vSize.size() >= 3);							// at least 1 input layer, 1 hidden layer, 1 output layer
	for ( size_t i=0; i<vSize.size(); ++i )
	{
		m_Layers.push_back(Layer(vSize[i], bAddBias));	// add new layer
	}
}

// ConnectFullyConnected connects each layers neurons with all neurons of next layer
void NeuralNetwork::ConnectFullyConnected()
{
	for ( size_t j=1; j<m_Layers.size(); ++j )
	{
		// note: XxxLayer.Size() is neuron count without bias neuron
		// XxxLayer.m_Neurons.size() is neuron count including bias neuron
		Layer&				SrcLayer = m_Layers[j-1];
		Layer&				DstLayer = m_Layers[j];
		for ( size_t nDst=0; nDst<DstLayer.Size(); ++nDst )
		{												// destinations exclude bias neuron
			Neuron*			pDst = DstLayer.m_Neurons[nDst];
			for ( size_t nSrc=0; nSrc<SrcLayer.m_Neurons.size(); ++nSrc )
			{											// sources include bias neuron
				Neuron*		pSrc = SrcLayer.m_Neurons[nSrc];
				Connection*	pConn = new Connection(pSrc, pDst);
				m_ConnectionsAll.push_back(pConn);
				pSrc->m_DstConnections.push_back(pConn);
				pDst->m_SrcConnections.push_back(pConn);
			}
		}
	}
}

// Feed forward function for the entire network.
void NeuralNetwork::FeedForward(const vector<double>& InputValues)
{
	assert(InputValues.size() == m_Layers[0].Size());	// size check

	// Assign (latch) the input values into the input neurons in m_layers[0]
	for ( size_t j=0; j<m_Layers[0].Size(); ++j )
	{
		m_Layers[0].m_Neurons[j]->SetOutputValue(InputValues[j]);
	}

	for ( size_t numLayer=1; numLayer<m_Layers.size(); ++numLayer )	// forward propagation for following layers
		m_Layers[numLayer].FeedForward();
}

// backpropagation function for the entire network
void NeuralNetwork::BackPropagation(const vector<double>& targetValues)
{
	// Verify that we have the right number of target output values:
	assert(m_Layers.back().Size() == targetValues.size() );

	// Calculate the gradients of all the neurons' outputs, starting at the output layer:
	m_Layers.back().CalcOutputGradients(targetValues);
	// and for all hidden layers
	for ( size_t layerNum=m_Layers.size()-2; layerNum>0; --layerNum )
	{
		m_Layers[layerNum].CalcHiddenGradients();
	}

	// For all layers from outputs to first hidden layer, in reverse order, update connection weights.
	for ( size_t layerNum= m_Layers.size()-1; layerNum>0; --layerNum )
	{
		m_Layers[layerNum].UpdateWeights(m_eta, m_alpha);
	}
}

//! get weights as string
void NeuralNetwork::GetWeights(string& sWeights)
{
	for ( size_t layerNum = 0; layerNum < m_Layers.size()-1; ++layerNum )
	{
		m_Layers[layerNum].GetWeights(sWeights);
	}
}

// calculate error values
void NeuralNetwork::CalcError(const vector<double>& targetValues)
{
	// Calculate overall net error (RMS of output neuron errors)
	Layer&		OutputLayer = m_Layers.back();
	size_t		nOutputLayerSize = OutputLayer.Size();
	m_Error = 0.0;
	for ( size_t n = 0; n < nOutputLayerSize; ++n )
	{
		double	delta = targetValues[n] - OutputLayer.m_Neurons[n]->GetOutputValue();
		m_Error += delta * delta;
	}
	m_Error /= nOutputLayerSize;						// get average error squared
	m_Error = sqrt(m_Error);							// RMS

	// Implement a recent average measurement
	m_RecentAverageError = ( m_RecentAverageError * m_RecentAverageSmoothingFactor + m_Error )
		/ ( m_RecentAverageSmoothingFactor + 1.0 );
}

// get results
void NeuralNetwork::GetResults(vector<double>& resultValues)
{
	resultValues.clear();
	Layer&		OutputLayer = m_Layers.back();
	for ( size_t j = 0; j < OutputLayer.Size(); ++j )
	{
		resultValues.push_back(OutputLayer.m_Neurons[j]->GetOutputValue());	// for all neurons in a layer
	}
}
