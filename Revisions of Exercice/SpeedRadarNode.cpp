class SpeedRadarNode : public rclcpp::Node{

    public:
        SpeedRadarNode() : Node("SpeedRadarNode"){

        this->declare_parameter<double>("speed_limit",50.0);
        this->speed_limit = get_parameter("speed_limit").as_double();

        subscriber_ = this->create_subscription<std_msgs::msg::Float64>("/car_speed",10,[this](const std::shared_ptr<std_msgs::msg::Float64> msg){this->callback_subscription(msg);});
 
    }





    private:

    void callback_subscription(const std::shared_ptr<std_msgs::msg::Float64> msg){

        double data = msg->data;

        if (data > speed_limit){
            RCLCPP_WARN(this->get_logger(),"dépassement de la vitesse: %f", data);
        }
        else{
             RCLCPP_INFO(this->get_logger(),"Vitesse ok: %f", data);
        }
    }
        std::shared_ptr<rclcpp:Subscriber<std_msgs::msg::Float64>> subscriber_;
        double speed_limit;

};