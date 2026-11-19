#include "BranchAndBound.hpp"
#include <cassert>
#include <limits>

namespace
{
    constexpr double INF = std::numeric_limits<double>::infinity();
}

void BranchAndBound::precompute(const std::vector<std::vector<double>>& paths,
                                const std::vector<double>& profits,
                                int endNode)
{
    const int n = static_cast<int>(profits.size());

    minIn.assign(n, INF);
    for(int i{0}; i < n; ++i)
    {
        if(i == endNode) 
            continue;

        for(int j{0}; j < n; ++j)
            if(j != i && j != endNode)
                minIn[i] = std::min(minIn[i], std::max(0., paths[j][i]));
    }

    order.clear();
    order.reserve(n);
    for(int i{0}; i < n; ++i)
        if(i != endNode && minIn[i] < INF)
            order.push_back(i);

    auto ratio = [&](int i) 
    {
        return (minIn[i] <= 0.) ? INF : profits[i] / minIn[i];
    };

    std::sort(order.begin(), order.end(), [&](int a, int b) 
    {
        return ratio(a) > ratio(b);
    });
}

double BranchAndBound::calculateUpperBound(double currentDist, 
                                           uint64_t visitedMask, 
                                           const std::vector<std::vector<double>>& paths, 
                                           const std::vector<double>& profits, 
                                           double tMax, int endNode) const
{
    const double timeLeft = tMax - currentDist;

    double minEnd{INF};

    for(auto i : order)
    {
        if((visitedMask >> i) & 1ULL)
            continue;

        const double toEnd = paths[i][endNode];
        if(minIn[i] + toEnd <= timeLeft && toEnd < minEnd)
            minEnd = toEnd;
    }

    if(minEnd == INF)
        return 0.;

    const double capacity = timeLeft - minEnd + 1e-9;
    
    if(capacity < 0.)
        return 0.;

    double potentialScore{0.};
    double used{0.};

    for(int i : order)
    {
        if((visitedMask >> i) & 1ULL)
            continue;

        if(minIn[i] + paths[i][endNode] > timeLeft)
            continue;

        const double w = minIn[i];

        if(w <= 0.)
        {
            potentialScore += profits[i];
            continue;
        }
        if(used + w <= capacity)
        {
            used += w;
            potentialScore += profits[i];
        }
        else
        {
            potentialScore += profits[i] * ((capacity - used) / w);
            break;
        }
    }

    return potentialScore;
}

void BranchAndBound::bbHelper(int currentNode, double currentDist, double currentScore, 
                              std::vector<bool>& visited, uint64_t visitedMask, std::vector<int>& currentPath, 
                              const std::vector<std::vector<double>>& paths, 
                              const std::vector<double>& profits, 
                              double tMax, int endNode)
{
    State state{currentNode, visitedMask};
    auto it = memo.find(state);

    if(it != memo.end()) 
    {
        if(currentDist >= it->second - 1e-6) 
            return;
        
        it->second = currentDist; 
    } 
    else 
        memo[state] = currentDist;

    double upperBound = calculateUpperBound(currentDist, visitedMask, paths, profits, tMax, endNode);

    if(currentScore + upperBound <= bestInd.score)
        return;
    
    double distToEnd = paths[currentNode][endNode];

    if(currentDist + distToEnd <= tMax)
        if(currentScore > bestInd.score) 
        {
            Individual cand;
            cand.path.assign(profits.size(), SIZE_MAX);

            for(std::size_t i{0}; i < currentPath.size(); ++i)
                cand.path[i] = currentPath[i];
            
            pop->calcScore(cand); 

            if(cand.score > bestInd.score)
                bestInd = cand;
        }

    std::vector<int> candidates;
    candidates.reserve(visited.size());

    for(std::size_t i{0}; i < visited.size(); ++i) 
        if(!visited[i] && static_cast<int>(i) != endNode) 
            if(currentDist + paths[currentNode][i] + paths[i][endNode] <= tMax) 
                candidates.push_back(static_cast<int>(i));

    std::sort(candidates.begin(), candidates.end(), 
    [&](int a, int b) 
    {
        double distA = (paths[currentNode][a] <= 1e-6) ? 1e-4 : paths[currentNode][a];
        double distB = (paths[currentNode][b] <= 1e-6) ? 1e-4 : paths[currentNode][b];
        return (profits[a] / distA) > (profits[b] / distB);
    });

    for(auto i : candidates)
    {
        double distToNext = paths[currentNode][i];
        
        visited[i] = true;
        uint64_t nextMask = visitedMask | (1ULL << i);
        currentPath.push_back(i);

        bbHelper(i, currentDist + distToNext, currentScore + profits[i], 
                 visited, nextMask, currentPath, paths, profits, tMax, endNode);

        currentPath.pop_back();
        visited[i] = false;
    }
}

Individual BranchAndBound::solve()
{
    const auto& paths = *(pop->paths);
    const auto& profits = *(pop->profits);
    double tMax = pop->TMax;
    int startNode = pop->startNode;
    int endNode = pop->endNode;
    std::size_t cityQnt = pop->cityQnt;

    assert(cityQnt <= 64 && "visitedMask suporta no máximo 64 nós");

    std::vector<bool> visited(cityQnt, false);
    std::vector<int> currentPath;

    memo.clear(); 
    precompute(paths, profits, endNode);

    visited[startNode] = true;
    uint64_t startMask = (1ULL << startNode);
    currentPath.push_back(startNode);

    double startScore = profits[startNode];

    bbHelper(startNode, 0., startScore, visited, startMask, currentPath, paths, profits, tMax, endNode);

    return bestInd;
}