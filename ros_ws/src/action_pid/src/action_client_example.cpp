#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "action_pid/action/follow_target.hpp"

class ClientAction : public rclcpp::Node {
public:
    using FollowAction = action_pid::action::FollowTarget;
    using GoalHandleFollow = rclcpp_action::ClientGoalHandle<FollowAction>;

    ClientAction() : Node("action_client_example") {
        client_ = rclcpp_action::create_client<FollowAction>(this, "canal_action");
    }

    void sendGoal() {
        if (!client_->wait_for_action_server(std::chrono::seconds(10))) {
            RCLCPP_ERROR(this->get_logger(), "Serveur d'action introuvable après 10 secondes.");
            return;
        }

        auto goal_msg = FollowAction::Goal();
        goal_msg.target_frame = "cible";

        typename rclcpp_action::Client<FollowAction>::SendGoalOptions send_goal_options;

        send_goal_options.goal_response_callback = [this](const std::shared_ptr<GoalHandleFollow> goal_handle) {
            if (!goal_handle) {
                RCLCPP_ERROR(this->get_logger(), "Objectif rejeté par le serveur.");
            } else {
                RCLCPP_INFO(this->get_logger(), "Objectif accepté par le serveur.");
            }
        };

        send_goal_options.feedback_callback = [this](
            GoalHandleFollow::SharedPtr,
            const std::shared_ptr<const FollowAction::Feedback> feedback) {
            RCLCPP_INFO(this->get_logger(), "Feedback reçu - Vitesse moteur : %f", feedback->vitesse);
        };

        send_goal_options.result_callback = [this](const GoalHandleFollow::WrappedResult& result) {
            switch (result.code) {
                case rclcpp_action::ResultCode::SUCCEEDED:
                    RCLCPP_INFO(this->get_logger(), "Action réussie !");
                    break;
                case rclcpp_action::ResultCode::ABORTED:
                    RCLCPP_ERROR(this->get_logger(), "Action abandonnée (perte de cible).");
                    break;
                case rclcpp_action::ResultCode::CANCELED:
                    RCLCPP_ERROR(this->get_logger(), "Action annulée.");
                    break;
                default:
                    RCLCPP_ERROR(this->get_logger(), "Code de résultat inconnu.");
                    break;
            }
            rclcpp::shutdown();
        };

        client_->async_send_goal(goal_msg, send_goal_options);
    }

private:
    rclcpp_action::Client<FollowAction>::SharedPtr client_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ClientAction>();
    node->sendGoal();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}