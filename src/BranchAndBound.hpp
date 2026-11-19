#ifndef BRANCH_AND_BOUND_HPP
#define BRANCH_AND_BOUND_HPP

#include "Population.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstdint>

struct State 
{
    int node;
    uint64_t mask;
    
    bool operator==(const State& other) const 
    {
        return node == other.node && mask == other.mask;
    }
};

struct StateHash 
{
    std::size_t operator()(const State& s) const 
    {
        uint64_t x = s.mask ^ (static_cast<uint64_t>(s.node + 1) * 0x9E3779B97F4A7C15ULL);  //< 2^64 / fibophi para distribuir legal

        /// Não pergunte para mim sobre esse mix, só peguei a ideia de MurmurHash3
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL; 
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL; 
        x ^= x >> 33;

        return static_cast<std::size_t>(x);
    }
};

class BranchAndBound
{
    private:
        std::unique_ptr<Population> pop;
        Individual bestInd;
        
        std::unordered_map<State, double, StateHash> memo;

        std::vector<double> minIn;
        std::vector<int>    order;

        void precompute(const std::vector<std::vector<double>>& paths,
                        const std::vector<double>& profits,
                        int endNode);

        void bbHelper(int currentNode, double currentDist, double currentScore, 
                    std::vector<bool>& visited, uint64_t visitedMask, std::vector<int>& currentPath, 
                    const std::vector<std::vector<double>>& paths, 
                    const std::vector<double>& profits, 
                    double tMax, int endNode);

        double calculateUpperBound(double currentDist, 
                                uint64_t visitedMask, 
                                const std::vector<std::vector<double>>& paths, 
                                const std::vector<double>& profits, 
                                double tMax, int endNode) const;

    public:
        BranchAndBound(const Population& population)
        {
            pop = std::make_unique<Population>(population);

            bestInd = pop->generateProfitRatioInsertion();
            for(auto a : bestInd.path)
                std::cout << a << ' ';

        }

        Individual solve();
};

#endif