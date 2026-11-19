#ifndef POPULATION_HPP
#define POPULATION_HPP

#include <vector>
#include <random>

struct Individual 
{
    std::vector<int> path;
    double score;
    double dist;
    double fitness;
    int vis;
};

class Population 
{
    private:
 
        friend class BranchAndBound;

        std::size_t cityQnt;
        
        const int startNode{};
        const int endNode{};
        const double TMax{};
        const std::vector<std::vector<double>>* paths{nullptr};
        const std::vector<double>* profits{nullptr};
        std::mt19937 rng{std::random_device{}()};
        
        void calcScore(Individual& ind) const;   
        
    public:

        Population(std::vector<std::vector<double>>*, std::vector<double>*, double, int, int);

        /*Heuristicas de Melhoria*/
        Individual opt2(const Individual&);
        Individual opt3(const Individual&);
        Individual opt4BI(const Individual&);
        Individual opt4FI(const Individual&);
        Individual ruin(const Individual&);
        Individual recreate(const Individual&);
        
        /*Heuristicas Construtivas*/
        Individual generateGrasp();
        Individual generateRandom();
        Individual generateProfitRatioInsertion();
        

        std::vector<Individual> pop;
};


#endif //< POPULATION_HPP