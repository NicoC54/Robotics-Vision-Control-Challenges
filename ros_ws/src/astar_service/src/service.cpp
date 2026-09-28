#include "rclcpp/rclcpp.hpp"
#include <memory>

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

#include "astar_service/srv/get_path.hpp"

class Astar{

    public:
        int nb_cols;
        int nb_lines;
        
        std::vector<int> grid;

        Astar(int nb_cols, int nb_lines, std::vector<int> grid) : nb_cols(nb_cols), nb_lines(nb_lines), grid(grid){}


        int getX(int index){
            return index%nb_cols;
        }

        int getY(int index){
            return index/nb_cols;
        }
        int gotoIndex(int x, int y){
            return y*nb_cols + x;
        }

        std::vector<int> findNeighbors(int index){

            std::vector<int> neighbors;

            int current_index_x = getX(index);
            int current_index_y = getY(index);

            if (current_index_x >0){
                neighbors.push_back(gotoIndex(current_index_x-1,current_index_y));
            }
            if (current_index_x < nb_cols - 1){
                neighbors.push_back(gotoIndex(current_index_x+1,current_index_y));
            }
            if (current_index_y > 0){
                neighbors.push_back(gotoIndex(current_index_x,current_index_y-1));
            }
            if (current_index_y < nb_lines - 1){
                neighbors.push_back(gotoIndex(current_index_x,current_index_y+1));
            }


            return neighbors;

        }

        int heuristique(int starting_index, int ending_index){
            int dx = getX(starting_index) - getX(ending_index);
            int dy = getY(starting_index) - getY(ending_index);
            return std::abs(dx)+std::abs(dy); //distance de manhattan car cest une grille

        }

        std::vector<int> ConstructPath(int ending_index, const std::vector<int>& parents){
            std::vector<int> path;
            while (ending_index != -1){
                path.push_back(ending_index);
                ending_index = parents[ending_index];
            }

            std::reverse(path.begin(),path.end());
            return path;
        }

        std::vector<int> AlgoAstar(int starting_index, int ending_index){

            const double INF = 10e9;

            std::vector<int> parents(grid.size(),-1);
            std::vector<double> gscores(grid.size(), INF);

            using Pair = std::pair<double,int>;
            std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;

            gscores[starting_index] = 0;
            int initial_f_score = heuristique(starting_index,ending_index);
            pq.push({initial_f_score,starting_index});

            while(!pq.empty()){
                auto [current_f_score, current_index] = pq.top();
                pq.pop();

                if (current_index == ending_index){
                    return ConstructPath(current_index, parents);
                }

                double max_f_score = gscores[current_index] + heuristique(current_index, ending_index);
                if (current_f_score>max_f_score){
                    continue;
                }

                std::vector<int> neighbors_indexes_for_current_index = findNeighbors(current_index);

                for (auto& neighbor_index : neighbors_indexes_for_current_index){

                    if (grid[neighbor_index]==100){
                        continue;
                    }
                    double neighbor_index_gscore = gscores[current_index] + 1;

                    if (neighbor_index_gscore < gscores[neighbor_index]){
                        gscores[neighbor_index] = neighbor_index_gscore;
                        parents[neighbor_index] = current_index;
                        double f_score = gscores[neighbor_index] + heuristique(neighbor_index,ending_index);
                        pq.push({f_score,neighbor_index});
                    }
                }
            }

            return {};





        }
    };
/*

int main(){

    std::vector<int> grid = {
        0,  0,  0,
        0, 100, 0,
        0,  0,  0
    };

    int nb_cols = 3;
    int nb_lines = 3;

    Astar monAstar(nb_cols, nb_lines, grid);

    std::vector<int> sortie = monAstar.AlgoAstar(0, 8);

    for (int element : sortie){
        std::cout << "L'ordre des indexes est:" << element <<std::endl;
    }




}

*/

class Service : public rclcpp::Node {

    public:
        Service() : Node("Service_node"){

            service_ = create_service<astar_service::srv::GetPath>("service_canal",[this](const std::shared_ptr<astar_service::srv::GetPath::Request>& request, const std::shared_ptr<astar_service::srv::GetPath::Response>& response){callbackService(request,response);});
        }



    private:
        
        void callbackService(const std::shared_ptr<astar_service::srv::GetPath::Request>& request, const std::shared_ptr<astar_service::srv::GetPath::Response>& response){
            int starting_index = request->starting_index;
            int ending_index = request-> ending_index;
            std::vector<int> grid = request->grid;
            int nb_cols = request-> nb_cols;
            int nb_lines = request-> nb_lines;
            Astar myAstar(nb_cols,nb_lines,grid);
            response->path = myAstar.AlgoAstar(starting_index,ending_index);
        }

        std::shared_ptr<rclcpp::Service<astar_service::srv::GetPath>> service_;
};


int main(int argc, char* argv[]){
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<Service>());
    rclcpp::shutdown();
}