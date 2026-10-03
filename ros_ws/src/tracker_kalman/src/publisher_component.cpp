#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <rclcpp_components/register_node_macro.hpp> // 1. Indispensable

namespace my_camera_package {

class Publisher : public rclcpp::Node {
public:
    // 2. Le constructeur accepte et transmet les NodeOptions
    explicit Publisher(const rclcpp::NodeOptions & options) : Node("publisher", options), cap(0) {

        if (!cap(0).is_opened()){
            RCLCPP_ERROR(this->get_logger(), "Erreur : Impossible d'ouvrir la caméra (index 0) !")
        }

        rclcpp::QoS custom_qos(10);
        custom_qos.reliability(rclcpp::ReliabilityPolicy::BestEffort);
        custom_qos.durability(rclcpp::DurabilityPolicy::Volatile);

        publisher_ = this->create_publisher<sensor_msgs::msg::Image>("image/topic", custom_qos);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(33), [this](){ this->callback_timer(); });
    }

private:
    cv::VideoCapture cap;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    void callback_timer() {
        cv::Mat image_captured;
        cap >> image_captured;

        if (image_captured.empty()) {
            return;
        }

        std_msgs::msg::Header header;
        header.stamp = this->get_clock()->now();
        header.frame_id = "camera_link";

        auto rosimageptr = std::make_unique<sensor_msgs::msg::Image>();
        cv_bridge::CvImage(header, "bgr8", image_captured).toImageMsg(*rosimageptr);

        publisher_->publish(std::move(rosimageptr));
    }
};

} 

RCLCPP_COMPONENT_REGISTER_NODE(my_camera_package::Publisher)