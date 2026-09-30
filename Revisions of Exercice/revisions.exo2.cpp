#include <vector>
#include <queue>
#include <memory>
#include <cmath>
#include <iostream>
#include <algorithm>




//fonctions utilitaires

class Astar{

    Astar(int cols, int lines, std::vector<int> grid) : nb_cols(cols), nb_lines(lines){}
    
    public :

        int getX(int index){
            return index%nb_cols;

        }

        int getY(int index){
            return index/nb_cols;

        }

        int getIndex(int x, int y){
            return nb_cols*y + x;

        }


        double heuristique (int starting_node, int ending_node){
            double dx = getX(starting_node) - getX(ending_node);
            double dy = getY(starting_node) - getY(ending_node);
            return std::abs(dx) + std::abs(dy);
        }


        std::vector<int> ConstructNodesPath(int index, const std::vector<int>& parents){
            std::vector<int> path;
            while (index!=-1){
                path.push_back(index);
                index = parents[index];
            }
            std::reverse(path.begin(),path.end());
            return path;
        }


        std::vector<int> findNeighbors(int index){

            int current_index_x = getX(index);
            int current_index_y = getY(index);
            std::vector<int> neighbors;

            if (current_index_x > 0){
                neighbors.push_back(getIndex(current_index_x-1, current_index_y));
            }
            if (current_index_x < nb_cols-1){
                neighbors.push_back(getIndex(current_index_x+1, current_index_y));
            }
            if (current_index_y > 0){
                neighbors.push_back(getIndex(current_index_x, current_index_y-1));
            }
            if (current_index_y < nb_lines-1){
                neighbors.push_back(getIndex(current_index_x, current_index_y+1));
            }

            return neighbors;

        }

        std::vector<int> ComputeAstar(int starting_node, int ending_node, const std::vector<int>& grid){

            double INF = 10e9;

            std::vector<int> parents(grid.size(), -1);
            std::vector<double> gscores(grid.size(), INF);
            
            using Pair = std::pair<double,int>;
            std::priority_queue<Pair,std::vector<Pair>,std::greater<Pair>> pq;

            gscores[starting_node]=0;
            double initial_f_score =  gscores[starting_node] + heuristique(starting_node,ending_node);
            pq.push({initial_f_score, starting_node});


            while (!pq.empty()){

                auto [current_f_score,current_node_id] = pq.top();
                pq.pop();

                if (current_node_id == ending_node){
                    return ConstructNodesPath(current_node_id, parents);
                }
                double current_best_fscore = gscores[current_node_id] + heuristique(current_node_id, ending_node);

                if (current_f_score > current_best_fscore){
                    continue;
                }

                std::vector<int> neighbors;
                neighbors = findNeighbors(current_node_id);

                for (int neighbor : neighbors){

                    if (grid[neighbor]==100){
                        continue;
                    }

                    double current_neighbor_gscore = gscores[current_node_id] + 1;

                    if (current_neighbor_gscore < gscores[neighbor]){
                        gscores[neighbor] = current_neighbor_gscore;
                        parents[neighbor] = current_node_id;
                        double f_score_current_neighbor = gscores[neighbor] + heuristique(neighbor, ending_node);
                        pq.push({f_score_current_neighbor,neighbor});
                    }



                }
            }

            return {};

        }

    private :

        int nb_cols;
        int nb_lines;
        

};