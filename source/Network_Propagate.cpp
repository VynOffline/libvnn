/*
* @Author: Vyn
* @Date:   2019-03-24 10:06:27
* @Last Modified by:   Vyn
* @Last Modified time: 2019-05-10 14:56:52
*/

#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <float.h>

#include "Network.h"
#include "Layer.h"
#include "Neuron.h"
#include "Connection.h"

#define ABS(x) (x < 0 ? -x : x)

namespace Vyn
{

	namespace NeuralNetwork
	{

		void					Network::CalcNeuron(Neuron *neuron)
		{
			Value 					currentDerivedValue;

			currentDerivedValue = 0;
			const Connections &connections = neuron->GetOutputConnections();
			for (Connections::size_type i = 0; i < connections.size(); ++i)
			{
				DEBUG_CHECK_VALUE(connections[i]->GetOutput()->GetDerivedError(), "Current derived value at output neuron");
				DEBUG_CHECK_VALUE(connections[i]->GetOutput()->GetDerivedValue(neuron), "Derived value w.r.t neuron (self)");
				DEBUG_CHECK_VALUE(currentDerivedValue, "Current derived value for self");
				currentDerivedValue += (connections[i]->GetOutput()->GetDerivedError() * connections[i]->GetOutput()->GetDerivedValue(neuron));
				DEBUG_CHECK_VALUE(currentDerivedValue, "New current derived value for self (" + std::to_string(connections[i]->GetOutput()->GetDerivedError()) + " * " + std::to_string(connections[i]->GetOutput()->GetDerivedValue(neuron)));
			}

			if (std::isinf(currentDerivedValue))
				throw std::string("current derived value inf");
			neuron->SetDerivedError(currentDerivedValue);
			UpdateNeuronWeights(neuron);
		}

		void					Network::UpdateNeuronWeights(Neuron *neuron)
		{
			const Connections &connections = neuron->GetInputConnections();

			for (Connections::size_type i = 0; i < connections.size(); ++i)
			{
				Weight	gradient;

				DEBUG_CHECK_VALUE(neuron->GetDerivedError(), "Current derived error");
				DEBUG_CHECK_VALUE(neuron->GetDerivedValue(connections[i]), "Derived value w.r.t connection");
				gradient = neuron->GetDerivedError() * neuron->GetDerivedValue(connections[i]);
				DEBUG_CHECK_VALUE(gradient, "gradient of weight");

				connections[i]->SetGradient(gradient);
				connections[i]->SetShouldUpdate(true);
			}
		}

		void					Network::Propagate(Values const &goodValues, Values const &derivedCost)
		{
			const Neurons &outputLayerNeurons = GetOutputLayer()->GetNeurons();
			for (Neurons::size_type i = 0; i < outputLayerNeurons.size(); ++i)
			{
				DEBUG_CHECK_VALUE(derivedCost[i], "Derived cost");
				outputLayerNeurons[i]->SetDerivedError(derivedCost[i]);
				UpdateNeuronWeights(outputLayerNeurons[i]);
			}
			
			for (Layers::size_type i = 0; i < layers.size() - 1; ++i)
			{
				const Neurons &neurons = layers[(layers.size() - 2) - i]->GetNeurons();
				for (Neurons::size_type j = 0; j < neurons.size(); ++j)
				{
					if (!neurons[j]->IsBias())
						CalcNeuron(neurons[j]);
				}
			}
			this->UpdateWeights();
		}

		void					Network::Propagate(Values const &goodValues)
		{
			const Neurons &outputLayerNeurons = GetOutputLayer()->GetNeurons();
			Values derivedCost;

			derivedCost.reserve(outputLayerNeurons.size());
			for (Neurons::size_type i = 0; i < outputLayerNeurons.size(); ++i)
			{
				derivedCost.push_back(this->GetDerivedCost(goodValues, i));
			}
			this->Propagate(goodValues, derivedCost);
		}

		void					Network::UpdateWeights()
		{
			for (Connections::size_type i = 0; i < connections.size(); ++i)
			{
				if (connections[i]->ShouldUpdate())
				{
					Value gradient;

					gradient = learningRate * connections[i]->GetGradient();
					DEBUG_CHECK_VALUE(gradient, "gradient value after learning rate");
					if (gradientClipping != 0)
					{
						if (gradient > gradientClipping)
							gradient = gradientClipping;
						else if (gradient < -gradientClipping)
							gradient = -gradientClipping;
					}
					connections[i]->SetWeight(connections[i]->GetWeight() - gradient);
					connections[i]->SetShouldUpdate(false);

					if (connections[i]->GetWeight() < -100 || connections[i]->GetWeight() > 100)
						throw std::string("Weight too high");

				}
			}
		}
	}
}