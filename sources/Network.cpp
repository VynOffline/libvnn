/*
* @Author: Vyn
* @Date:   2019-02-02 11:29:39
* @Last Modified by:   Vyn
* @Last Modified time: 2019-03-04 14:47:19
*/

#include "Network.h"

#include "Layer.h"
#include "Neuron.h"
#include "Connection.h"

#include <iostream>
#include <cmath>

Network::Network()
{

}

void					Network::AddLayer(int nbNeuron, int neuronType, int nbBias)
{
	Layer	*newLayer;
	newLayer = new Layer(nbNeuron, neuronType, nbBias);
	AddLayer(newLayer);
}

void					Network::AddLayer(Layer *layer)
{
	Layer	*lastLayer;

	if (layers.size() == 0)
	{
		layers.push_back(layer);
		return ;
	}
	lastLayer = layers[layers.size() - 1];
	lastLayer->ConnectTo(layer);
	layers.push_back(layer);
}

void					Network::SetCostFunction(int id)
{
	if (id == COST_FUNCTION_MSE)
	{
		this->SetCostFunction(&SquaredError);
		this->SetCostFunctionDerivative(&SquaredErrorDerivative);
	}
}

std::vector<value_t>	Network::Predict(std::vector<value_t> inputs)
{
	std::vector<Neuron *>	inputLayerNeurons;
	std::vector<Neuron *>	outputLayerNeurons;
	std::vector<value_t>	outputValues;

	if (layers.size() < 2)
		throw std::string("There must be minimum 2 layers (input & output)");
	inputLayerNeurons = layers[0]->GetNeurons();
	if (inputLayerNeurons.size() - layers[0]->GetBiasCount() != inputs.size())
		throw std::string("Missing inputs");
	for (std::vector<Neuron *>::size_type i = 0; i < inputs.size(); ++i)
		inputLayerNeurons[i]->SetValue(inputs[i]);
	for (std::vector<Layer *>::size_type i = 1; i < layers.size(); ++i)
		layers[i]->ComputeValues();
	outputLayerNeurons = layers[layers.size() - 1]->GetNeurons();
	for (std::vector<Neuron *>::size_type i = 0; i < outputLayerNeurons.size(); ++i)
		outputValues.push_back(outputLayerNeurons[i]->GetValue());
	lastPredictionValues = outputValues;
	return (outputValues);
}

value_t					Network::GetCost(values_t expectedOutput)
{
	if (costFunction != nullptr)
		return ((*costFunction)(GetOutputLayer()->GetNeurons(), expectedOutput));
	throw std::string("No cost function defined");
}

value_t					Network::GetDerivedCost(values_t expectedOutput, Neuron *outputNeuron)
{
	if (costFunctionDerivative != nullptr)
		return ((*costFunctionDerivative)(GetOutputLayer()->GetNeurons(), expectedOutput, outputNeuron));
	throw std::string("No cost function derivative defined");
}

Layer					*Network::GetInputLayer() const
{
	if (layers.size() > 0)
		return (layers[0]);
	return (nullptr);
}

Layer					*Network::GetOutputLayer() const
{
	if (layers.size() > 1)
		return (layers[layers.size() - 1]);
	return (nullptr);
}

static void				UpdateNeuronWeights(Neuron* neuron);

static void				CalcNeuron(Neuron *neuron)
{
	std::vector<Connection *>	connections;
	value_t 					currentDerivedValue;

	currentDerivedValue = 0;
	connections = neuron->GetOutputConnections();
	for (std::vector<Connection *>::size_type i = 0; i < connections.size(); ++i)
	{
		currentDerivedValue += (connections[i]->GetOutput()->GetDerivedError() * connections[i]->GetOutput()->GetDerivedValue(neuron));
	}
	neuron->SetDerivedError(currentDerivedValue);
	UpdateNeuronWeights(neuron);
}

static void				UpdateNeuronWeights(Neuron *neuron)
{
	std::vector<Connection *>	connections;
	
	connections = neuron->GetInputConnections();
	for (std::vector<Connection *>::size_type i = 0; i < connections.size(); ++i)
	{
		connections[i]->SetNextWeight(connections[i]->GetWeight() - 0.1 * (neuron->GetDerivedError() * neuron->GetDerivedValue(connections[i])));
		connections[i]->SetShouldUpdate(true);
	}
}

void					Network::Propagate(value_t goodValue)
{
	values_t					goodValues;
	std::vector<Neuron *>		outputLayerNeurons;
	std::vector<Neuron *>		neurons;
	value_t						derivedCost;

	goodValues.push_back(goodValue);
	outputLayerNeurons = this->GetOutputLayer()->GetNeurons();
	for (std::vector<Neuron *>::size_type i = 0; i < outputLayerNeurons.size(); ++i)
	{
		derivedCost = this->GetDerivedCost(goodValues, outputLayerNeurons[i]);
		outputLayerNeurons[i]->SetDerivedError(derivedCost);
		UpdateNeuronWeights(outputLayerNeurons[i]);
	}
	for (std::vector<Layer *>::size_type i = 0; i < layers.size() - 1; ++i)
	{
		neurons = layers[(layers.size() - 2) - i]->GetNeurons();
		for (std::vector<Neuron *>::size_type j = 0; j < neurons.size(); ++j)
		{
			if (!neurons[j]->IsBias())
				CalcNeuron(neurons[j]);
		}
	}
	this->UpdateWeights();
}

void					Network::UpdateWeights()
{
	std::vector<Connection *>		connections;

	connections = Connection::GetConnections();
	for (std::vector<Connection *>::size_type i = 0; i < connections.size(); ++i)
	{
		if (connections[i]->ShouldUpdate())
		{
			weight_t newWeight;
			newWeight = connections[i]->GetNextWeight();
			
			connections[i]->SetWeight(newWeight);
			connections[i]->SetShouldUpdate(false);
		}
	}
}