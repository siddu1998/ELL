////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     SDCAOptimizer.tcc (optimization)
//  Authors:  Lin Xiao, Ofer Dekel
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
    template <typename SolutionType, typename LossFunctionType, typename RegularizerType>
    SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType>::SDCAOptimizer(std::shared_ptr<const ExampleSetType> examples, LossFunctionType lossFunction, RegularizerType regularizer, SDCAOptimizerParameters parameters) :
        _examples(examples),
        _lossFunction(lossFunction), 
        _regularizer(regularizer)
    {
        if (examples.get() == nullptr || examples->Size() == 0)
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidSize, "Empty dataset");
        }
        
        // set parameters
        _lambda = parameters.regularization;
        _desiredDualityGap = parameters.desiredDualityGap;
        _permuteData = parameters.permuteData;

        size_t numExamples = examples->Size();
        _normalizedInverseLambda = 1.0 / (numExamples * parameters.regularization);

        // set up random engine
        std::seed_seq seed(parameters.randomSeedString.begin(), parameters.randomSeedString.end());
        _randomEngine.seed(seed);

        // resize data structures according to examples
        auto firstExample = examples->Get(0);
        _w.Resize(firstExample.input, firstExample.output);
        _v.Resize(firstExample.input, firstExample.output);
        _exampleInfo.resize(numExamples);

        // initialize the per-example info and compute primal objective
        double primalSum = 0;
        for (size_t i = 0; i < numExamples; ++i)
        {
            auto example = examples->Get(i);

            // cache the norm of the example
            double norm2Squared = _w.GetNorm2SquaredOf(example.input); 
            if (norm2Squared == 0)
            {
                throw utilities::InputException(utilities::InputExceptionErrors::badData, "input cannot have zero norm");
            }
            _exampleInfo[i].norm2Squared = norm2Squared;

            // initialize the dual 
            _w.InitializeAuxiliaryVariable(_exampleInfo[i].dual);

            // compute the primal objective
            auto prediction = example.input * _w;
            primalSum += _lossFunction.Value(prediction, example.output);
        }

        _solutionInfo.primalObjective = primalSum / numExamples + _lambda * _regularizer.Value(_w);
    }

    template <typename SolutionType, typename LossFunctionType, typename RegularizerType>
    void SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType>::PerformEpochs(size_t count)
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
            // early exit
            if (_solutionInfo.DualityGap() <= _desiredDualityGap)
            {
                break;
            }

            // generate random permutation
            if (_permuteData)
            {
                std::shuffle(permutation.begin(), permutation.end(), _randomEngine);
            }

            // process each example
            for (size_t index : permutation)
            {
                Step(_examples->Get(index), _exampleInfo[index]);
            }

            _solutionInfo.numEpochsPerformed++;
            ComputeObjectives();
        }
    }

    template <typename SolutionType, typename LossFunctionType, typename RegularizerType>
    void SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType>::Step(ExampleType example, ExampleInfo& exampleInfo)
    {
        auto& dual = exampleInfo.dual;
        auto lipschitz = exampleInfo.norm2Squared * _normalizedInverseLambda;
        auto prediction = example.input * _w;
        prediction /= lipschitz;
        prediction += dual;

        auto newDual = _lossFunction.ConjugateProx(1.0 / lipschitz, prediction, example.output);
        dual -= newDual;
        dual *= _normalizedInverseLambda;

        _v += Transpose(example.input) * dual;
        _regularizer.ConjugateGradient(_v, _w);
        exampleInfo.dual = newDual;
    }

    template <typename SolutionType, typename LossFunctionType, typename RegularizerType>
    void SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType>::ComputeObjectives()
    {
        double primalSum = 0;
        double dualSum = 0;

        for (size_t i = 0; i < _examples->Size(); ++i)
        {
            auto example = _examples->Get(i);
            
            auto prediction = example.input * _w;
            primalSum += _lossFunction.Value(prediction, example.output);

            dualSum += _lossFunction.Conjugate(_exampleInfo[i].dual, example.output);
        }

        _solutionInfo.primalObjective = primalSum / _examples->Size() + _lambda * _regularizer.Value(_w);
        _solutionInfo.dualObjective = -dualSum / _examples->Size() - _lambda * _regularizer.Conjugate(_v);
    }

    template <typename SolutionType, typename LossFunctionType, typename RegularizerType>
    SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType> MakeSDCAOptimizer(std::shared_ptr<const typename SolutionType::ExampleSetType> examples, LossFunctionType lossFunction, RegularizerType regularizer, SDCAOptimizerParameters parameters)
    {
        return SDCAOptimizer<SolutionType, LossFunctionType, RegularizerType>(examples, lossFunction, regularizer, parameters);
    }
}
}
}