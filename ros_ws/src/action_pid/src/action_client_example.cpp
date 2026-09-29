#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include <memory>
#include "package_name/action/Struct_name"



class ClientAction : public rclcpp::Node{
    public:

        ClientAction () : Node("ClientAction"){

            client_ = rclcpp_action::create_client<pkg_name::action::Struct_name>(this, "canal_action");
            timer_ = this -> create_wall_timer(std::chrono::milliseconds(500),[this](){this->sendGoal();});
        }


    private:

        std::shared_ptr<rclcpp_action::Client<pkg_name::action::Struct_name>> client_;
        std::shared_ptr<rclcpp::TimerBase> timer_;


        void sendGoal(){

            this->timer_->cancel();

            if (!client_->wait_for_action_server(std::chrono::seconds(10))){
                RCLCPP_ERROR(this->get_logger(),"Erreur survenue, serveur d'action injoignable");
                return;
            }




            pkg_name::action::Struct_name::Goal goal_msg;
            goal_msg.depart = 5;


            rclcpp_action::Client<pkg_name::action::Struct_name>::SendGoalOptions send_goal_options;

            //callback 1 : retour du goal response

            send_goal_options.goal_response_callback = [this](std::shared_ptr<rclcpp_action::ClientGoalHandle<pkg_name::action::Struct_name>> goal_handle){
                    if (!goal_handle){
                        RCLCPP_ERROR(this->get_logger(),"Erreur survenue, serveur d'action injoignable");
                    }

                    else{
                        RCLCPP_INFO{this->get_logger(),"Message bien recu et en cours de traitement"};
                    }
                };

            //callback2 : reception du feedback
            send_goal_options.feedback_callback =  [this](std::shared_ptr<rclcpp_action::ClientGoalHandle<pkg_name::action::Struct_name>> goal_handle, const std::shared_ptr<const pkg_name::action::Struct_name::Feedback> feedback){
                RCLCPP_INFO(this->get_logger(), "Feeback en cours de récéption : %i", feedback->temps_restant);
            }

            send_goal_options.result_callback = [this](const rclcpp_action::ClientGoalHandle<pkg_name::action::Struct_name::Feedback>::WrappedResult& enveloppe){

                switch (enveloppe.code){
                    case rclcpp_action::ResultCode::SUCCEEDED:
                         RCLCPP_INFO(this->get_logger(), "Reussite de l'action : %s", enveloppe.result->termine);
                              break; 

                    case rclcpp_action::ResultCode::ABORTED:
                        RCLCPP_INFO(this->get_logger(), "Echec du goal");
                             break; 

                    case rclcpp_action::ResultCode::CANCELED:
                        RCLCPP_INFO(this->get_logger(), "Annulation de l'action");
                             break; 
                    default:
                        RCLCPP_INFO(this->get_logger(), "fin inconnue");
                             break; 
                }


            }

            client -> async_send_goal(goal_msg, send_goal_options);
        

            }


        };

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(<std::make_shared<ClientAction>());
    rclcpp::shutdown();
    return 0;
}