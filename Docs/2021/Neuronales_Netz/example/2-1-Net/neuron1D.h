/*

1 dimensional neural network
partially inspired by David R. Miller, 2015 version

*/

#include <vector>
#include <tuple>
using namespace std;

#pragma once

class Neuron;											// Forward references
class Layer;
class NeuralNetwork;

// function type for random number functions and transfer functions
typedef double (*ANNFunction_t)();
typedef double (*ANNTransferFunction_t)(double);

/*!
Connection
*/
class Connection
{
public:
	Connection(Neuron* from, Neuron* to);				///< constructor
	Neuron*				m_SrcNeuron;					///< source connection (from)
	Neuron*				m_DstNeuron;					///< destination connection (to)
	double				m_Weight;						///< updated in BackPropagation() else constant
	double				m_DeltaWeight;					///< only used in BackPropagation()
};

/*!
Neuron

Basic element of a neuronal network.

Neurons are organized in layers.
Each Neuron in a layer receives data from all Neurons in the layer above
and delivers data to all Neurons in the following layer.
*/
class Neuron
{
	friend class NeuralNetwork;
public:
	Neuron();											///< constructor

	void SetOutputValue(double value) { m_OutputValue = value; }
	double GetOutputValue() const { return m_OutputValue; }
	//! Feed forward function for a neuron
	void FeedForward(Layer* pLayer);
	//! calculate output neuron gradient
	void CalcOutputGradients(double targetVal);
	//! calculate hidden neuron gradient
	void CalcHiddenGradients();
	///! update weights
	void UpdateInputWeights(double eta, double alpha);
	//! get weights as string
	void GetWeights(string& sWeights);

//private:
public:
	double				m_OutputValue;					///< neuron output value
	double				m_Gradient;						///< only used in backPropagation()
	vector<Connection*>	m_SrcConnections;				///< back connections (previous layer)
	vector<Connection*>	m_DstConnections;				///< forward connections (next layer)
};

/*!
Layer
*/
class Layer
{
	friend class NeuralNetwork;									// allow access to m_Neurons
public:
	Layer(size_t nSize, bool bAddBias);					///< constructor
	//! number of neurons without bias neuron
	size_t Size() const { return m_nSize; }
	//! Feed forward function for a layer
	void FeedForward();
	//! calculate output gradients
	void CalcOutputGradients(const vector<double>& targetValues);
	//! calculate hidden neuron gradient
	void CalcHiddenGradients();
	//! update weights
	void UpdateWeights(double eta, double alpha);
	//! get weights as string
	void GetWeights(string& sWeights);

//private:
public:
	size_t				m_nSize;						///< layer geometry without bias neuron
	bool				m_bUseBias;						///< flag if bias neuron used
	vector<Neuron*>		m_Neurons;						///< m_Neurons[j] within a layer
};

/*!
NeuralNetwork
*/
class NeuralNetwork
{
public:
	NeuralNetwork();									///< constructor
	~NeuralNetwork();									///< destructor

	//! Create layers
	void CreateLayers1D(vector<size_t>& vSize, bool bAddBias);
	//! ConnectFullyConnected connects each layers neurons with all neurons of next layer
	void ConnectFullyConnected();

	//! set random number function for initial weights
	void SetRandomNumberFunction(ANNFunction_t pFunction) { m_pRandomFunction = pFunction; }
	//! Set training rate
	void SetTrainingRate(double eta) { m_eta = eta; }
	//! Set last weight change
	void SetLastWeightChange(double alpha) { m_alpha = alpha; }
	//! set transfer function and it's derivative
	void SetTransferFunctions(ANNTransferFunction_t pFunction, ANNTransferFunction_t pFunctionDerivative) {
		m_pTransferFunction = pFunction; m_pTransferFunctionDerivative = pFunctionDerivative;
	}

	//!	Feed forward function for the entire network
	void FeedForward(const vector<double>& inputValues);
	//! backpropagation function for the entire network
	void BackPropagation(const vector<double>& targetValues);
	//! calculate error values
	void CalcError(const vector<double>& targetValues);
	//! get error
	double GetError() { return m_Error; }
	//! get recent average error
	double GetRecentAverageError() { return m_RecentAverageError; }
	//! get results
	void GetResults(vector<double>& resultValues);
	//! get weights as string
	void GetWeights(string& sWeights);

//private:
public:
	vector<Layer>					m_Layers;						///< list of layers, first is input, last is output

	double							m_Error;						///< calucation error (not a software error)
	double							m_RecentAverageError;			///< averaged over m_RecentAverageSmoothingFactor samples
	double							m_RecentAverageSmoothingFactor;	///< For some calculations, we use a running average of net error, averaged over this many input samples:

	static ANNFunction_t			m_pRandomFunction;				///< selected random number function for initial weights
	static ANNTransferFunction_t	m_pTransferFunction;			///< transfer function
	static ANNTransferFunction_t	m_pTransferFunctionDerivative;	///< transfer function derivative

//public:
private:
	static vector<Connection*>		m_ConnectionsAll;				///< list of all connections between neurons
	static double					m_eta;							///< [0.0..1.0] overall net training rate, only used in backPropagation()
	static double					m_alpha;						///< [0.0..n] multiplier of last weight change (momentum), only used in backPropagation()
};
