#pragma once

#include <vector>
#include <algorithm>
#include <iterator>

// General transform function that works with any container (like std::vector or std::initializer_list)
// and any transformation function.
template<typename InputContainer, typename TransformationFunction>
auto transformVector(const InputContainer & input, TransformationFunction transformFunc)
  -> std::vector<decltype(transformFunc(*input.begin()))>
{
    std::vector<decltype(transformFunc(*input.begin()))> result;
    std::transform(input.begin(), input.end(), std::back_inserter(result), transformFunc);
    return result;
}