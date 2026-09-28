#include "rclcpp/rclcpp.hpp"
#include <memory>
#include "astar_service/srv/get_path.hpp"


class Client : public rclcpp::Node {

    public:
        Client() : Node("Client_node"){
            client_ = this->create_client<astar_service::srv::GetPath>("service_canal");
        }


        void sendRequest(){

            while(!client_->wait_for_service(std::chrono::seconds(1))){
                RCLCPP_INFO(get_logger(),"service non joignable reesai dans 1s");
            }

            auto msg = std::make_shared<astar_service::srv::GetPath::Request>();
            msg->starting_index = 0;
            msg->ending_index = 8;
            msg->grid = {
                    0,  0,  0,
                    0, 100, 0,
                    0,  0,  0
            };
            msg->nb_cols = 3;
            msg->nb_lines =3;

            using Future = rclcpp::Client<astar_service::srv::GetPath>::SharedFuture;

            auto lambda = [this](Future future)
            {auto response = future.get();
            std::cout<< "Réponse recue, taille de la réponse"<< response->path.size()<<std::endl;
            for (int index : response->path) {
            RCLCPP_INFO(rclcpp::get_logger("client"), "-> Case index : %d", index);}};

            client_->async_send_request(msg, lambda);
        }

    private:
        std::shared_ptr<rclcpp::Client<astar_service::srv::GetPath>> client_;

};

int main(int argc, char* argv[]){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<Client>();
    node -> sendRequest(); 
    rclcpp::spin(node);
    rclcpp::shutdown();

}

