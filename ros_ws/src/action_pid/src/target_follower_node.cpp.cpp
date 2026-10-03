#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <memory>
#include <thread>
#include "action_pid/action/follow_target.hpp"

class PID {
public:
    PID(double p, double i, double d) : kp(p), ki(i), kd(d), last_error(0), integral_sum(0) {}

    double compute(double dt, double error) {
        double proportionnal = kp * error;
        integral_sum += error * dt;
        double integral = ki * integral_sum;
        double derivative = kd * (error - last_error) / dt;
        last_error = error;
        return proportionnal + integral + derivative;
    }
private:
    double kp, ki, kd, last_error, integral_sum;
};

class ServerAction : public rclcpp::Node {
public:
    
    using PursuitAction = action_pid::action::FollowTarget;
    using GoalHandlePursuit = rclcpp_action::ServerGoalHandle<PursuitAction>;

    ServerAction() : Node("ServerAction") {
        server_ = rclcpp_action::create_server<PursuitAction>(
            this, 
            "canal_action",
            [this](const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const PursuitAction::Goal> goal) {
                return this->handle_goal(uuid, goal);
            },
            [this](const std::shared_ptr<GoalHandlePursuit> goal_handle) {
                return this->handle_cancel(goal_handle);
            },
            [this](const std::shared_ptr<GoalHandlePursuit> goal_handle) {
                this->handle_accept(goal_handle);
            }
        );
        tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    }

private:
    std::shared_ptr<rclcpp_action::Server<PursuitAction>> server_;
    std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

    bool SeeObject() {
        return tf_buffer_->canTransform("base_link", "cible", tf2::TimePointZero);
    }

    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& /*uuid*/, std::shared_ptr<const PursuitAction::Goal> /*goal*/) {
        if (SeeObject()) {
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        } else {
            return rclcpp_action::GoalResponse::REJECT;
        }
    }

    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandlePursuit> /*goal_handle*/) {
        RCLCPP_INFO(this->get_logger(), "Demande d'annulation recue");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accept(const std::shared_ptr<GoalHandlePursuit> goal_handle) {
   
        std::thread{ [this, goal_handle]() { this->execute(goal_handle); } }.detach();
    }

    void execute(const std::shared_ptr<GoalHandlePursuit> goal_handle) {
        auto feedback = std::make_shared<PursuitAction::Feedback>();
        auto result = std::make_shared<PursuitAction::Result>();
        
        PID my_pid(1.0, 0.1, 0.01);
        geometry_msgs::msg::TransformStamped tf;
        rclcpp::Rate loop(10);

        while (rclcpp::ok() && !goal_handle->is_canceling() && SeeObject()) {
            
            if (goal_handle->is_canceling()) {
                goal_handle->canceled(result);
                return;
            }

            try {
                tf = tf_buffer_->lookupTransform("base_link", "cible", tf2::TimePointZero);
            } catch (const tf2::TransformException& ex) {
                RCLCPP_WARN(this->get_logger(), "Erreur TF2 : %s", ex.what());
                loop.sleep();
                continue;
            }

            double error = tf.transform.translation.x;
            double correction = my_pid.compute(0.1, error);
            
         
            feedback->vitesse = correction;
            goal_handle->publish_feedback(feedback);

            loop.sleep();
        }

        if (!SeeObject()) {
            goal_handle->abort(result);
        } else if (rclcpp::ok()) {
            goal_handle->succeed(result);
        }
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ServerAction>());
    rclcpp::shutdown();
    return 0;
}