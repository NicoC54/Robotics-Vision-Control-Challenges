#include "rclcpp/rclcpp.hpp"
#include <rclcpp_action/rclcpp_action.hpp>
#include <memory>
#include <thread>
#include <nom_package/action/nom_struct.hpp>

class Server : public rclcpp::Node{
    public:
        Server() : Node("Server_node"){


            action_server_ = rclcpp_action::create_server<nom_pkg::action::nom_struct>(this,"canal",
            [this](const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const nom_pkg::action::nom_struct::Goal> goal){return this->handle_goal(uuid,goal);},
            [this](std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){return this->handle_cancel(goal_handle);},
            [this](std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){this->handle_accept(goal_handle);});
        }

    private:

    std::shared_ptr<rclcpp_action::Server<nom_pkg::action::nom_struct>> action_server_;


        //callback receiveing goal callback
        rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const nom_pkg::action::nom_struct::Goal> goal){
            if (goal->depart <0){
                return rclcpp_action::GoalResponse::REJECT;
            }
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

        //callback2 cancel callback
        rclcpp_action::CancelResponse handle_cancel(std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        //callback3 accept callback

        void handle_accept(std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){
            auto thread_function = [this,goal_handle](){this->execute(goal_handle);};

            std::thread thread_execute(thread_function);
            thread_execute.detach();
        }

        void execute(std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){

            std::shared_ptr<const nom_package::action::nom_Struct::Goal> goal = goal_handle->get_goal();
            std::shared_ptr<nom_package::action::nom_Struct::Result> result = std::make_shared<nom_package::action::nom_Struct::Result>();
            std::shared_ptr<nom_package::action::nom_Struct::Feedback> feedback = std::make_shared<nom_package::action::nom_Struct::Feeback>();
            
            int temps_actuel = goal->depart;

            rclcpp::Rate loop_rate(1); //boucle dattente cadencée à 1hz

            while((temps_actuel>=0) && (rclcpp::ok())){
                if (goal_handle->is_canceling()){
                    result->termine = false;
                    goal_handle->canceled(result);
                    return;
                }

                temps_actuel--;
                feedback->temps_restant = temps_actuel;
                goal_handle->publish_feedback(feedback);

                loop_rate.sleep();
            }


            if (rclcpp::ok()){
                result->termine=true;
                goal_handle->succeed(result);
            }
            
        }

};


int main(int argc, char* argv[]){

    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<Server>());
    rclcpp::shutdown();
}