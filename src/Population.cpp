#include "Population.hpp"

#include <algorithm>
#include <cstdint>

Individual Population::generateRandom()
{
    Individual ind;
    ind.path.assign(cityQnt, SIZE_MAX);
    ind.path[0] = startNode;

    std::vector<int> available_cities;
    available_cities.reserve(cityQnt);

    for(std::size_t i{0}; i < cityQnt; ++i) 
        if(i != std::size_t(startNode) && i != std::size_t(endNode)) 
            available_cities.push_back(i);
    

    std::shuffle(available_cities.begin(), available_cities.end(), rng);


    std::size_t current_node = startNode;
    double current_dist{0.};
    std::size_t idx{1};

    for(std::size_t next_city : available_cities) 
    {
        if(idx >= cityQnt) 
            break;

        double dist_to_next = (*paths)[current_node][next_city];
        double dist_to_end  = (*paths)[next_city][endNode];

        if(current_dist + dist_to_next + dist_to_end <= TMax) 
        {
            ind.path[idx] = next_city;
            current_dist += dist_to_next;
            current_node = next_city;
            idx++;
        }
    }

    calcScore(ind);

    return ind;
}

Individual Population::generateProfitRatioInsertion()
{
    Individual ind;
    ind.path.assign(cityQnt, SIZE_MAX); 
    ind.path[0] = startNode;

    std::vector<bool> visited(cityQnt, false);
    visited[startNode] = true;
    visited[endNode] = true;

    int currentNode = startNode;
    double currentDist{0.};
    std::size_t idx{1};

    while(idx < cityQnt) 
    {
        std::vector<std::pair<double, std::size_t>> candidates; 

        for(std::size_t i{0}; i < cityQnt; ++i) 
        {
            if(!visited[i]) 
            {
                double distToNext = (*paths)[currentNode][i];
                double distToEnd = (*paths)[i][endNode];

                if(currentDist + distToNext + distToEnd <= TMax) 
                {
                    double distSafe = (distToNext <= 1e-8) ? 1e-4 : distToNext; 
                    double ratio = (*profits)[i] / distSafe; 
                    candidates.push_back({ratio, i});
                }
            }
        }

        if(candidates.empty()) 
            break; 

        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {return a.first < b.first;});

        int chosenCity = candidates.back().second;

        ind.path[idx] = chosenCity;
        currentDist += (*paths)[currentNode][chosenCity];
        currentNode = chosenCity;
        visited[chosenCity] = true;
        idx++;
    }

    calcScore(ind); 
    return ind;
}

Individual Population::generateGrasp()
{
    Individual ind;
    ind.path.assign(cityQnt, SIZE_MAX); 
    ind.path[0] = startNode;

    std::vector<bool> visited(cityQnt, false);
    visited[startNode] = true;
    visited[endNode] = true;

    int currentNode = startNode;
    double currentDist{0.};
    std::size_t idx{1};

    while(idx < cityQnt) 
    {
        std::vector<std::pair<double, std::size_t>> candidates; 

        for(std::size_t i{0}; i < cityQnt; ++i) 
        {
            if(!visited[i]) 
            {
                double distToNext = (*paths)[currentNode][i];
                double distToEnd = (*paths)[i][endNode];

                if(currentDist + distToNext + distToEnd <= TMax) 
                {
                    double distSafe = (distToNext <= 1e-8) ? 1e-4 : distToNext; 
                    double ratio = (*profits)[i] / distSafe; 
                    candidates.push_back({ratio, i});
                }
            }
        }

        if(candidates.empty()) 
            break; 

        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {return a.first > b.first;});

        std::size_t rcl_size = std::min(std::size_t(3), candidates.size());
        
        std::uniform_int_distribution<std::size_t> dist_rand(0, rcl_size - 1);

        std::size_t chosenIdx = dist_rand(rng);
        int chosenCity = candidates[chosenIdx].second;

        ind.path[idx] = chosenCity;
        currentDist += (*paths)[currentNode][chosenCity];
        currentNode = chosenCity;
        visited[chosenCity] = true;
        idx++;
    }

    calcScore(ind); 
    return ind;
}

Individual Population::opt2(const Individual& ind)
{
    Individual best = ind;
    std::vector<std::size_t> path;
    
    for(std::size_t n : best.path) 
        if(n != SIZE_MAX) 
            path.push_back(n);
    
    
    if(path.size() < 3) 
        return best;

    bool found{true};

    while(found)
    {
        found = false;
        std::size_t n = path.size();
        int bestI{-1}, bestJ{-1};
        double bestDelta{-1e-8};

        for(std::size_t i{1}; i < n - 1; ++i)
        {
            std::size_t A = path[i - 1];
            std::size_t B = path[i];

            for(std::size_t j{i + 1}; j <= n; ++j)
            {
                std::size_t C = path[j - 1];
                std::size_t D = (j == n) ? endNode : path[j];

                double currentDist = (*paths)[A][B] + (*paths)[C][D];
                double newDist     = (*paths)[A][C] + (*paths)[B][D];
                double delta       = newDist - currentDist;

                if(delta < bestDelta)
                {
                    bestDelta = delta;
                    bestI = i;
                    bestJ = j;
                }
            }

        }

        if(bestI != -1)
        {
            std::reverse(path.begin() + bestI, path.begin() + bestJ);
            found = true; 
        }

    }

    best.path.assign(cityQnt, SIZE_MAX);
    
    for(std::size_t i{0}; i < path.size(); ++i) 
        best.path[i] = path[i];
    
    calcScore(best);

    return best;
}

Individual Population::opt3(const Individual& ind)
{
    Individual best = ind;
    std::vector<std::size_t> path;
    
    for(std::size_t n : best.path) 
        if(n != SIZE_MAX) 
            path.push_back(n);
    
    
    if(path.size() < 4) 
        return best;

    bool found{true};

    while(found)
    {
        found = false;
        std::size_t n = path.size();
        int bestI{-1}, bestJ{-1}, bestK{-1};
        std::size_t bestMoveType{0};
        double bestDelta{-1e-8};

        for(std::size_t i{1}; i < n - 2; ++i)
        {
            std::size_t A = path[i - 1];
            std::size_t B = path[i];
            
            for(std::size_t j{i + 1}; j < n - 1; ++j)
            {
                std::size_t C = path[j - 1];
                std::size_t D = path[j];

                double ABCDdist = (*paths)[A][B] + (*paths)[C][D];
            
                for(std::size_t k{j + 1}; k <= n; ++k)
                {

                    std::size_t E = path[k - 1];
                    std::size_t F = (k == n) ? endNode : path[k];

                    double currentDist = ABCDdist + (*paths)[E][F];

                    double d1 = (*paths)[A][D] + (*paths)[E][B] + (*paths)[C][F] - currentDist;
                    double d2 = (*paths)[A][D] + (*paths)[E][C] + (*paths)[B][F] - currentDist;
                    double d3 = (*paths)[A][E] + (*paths)[D][B] + (*paths)[C][F] - currentDist;
                    double d4 = (*paths)[A][E] + (*paths)[D][C] + (*paths)[B][F] - currentDist;

                    if(d1 < bestDelta) 
                    { 
                        bestDelta = d1; 
                        bestI = i; 
                        bestJ = j; 
                        bestK = k; 
                        bestMoveType = 1; 
                    }
                    if(d2 < bestDelta) 
                    { 
                        bestDelta = d2; 
                        bestI = i; 
                        bestJ = j; 
                        bestK = k; 
                        bestMoveType = 2; 
                    }
                    if(d3 < bestDelta) 
                    { 
                        bestDelta = d3; 
                        bestI = i; 
                        bestJ = j; 
                        bestK = k; 
                        bestMoveType = 3; 
                    }
                    if(d4 < bestDelta) 
                    { 
                        bestDelta = d4; 
                        bestI = i; 
                        bestJ = j; 
                        bestK = k; 
                        bestMoveType = 4; 
                    }

                }
            }
        }
        if(bestMoveType != 0)
        {
            auto itI = path.begin() + bestI;
            auto itJ = path.begin() + bestJ;
            auto itK = path.begin() + bestK;

            if(bestMoveType == 1) 
                std::rotate(itI, itJ, itK);
            else if(bestMoveType == 2) 
            {
                std::reverse(itI, itJ);
                std::rotate(itI, itJ, itK);
            }
            else if(bestMoveType == 3) 
            {
                std::reverse(itJ, itK);
                std::rotate(itI, itJ, itK);
            }
            else if(bestMoveType == 4) 
                std::reverse(itI, itK);
            
            found = true; 
        }
        
    }

    best.path.assign(cityQnt, SIZE_MAX);
    
    for(size_t i{0}; i < path.size(); ++i) 
        best.path[i] = path[i];
    
    calcScore(best);

    return best;
}

Individual Population::opt4FI(const Individual& ind)
{
    Individual best = ind;
    std::vector<std::size_t> path;
    
    for(std::size_t n : best.path) 
        if(n != SIZE_MAX) 
            path.push_back(n);
    
    
    if(path.size() < 5) 
        return best;

    bool found{true};

    while(found)
    {
        found = false;
        std::size_t n = path.size();

        for(std::size_t i{1}; i < n - 3 && !found; ++i)
        {
            std::size_t A = path[i - 1];
            std::size_t B = path[i];

            for(std::size_t j{i + 1}; j < n - 2 && !found; ++j)
            {
                std::size_t C = path[j - 1];
                std::size_t D = path[j];

                double ABCDdist = (*paths)[A][B] + (*paths)[C][D];

                for(std::size_t k{j + 1}; k < n - 1 && !found; ++k)
                {
                    std::size_t E = path[k - 1];
                    std::size_t F = path[k];

                    double ABCDEFdist = ABCDdist + (*paths)[E][F];
                    
                    for(std::size_t l{k + 1}; l <= n && !found; ++l)
                    {
                        std::size_t G = path[l - 1];
                        std::size_t H = (l == n) ? endNode : path[l];

                        double currentDist = ABCDEFdist + (*paths)[G][H];
                        double d1 = ((*paths)[A][F] + (*paths)[G][D] + (*paths)[E][B] + (*paths)[C][H]) - currentDist;

                        if(d1 < -1e-6)
                        {
                            auto itI = path.begin() + i;
                            auto itJ = path.begin() + j;
                            auto itK = path.begin() + k;
                            auto itL = path.begin() + l;

                            std::size_t sizeC = std::distance(itJ, itK);
                            std::size_t sizeD = std::distance(itK, itL);

                            std::reverse(itI, itL);
                            std::reverse(itI, itI + sizeD);
                            std::reverse(itI + sizeD, itI + sizeD + sizeC);
                            std::reverse(itI + sizeD + sizeC, itL);
                            found = true;
                            break;
                        }

                        double d2 = ((*paths)[A][G] + (*paths)[F][D] + (*paths)[E][C] + (*paths)[B][H]) - currentDist;

                        if(d2 < -1e-6)
                        {
                            auto itI = path.begin() + i;
                            auto itJ = path.begin() + j;
                            auto itK = path.begin() + k;
                            auto itL = path.begin() + l;

                            std::size_t sizeC = std::distance(itJ, itK);
                            std::size_t sizeD = std::distance(itK, itL);

                            std::reverse(itI, itL);
                            std::reverse(itI + sizeD, itI + sizeD + sizeC);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
        
    }

    best.path.assign(cityQnt, SIZE_MAX);

    for(std::size_t i{0}; i < path.size(); ++i) 
        best.path[i] = path[i];
    
    calcScore(best);

    return best;
}

Individual Population::opt4BI(const Individual& ind)
{
    Individual best = ind;
    std::vector<std::size_t> path;
    
    for(std::size_t n : best.path) 
        if(n != SIZE_MAX) 
            path.push_back(n);
    
    if(path.size() < 5) 
        return best;

    bool found{true};

    while(found)
    {
        found = false;
        std::size_t n = path.size();
        double bestDelta{-1e-6};
        int bestI{-1}, bestJ{-1}, bestK{-1}, bestL{-1}, bestType{0};

        for(std::size_t i{1}; i < n - 3; ++i)
        {
            std::size_t A = path[i - 1];
            std::size_t B = path[i];

        
            for(std::size_t j{i + 1}; j < n - 2; ++j)
            {
                std::size_t C = path[j - 1];
                std::size_t D = path[j];

                double ABCDdist = (*paths)[A][B] + (*paths)[C][D];

                for(std::size_t k{j + 1}; k < n - 1; ++k)
                {
                    std::size_t E = path[k - 1];
                    std::size_t F = path[k];
            
                    double ABCDEFdist = ABCDdist + (*paths)[E][F];

                    for(std::size_t l{k + 1}; l <= n; ++l)
                    {
                        std::size_t G = path[l - 1];
                        std::size_t H = (l == n) ? endNode : path[l];

                        double currentDist = ABCDEFdist + (*paths)[G][H];
                        double d1 = ((*paths)[A][F] + (*paths)[G][D] + (*paths)[E][B] + (*paths)[C][H]) - currentDist;
                        double d2 = ((*paths)[A][G] + (*paths)[F][D] + (*paths)[E][C] + (*paths)[B][H]) - currentDist;

                        if(d1 < bestDelta)
                        {
                            bestDelta = d1;
                            bestI = i; 
                            bestJ = j; 
                            bestK = k;
                            bestL = l;
                            bestType = 1;
                        }
                        if(d2 < bestDelta)
                        {
                            bestDelta = d2;
                            bestI = i; 
                            bestJ = j; 
                            bestK = k; 
                            bestL = l;
                            bestType = 2;
                        }
                    }        
                }
            }
        }

        if(bestType != 0)
        {
            auto itI = path.begin() + bestI;
            auto itJ = path.begin() + bestJ;
            auto itK = path.begin() + bestK;
            auto itL = path.begin() + bestL;

            if(bestType == 1)
            {
                std::size_t sizeC = std::distance(itJ, itK);
                std::size_t sizeD = std::distance(itK, itL);

                std::reverse(itI, itL);
                
                std::reverse(itI, itI + sizeD);
                std::reverse(itI + sizeD, itI + sizeD + sizeC);
                std::reverse(itI + sizeD + sizeC, itL);
            }
            else if(bestType == 2)
            {
                std::size_t sizeC = std::distance(itJ, itK);
                std::size_t sizeD = std::distance(itK, itL);

                std::reverse(itI, itL);
                std::reverse(itI + sizeD, itI + sizeD + sizeC);
            }
            
            found = true;
        }
    }

    best.path.assign(cityQnt, SIZE_MAX);
    for(size_t i{0}; i < path.size(); ++i) 
        best.path[i] = path[i];
    
    calcScore(best);
    return best;
}

Individual Population::ruin(const Individual& ind)
{   
    Individual best = ind;
    std::vector<std::size_t> path;

    for(std::size_t n : best.path)
        if(n != SIZE_MAX)
            path.push_back(n);
    
    if(path.size() <= 2) 
        return best;

    std::size_t numToRemove = std::clamp(static_cast<std::size_t>(path.size() * 0.3), static_cast<std::size_t>(1), path.size() - 1);
    
    std::vector<std::size_t> idxPool(path.size() - 1);
    std::iota(idxPool.begin(), idxPool.end(), 1);
    std::shuffle(idxPool.begin(), idxPool.end(), rng);

    std::vector<std::size_t> toRemove(idxPool.begin(), idxPool.begin() + numToRemove);
    std::vector<bool> isRemoved(path.size(), false);

    for(std::size_t idx : toRemove)
        isRemoved[idx] = true;

    std::vector<std::size_t> newPath;
    newPath.reserve(path.size() - numToRemove);

    for(std::size_t i{0}; i < path.size(); ++i)
        if(!isRemoved[i])
            newPath.push_back(path[i]);
    

    best.path.assign(cityQnt, SIZE_MAX);
    
    for(std::size_t i{0}; i < newPath.size(); ++i) 
        best.path[i] = newPath[i];
    
    calcScore(best);

    return best;
}

Individual Population::recreate(const Individual& ind)
{
    Individual best = ind;
    std::vector<std::size_t> path;
    std::vector<bool> visited(cityQnt, false);
    
    for(size_t n : best.path) 
        if(n != SIZE_MAX) 
        {
            path.push_back(n);
            visited[n] = true;
        }
    
    visited[endNode] = true;
    bool inserted = true;

    while(inserted)
    {
        inserted = false;
        int bestPos{-1};
        int bestC {-1};
        double bestRatio{-1.};
        double bestAddedDist{0.};

        for(std::size_t i{0}; i < cityQnt; ++i) 
            if(!visited[i]) 
                for(std::size_t p{1}; p <= path.size(); ++p) 
                {
                    std::size_t prev = path[p - 1];
                    std::size_t next = (p == path.size()) ? endNode : path[p];
                    
                    double addedDist = (*paths)[prev][i] + (*paths)[i][next] - (*paths)[prev][next];
                    
                    if(best.dist + addedDist <= TMax) 
                    {
                        double safeDist = (addedDist <= 1e-4) ? 1e-4 : addedDist;
                        double ratio = (*profits)[i] / safeDist;
                        
                        if(ratio > bestRatio) 
                        {
                            bestRatio = ratio;
                            bestC = i;
                            bestPos = p;
                            bestAddedDist = addedDist;
                        }
                    }
                }
            
        if(bestC != -1) 
        {
            path.insert(path.begin() + bestPos, bestC);
            visited[bestC] = true;
            inserted = true;
            
            best.dist += bestAddedDist;
            best.score += (*profits)[bestC];
            best.fitness = best.score;
            best.vis++;
        }

    }

    best.path.assign(cityQnt, SIZE_MAX);

    for(std::size_t i{0}; i < path.size(); ++i) 
        best.path[i] = path[i];

    return best;
}

void Population::calcScore(Individual& ind) const
{
    ind.score = (*profits)[ind.path[0]];
    ind.dist = 0;
    std::size_t prev = ind.path[0];
    ind.vis = 1;
    
    for(std::size_t i{1}; i < ind.path.size(); ++i) 
    {
        if(static_cast<std::size_t>(ind.path[i]) == SIZE_MAX) 
            continue;

        double edge = (*paths)[prev][ind.path[i]];
        ind.dist += edge;
        ind.score += (*profits)[ind.path[i]];
        prev = ind.path[i];
        ind.vis++;
    }

    ind.dist += (*paths)[prev][endNode];

    ind.fitness = ind.score; 
}

Population::Population(std::vector<std::vector<double>>* paths, std::vector<double>* profits, double TMax, int startNode, int endNode)
: cityQnt(paths->size()), startNode(startNode), endNode(endNode), TMax(TMax), paths(paths), profits(profits){}

