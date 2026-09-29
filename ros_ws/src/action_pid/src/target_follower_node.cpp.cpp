

class PID{

    public:
        PID(double p, double i, double d) : kp(p), ki(i), kd(d), last_error(0),integral_sum(0) {}

        double compute (double dt, double error){

            double proportionnal = kp * error;
            integral_sum += error*dt;
            double integral = ki* integral_sum;
            double derivative = kd * (error-last_error)/dt;
            last_error = error;

            return proportionnal + integral + derivative;

        }


    private:

    double kp,ki,kd,last_error, integral_sum;

};

class ServerAction : public rclcpp::Node{
     public:

        ServerAction() : Node("ServerACtion"){

                server_ = rclcpp_action::create_server<pkg_name::action::Struct_name>(this, "canal_action",
                [this](const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const pkg_name::action::Struct_name::Goal> goal){return this->handle_goal(uuid,goal);},
                [this](const std::shared_ptr<rclcpp_action::ServerGoalHandle<pkg_name::action::Struct_name>> goal_handle){return this->handle_cancel(goal_handle);},
                [this](const std::shared_ptr<rclcpp_action::ServerGoalHandle<pkg_name::action::Struct_name>> goal_handle){this->handle_accept(goal_handle);}  
        );
                tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
                tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    }

    private:

       
        std::shared_ptr<rclcpp_action::Server<pkg_name::action::Struct_name>> server_;
        std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
        std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

        bool SeeObject(){
            bool cantransform = tf_buffer_->canTransform("base_link","cible",tf2::TimePointZero);
            return cantransform;
        }


        rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& uuid, std::shared_ptr<const pkg_name::action::Struct_name::Goal> goal){
            bool seeobject = SeeObject();
            if (seeobject){
                return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
            }
            else{
                return rclcpp_action::GoalResponse::REJECT;
            }
        }

        rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<pkg_name::action::Struct_name>> goal_handle){
            RCLCPP_INFO(this->get_logger(),"demande d'annulation recue");
                return rclcpp_action::CancelResponse::ACCEPT;
            }
        

        void handle_accept(const std::shared_ptr<rclcpp_action::ServerGoalHandle<pkg_name::action::Struct_name>> goal_handle){

            auto thread_function = [this,goal_handle](){this->execute(goal_handle);};
            std::thread ThreadExecute(thread_function);
            ThreadExecute.detach();}



        void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<pkg_name::action::Struct_name>> goal_handle){
            std::shared_ptr<const pkg_name::action::Struct_name::Goal> goal = goal_handle->get_goal();
            std::shared_ptr<pkg_name::action::Struct_name::Feedback> feedback = std::make_shared<pkg_name::action::Struct_name::Feedback>();
            std::shared_ptr<pkg_name::action::Struct_name::Result> result = std::make_shared<pkg_name::action::Struct_name::Result>();
            bool seeobject = SeeObject();
           
            PID my_pid(1,0.1,0.01);
            geometry_msgs::msg::TransformStamped tf;

            rclcpp::Rate loop(10);

            while((!goal_handle->is_canceling()) && seeobject ==true){
                seeobject = SeeObject();
                
                if (goal_handle->is_canceling()){
                    result->terminated = false;
                    goal_handle->canceled(result);
                    return;
                }

                if (seeobject==false){
                    result->terminated = false;
                    goal_handle->abort(result);
                    return;
                }

                try{
                    tf = tf_buffer_->lookupTransform("base_link", "cible", tf2::TimePointZero);
                }
                catch (const tf2::TransformException& ex){
                    RCLCPP_WARN(this->get_logger(), "Erreur TF2 :%s", ex.what());
                    loop.sleep();
                    continue;
                }

                double error = tf.transform.translation.x;

                
                double correction = my_pid.compute(0.1, error);
                //convertir la correction en vitesse moteur et la publier par feedback
                double vitesse_moteur = correction;
                
                feedback->vitesse = vitesse_moteur;
                goal_handle->publish_feedback(feedback);
                

                loop.sleep();
        }





};

