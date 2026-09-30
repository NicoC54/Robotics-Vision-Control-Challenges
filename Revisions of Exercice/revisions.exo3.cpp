

class PID{

    public:

        PID(double p, double i, double d) : kp(p), ki(i), kd(d), integral_sum(0.0), last_error(0.0), max_integral(100) {}

        double compute(double dt, double error){
            if (dt <= 0) return 0.0;

            double proportional = kp*error;

            integral_sum += error*dt;
            integral_sum = std::clamp(integral_sum, -max_integral, max_integral);

            double integral = integral_sum * ki;

            double derivative = (error-last_error)/dt *kd;
            last_error=error;

            return proportional+integral+derivative;
        }


    private:

        double kp;
        double ki;
        double kd;
        double integral_sum;
        double max_integral;
        double last_error;      
};




class ServerNode : public rclcpp::Node{


    public:

        ServerNode() : Node("Server_Node") {

            server_ = rclcpp_action::create_server<nom_pkg::action::nom_struct>(this,"canal_action",
            //callback reception goal
            [this](const rclcpp_action::GoalUUID& uuid, const shared_ptr<const nom_pkg::action::nom_struct::Goal> goal){return this->handle_goal(uuid,goal)},
            //callback cancel
            [this](const shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){return this->handle_cancel(goal_handle)},
            //callback acceptation       
            [this](const shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){this->handle_execute(goal_handle)},    );

            tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
            tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
        }

        bool SeeObjectX(){
            return tf_buffer_->canTransform("base_link", "objectX_tf", tf2::TimePointZero);
        }
        
      
        rclcpp_action::goalResponse handle_goal(const rclcpp_action::GoalUUID& uuid, const std::shared_ptr<const nom_pkg::action::nom_struct::Goal> goal){

            bool vision = SeeObjectX();

            if (vision){
                return rclcpp_action::goalResponse::ACCEPT_AND_EXECUTE;
            }
            else{
                return rclcpp_action::goalResponse::REJECT;
            }
        }

        rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        handle_execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){
            
            auto execute_thread = [this,goal_handle](){this->execute(goal_handle)}
            
            std::thread ThreadExecute(execute_thread);
            ThreadExecute.detach();
        }

        void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<nom_pkg::action::nom_struct>> goal_handle){


            std::shared_ptr<const nom_pkg::action::nom_struct::Goal> goal = goal_handle->get_goal();
            std::shared_ptr<nom_pkg::action::nom_struct::Feedback>> feedback = std::make_shared<nom_pkg::action::nom_struct::Feedback>>();
            std::shared_ptr<nom_pkg::action::nom_struct::Result>> result = std::make_shared<nom_pkg::action::nom_struct::Result>>();

            

            PID myPid(1,0.1,0.01);

            rclcpp::Rate loop(10);

            while(!goal_handle->is_canceling()){

                bool vision = SeeObjectX();

                if (goal_handle->is_cancelling()){
                    result->terminer = false;
                    goal_handle->canceled(result);
                    return;
                }

                if(!vision){
                    result->terminer = false;
                    goal_handle->abort(result);
                    return;
                }

                geometry_msgs::msg::TransformStamped tf_msg;

                try {
                    tf_msg = tf_buffer_->lookupTransform("base_link","objectX_tf", tf2::TimePointZero);
                    double error = tf_msg.transform.translation.x;
                    double correction = myPid.compute(0.1,error);
                    feedback->vitesse_moteur = correction;
                    goal_handle->publish_feedback(feedback);
                }

                catch(const tf2::TransformException& ex){
                    RCLCPP_WARN(this->get_logger(),"impossible de récupérer la tf %s", ex.what());
                }

                loop.sleep();

            }


        }







}


int main(int argc, char* argv[]){

    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<ServerNode>())
    rclcpp::shutdown();

}