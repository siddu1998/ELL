////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     SGDOptimizer.tcc (optimization)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// utilities
#include "Exception.h"

// stl
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace ell
{
namespace trainers
{
namespace optimization
{
    /// <summary> </summary>
    template <typename SolutionType, typename LossFunctionType>
    SGDOptimizer<SolutionType, LossFunctionType>::SGDOptimizer(std::shared_ptr<const ExampleSetType> examples, LossFunctionType lossFunction, SGDOptimizerParameters parameters) :
        _examples(examples), 
        _lossFunction(std::move(lossFunction)), 
        _lambda(parameters.regularization)
    {
        if (examples.get() == nullptr || examples->Size() == 0)
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidSize, "Empty dataset");
        }

        // setup random engine
        std::seed_seq seed(parameters.randomSeedString.begin(), parameters.randomSeedString.end());
        _randomEngine.seed(seed);

        auto example = examples->Get(0);
        _lastW.Resize(example.input, example.output);
        _averagedW.Resize(example.input, example.output);
    }

    template <typename SolutionType, typename LossFunctionType>
    void SGDOptimizer<SolutionType, LossFunctionType>::PerformEpochs(size_t count)
    {
        if (_examples == nullptr)
        {
            throw utilities::LogicException(utilities::LogicExceptionErrors::notInitialized, "Call SetExamples before calling Epoch");
        }

        std::vector<size_t> permutation(_examples->Size());
        std::iota(permutation.begin(), permutation.end(), 0);

        // epochs
        for (size_t e = 0; e < count; ++e)
        {
            // generate random permutation
            std::shuffle(permutation.begin(), permutation.end(), _randomEngine);

            // process each example
            for (size_t index : permutation)
            {
                Step(_examples->Get(index));
            }
        }
    }

    template <typename SolutionType, typename LossFunctionType>
    void SGDOptimizer<SolutionType, LossFunctionType>::Step(ExampleType example)
    {
        const auto& x = example.input;
        const auto& y = example.output;
        double weight = example.weight;

        ++_t;

        // predict
        auto p = x * _lastW;

        // calculate the loss derivative
        auto derivative = _lossFunction.Derivative(p, y);
        derivative *= -weight / (_lambda * _t);

        // update the solution
        double inverseT = 1.0 / _t;
        _lastW = _lastW * (1.0 - inverseT) + Transpose(x) * derivative;
        _averagedW = _averagedW * (1.0 - inverseT) + _lastW * inverseT;
    }

    template <typename SolutionType, typename LossFunctionType>
    SGDOptimizer<SolutionType, LossFunctionType> MakeSGDOptimizer(std::shared_ptr<const typename SolutionType::ExampleSetType> examples, LossFunctionType lossFunction, SGDOptimizerParameters parameters)
    {
        return SGDOptimizer<SolutionType, LossFunctionType>(examples, lossFunction, parameters);
    }
}
}
}