#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <tf2_ros/transform_broadcaster.hpp>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>
#include <memory>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

class PoseEstimator : public rclcpp::Node {
public:
    PoseEstimator() : Node("PoseEstimator") {
        this->declare_parameter<double>("marker_size", 0.1);
        this->declare_parameter<int>("dict_id", cv::aruco::DICT_4X4_50);

        this->size = this->get_parameter("marker_size").as_double();
        
        this->dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
        
        IntraMatrix = cv::Mat::eye(3, 3, CV_64F);
        DistortionMatrix = cv::Mat::zeros(1, 5, CV_64F);

        tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        
        subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera/image", 10, 
            [this](const sensor_msgs::msg::Image::SharedPtr msg) {
                this->callback_broadcaster(msg);
            }
        );
    }

private:
    void callback_broadcaster(const sensor_msgs::msg::Image::SharedPtr PtrRosImageMsg) {
        cv::Mat Image_subscribed = cv_bridge::toCvCopy(PtrRosImageMsg, "bgr8")->image;

        std::vector<cv::Point3f> aruco_real = {
            cv::Point3f(-size / 2.0, size / 2.0, 0),
            cv::Point3f(size / 2.0, size / 2.0, 0),
            cv::Point3f(size / 2.0, -size / 2.0, 0),
            cv::Point3f(-size / 2.0, -size / 2.0, 0)
        };

        cv::aruco::detectMarkers(Image_subscribed, dictionary, corners, ids);

        for (size_t i = 0; i < ids.size(); i++) {
            std::cout << "marqueur numéro: " << ids[i] << " detecté" << std::endl;
            std::cout << "coin haut gauche x: " << corners[i][0].x << " coin haut gauche y: " << corners[i][0].y << std::endl;
            std::cout << "coin haut droit x: " << corners[i][1].x << " coin haut droit y: " << corners[i][1].y << std::endl;
            std::cout << "coin bas droit x: " << corners[i][2].x << " coin bas droit y: " << corners[i][2].y << std::endl;
            std::cout << "coin bas gauche x: " << corners[i][3].x << " coin bas gauche y: " << corners[i][3].y << std::endl;

            cv::Mat rvec, tvec;
            cv::solvePnP(aruco_real, corners[i], IntraMatrix, DistortionMatrix, rvec, tvec);

            geometry_msgs::msg::TransformStamped tf;
            tf.header.frame_id = "camera_link";
            tf.header.stamp = this->get_clock()->now();
            tf.child_frame_id = "Aruco_Tag_" + std::to_string(ids[i]);

            tf.transform.translation.x = tvec.at<double>(0);
            tf.transform.translation.y = tvec.at<double>(1);
            tf.transform.translation.z = tvec.at<double>(2);

            cv::Mat rmat;
            cv::Rodrigues(rvec, rmat);

            tf2::Matrix3x3 ros_mat(
                rmat.at<double>(0,0), rmat.at<double>(0,1), rmat.at<double>(0,2),
                rmat.at<double>(1,0), rmat.at<double>(1,1), rmat.at<double>(1,2),
                rmat.at<double>(2,0), rmat.at<double>(2,1), rmat.at<double>(2,2)
            );

            tf2::Quaternion quat;
            ros_mat.getRotation(quat);

            tf.transform.rotation.x = quat.x();
            tf.transform.rotation.y = quat.y();
            tf.transform.rotation.z = quat.z();
            tf.transform.rotation.w = quat.w();

            tf_broadcaster_->sendTransform(tf);
        }
    }

    cv::Mat IntraMatrix;
    cv::Mat DistortionMatrix;
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    double size;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PoseEstimator>());
    rclcpp::shutdown();
    return 0;
}