#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <rclcpp_components/register_node_macro.hpp>

namespace my_camera_package{

class KF{

    public:

    KF(){

    X << x, y, vx, vy;
        
    H << 1, 0, 0, 0,
            0, 1, 0, 0;

    R << 9, 0,
            0, 9;

    Q <<    0.1, 0, 0, 0,
            0, 0.1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    F << 1, 0, period, 0,
            0, 1, 0, period,
            0, 0, 1, 0,
            0, 0, 0, 1;

    }


    void compute(double mesure_x, double mesure_y){

        

        //j'ai pris les formules de kalman dinternet, mais la logique dinsertion des mesures vient de moi

        K = P*H.transpose()*(H*P*H.transpose() + R).inverse();

        //update
        Eigen::Matrix<double,2,1> mesure;

        mesure << mesure_x,
                  mesure_y;

        P = (Identity - K*H)*P;
        X = X + K*(mesure-H*X);

        std::cout<<"mesure estimée en x: " << X(0,0)<< "pixels" << std::endl;
        std::cout<<"mesure estimée en y: " << X(1,0)<< "pixels" << std::endl; 

        //Extrapolation

        P = F*P*F.transpose() + Q;
        X = F*X;
    }


    private:

        double x=0,y=0,vx=0,vy=0;
        double period = 0.1;

        Eigen::Matrix<double,4,1> X;
        Eigen::Matrix<double,2,4> H;
        Eigen::Matrix<double,2,2> R;
        Eigen::Matrix<double,4,4> Q;
        Eigen::Matrix<double,4,2> K;
        Eigen::Matrix4d Identity = Eigen::Matrix4d::Identity();
        Eigen::Matrix4d P = Identity*1e4;
        Eigen::Matrix<double,4,4> F;

        //U = 0 car lobjet rouge est un objet extérieur dont on ne connait pas la physique;

};

        

class Subscriber : public rclcpp::Node{

    public:

        explicit Subscriber(const rclcpp::NodeOptions& options) : Node("Subscriber", options) {

            rclcpp::QoS subscriber_qos(10);
            subscriber_qos.reliability(rclcpp::ReliabilityPolicy::BestEffort);
            subscriber_qos.durability(rclcpp::DurabilityPolicy::Volatile);

            this -> qos = subscriber_qos;

            subscriber_ = this->create_subscription<sensor_msgs::msg::Image>("topic/image", qos, [this](const std::shared_ptr<const sensor_msgs::msg::Image> msg){this->callbackSubscriber(msg);});

        }

    private:

        rclcpp::QoS qos{10};
        std::shared_ptr<rclcpp::Subscription<sensor_msgs::msg::Image>> subscriber_;
        KF kf;

        void callbackSubscriber(const std::shared_ptr<const sensor_msgs::msg::Image> msg){

            cv::Mat ImageFromMsg;

            cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg,"bgr8");
            ImageFromMsg = cv_ptr->image;

            //passage en image hsv

            cv::Mat hsv,mask;
            cv::cvtColor(ImageFromMsg, hsv, cv::COLOR_BGR2HSV);

            cv::Scalar lim_down(0,70,70);
            cv::Scalar lim_up(10,255,255);
            cv::inRange(hsv, lim_down, lim_up, mask);

            std::vector<std::vector<cv::Point>> contours;
            double max_area = 0;
            double x_coordinate = 0;
            double y_coordinate = 0;


            cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            if (contours.empty()){
                RCLCPP_WARN(this->get_logger(),"aucun contour detécté");
                return;
            }

            for (size_t i =0 ; i < contours.size(); i++ ){

                cv::Moments M = cv::moments(contours[i]);

                if (M.m00 > max_area){

                    max_area = M.m00;
                    x_coordinate = M.m10/M.m00;
                    y_coordinate = M.m01/M.m00;
                }

            }

             if (max_area>0){

                kf.compute(x_coordinate, y_coordinate);}

        }
};


}

RCLCPP_COMPONENTS_REGISTER_NODE(my_camera_package::Subscriber);

