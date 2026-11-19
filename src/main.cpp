#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <format>

#include "Population.hpp"
#include "Parser.hpp"
#include "BranchAndBound.hpp"

const int FUNCTIONS_QNT = 4; 
const int POPULATION_QNT = 50;

std::unordered_map<int, std::string> m = 
{
    {0, "RCL                               "},
    {1, "RCL + Ruin + Recreate             "},
    {2, "RCL + 2-OPT + Ruin + Recreate     "},
    {3, "Branch and Bound                  "}
};

std::vector<std::vector<Result>> runTests(Population* pop, int, std::vector<double>* averageTimes)
{
    BranchAndBound bnb(*pop);
    std::vector<std::vector<Result>> results(FUNCTIONS_QNT);
    
    averageTimes->assign(FUNCTIONS_QNT, 0.);

    for(int i{0}; i < POPULATION_QNT; ++i)
    {
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        Individual res = pop->generateGrasp();
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        
        double indTime = std::chrono::duration<double, std::milli>(end - begin).count();
    
        results[0].push_back({res, indTime});
        (*averageTimes)[0] += indTime / POPULATION_QNT;
    
        begin = std::chrono::high_resolution_clock::now();
        res = pop->recreate(pop->ruin(pop->generateGrasp()));
        end = std::chrono::high_resolution_clock::now();
    
        indTime = std::chrono::duration<double, std::milli>(end - begin).count();
    
        results[1].push_back({res, indTime});
        (*averageTimes)[1] += indTime / POPULATION_QNT;
    
        begin = std::chrono::high_resolution_clock::now();
        res = pop->recreate(pop->ruin(pop->opt2(pop->generateGrasp())));
        end = std::chrono::high_resolution_clock::now();
    
        indTime = std::chrono::duration<double, std::milli>(end - begin).count();
    
        results[2].push_back({res, indTime});
        (*averageTimes)[2] += indTime / POPULATION_QNT;
    }

    std::chrono::high_resolution_clock::time_point beginO = std::chrono::high_resolution_clock::now();
    Individual otherRes = bnb.solve();
    std::chrono::high_resolution_clock::time_point endO = std::chrono::high_resolution_clock::now();
    
    double indTimeO = std::chrono::duration<double, std::milli>(endO - beginO).count();

    results[3].push_back({otherRes, indTimeO});
    (*averageTimes)[3] = indTimeO;
    
    for(auto& v : results)
        std::sort(v.begin(), v.end(), [&](const Result& a, const Result& b)
        {
            return a.ind.score < b.ind.score;
        });

    return results;
}

signed main(int argc, char* argv[])
{
    Population gaPop = Parser::parse(argc, argv);
        
    std::vector<double> averageTimes;

    auto results = runTests(&gaPop, POPULATION_QNT, &averageTimes);

    Parser::printResults(results, averageTimes, m);
    Parser::exportResults(results, averageTimes, m);

    return 0;
}