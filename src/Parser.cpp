#include "Parser.hpp"
#include <cmath>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

std::string Parser::output;

Population Parser::parse(int argc, char* argv[])
{
    std::string filename;
    if(argc < 2)
        filename = "tests/Tsiligirides1/tsiligirides_problem_1_budget_85.txt";
    else  
        filename = std::string(argv[1]);

    if(argc < 3)
        output = "teste.txt";
    else  
        output = std::string(argv[2]);

    std::ifstream ifs{filename};
    if(!ifs.is_open()) 
        throw std::runtime_error("Error reading the file: " + filename);

    std::string line;
    if(!getline(ifs, line)) 
        throw std::runtime_error("Error: Empty file: " + filename);
    
    
    std::stringstream ss{line};
    std::string tmax;
    ss >> tmax;

    std::vector<std::tuple<double, double, double>> vertices;
    

    while(getline(ifs, line))
    {
        std::stringstream ssl{ line };

        std::string x, y, point;
        if(ssl >> x >> y >> point) 
            vertices.push_back(std::tuple(std::stod(x), std::stod(y), std::stod(point)));
        
    }

    if(vertices.size() > 1) 
        std::swap(vertices[1], vertices.back());
    
    std::size_t N = vertices.size();

    auto profits = new std::vector<double>(N);
    auto dist = new std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));

    for(std::size_t i{0}; i < N; ++i)
        (*profits)[i] = std::get<2>(vertices[i]);

    for(std::size_t i{0}; i < N; ++i)
        for(std::size_t j{0}; j < N; ++j)
        {
            double dx = std::get<0>(vertices[i]) - std::get<0>(vertices[j]);
            double dy = std::get<1>(vertices[i]) - std::get<1>(vertices[j]);
            (*dist)[i][j] = std::sqrt(dx*dx + dy*dy);
        }

    return Population(dist, profits, std::stod(tmax), 0, dist->size() - 1);
}

void Parser::exportResults(const std::vector<std::vector<Result>>& res, const std::vector<double>& averageTimes, const std::unordered_map<int, std::string>& names)
{
    std::ofstream ofs(output);

    ofs << std::format("{:<38} | {:<8} | {:<8} | {:<9} | {:<11} | {:<10} | {:<11} \n", 
                            "Algorithm", 
                            "Median", 
                            "Max", 
                            "Delta Fit", 
                            "Average Time", 
                            "Overhead",
                            "Distance");

    Result graspMedian = res[0][res[0].size() / 2];

    for(std::size_t i{0}; i < names.size(); ++i)
    {
        Result median = res[i][res[i].size() / 2];

        ofs << std::format("{:<38} | {:<8.1f} | {:<8.1f} | {:<9} | {:<8.3f}ms | {:<10} | {:<11} \n",
                                names.at(i), 
                                median.ind.fitness,
                                res[i].back().ind.fitness, 
                                calc::formatRelativeFitness(median.ind.fitness, graspMedian.ind.fitness),
                                averageTimes[i], 
                                calc::formatRelativeTime(averageTimes[i], averageTimes[0]),
                                median.ind.dist
                            );
    }

    

    ofs.close();
}

void Parser::printResults(const std::vector<std::vector<Result>>& res, const std::vector<double>& averageTimes, const std::unordered_map<int, std::string>& names)
{
    std::cout << std::format("{:<38} | {:<8} | {:<8} | {:<9} | {:<11} | {:<10} | {:<11} \n", 
        "Algorithm", "Median", "Max", "Delta Fit", "Average Time", "Overhead", "Distance");
    std::cout << std::string(98, '-') << '\n';

    Result graspMedian = res[0][res[0].size() / 2];
    
    for(std::size_t i{0}; i < res.size(); ++i)
    {
        auto& v = res[i];
        
        Result median = v[v.size() / 2];
        
        std::cout << std::format("{:<38} | {:<8.1f} | {:<8.1f} | {:<9} | {:<8.3f}ms | {:<10} | {:<11} \n",
                                names.at(i), median.ind.fitness,
                                res[i].back().ind.fitness, 
                                calc::formatRelativeFitness(median.ind.fitness, graspMedian.ind.fitness),
                                averageTimes[i], 
                                calc::formatRelativeTime(averageTimes[i], averageTimes[0]),
                                median.ind.dist
                            );

    }
}

namespace calc
{

    double relativeTime(double indTime, double targetTime)
    {
        if(indTime <= 1e-3)
            return 0.;

        if(targetTime <= 1e-8) 
            return 1.;

        return indTime / targetTime;
    }

    double relativeFitness(double indFit, double graspFit)
    {
        if(graspFit <= 1e-8 || indFit <= 1e-8) 
            return 0.;

        return ((indFit - graspFit) / graspFit) * 100.; ;
    }

    std::string formatRelativeTime(double indTime, double targetTime)
    {
        return std::format("{:.2f}x", calc::relativeTime(indTime, targetTime));
    }
    std::string formatRelativeFitness(double indFit, double graspFit)
    {
        return std::format("{:+0.2f}%", calc::relativeFitness(indFit, graspFit));
    }

};