#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>


class Filter {
    public:
        virtual void apply_filter(cv::Mat& image) =0;

        virtual ~Filter() = default;
};

class GaussianFilter : public Filter{
    public :
        cv::Size Size;

        GaussianFilter(cv::Size Size){
            this->Size = Size;
        }
        void apply_filter(cv::Mat& image) override {
            cv::GaussianBlur(image,image,Size,0);
            std::cout << "Applied Gaussian Filter" << std::endl;
        }

};

class CannyFilter : public Filter{
    public:

        int treshold_down = 50;
        int treshold_up = 150;

        CannyFilter(const int& treshold_down, const int& threshold_up){
            this->treshold_down = treshold_down;
            this->treshold_up = treshold_up;
        }

        void apply_filter(cv::Mat& image) override {
            cv::Canny(image, image, treshold_down, treshold_up);
            std::cout << "Applied Gaussian Filter" << std::endl;
    }
};

std::mutex capture_mutex;

cv::VideoCapture cap(0);


cv::Mat image_being_read;
cv::Mat image_passation;
cv::Mat image_being_treated;
std::vector<std::unique_ptr<Filter>> filter_list;


void thread_read_image(){

    while(1){

        cap >> image_being_read;

        if (!image_being_read.empty()){
            std::lock_guard lock(capture_mutex);
            cv::swap(image_passation,image_being_read);
        }
    }
}

void thread_apply_filter(){

    filter_list.push_back(std::make_unique<GaussianFilter>(cv::Size(5,5)));
    filter_list.push_back(std::make_unique<CannyFilter>(50,150));

    while(1){
        bool newdata = false;
        {
        std::lock_guard lock(capture_mutex); 
        if(!image_passation.empty()){
            newdata = true;
            cv::swap(image_being_treated,image_passation);
            }
        }
        if((!image_being_treated.empty()) && newdata == true){

        for (auto& filter : filter_list){
            filter->apply_filter(image_being_treated);
        }
        cv::imshow("image traitée", image_being_treated);
        }
        cv::waitKey(1);

    }
}

int main(){
    std::thread Thread_lecture(thread_read_image);
    std::thread Thread_apply_filter(thread_apply_filter);
    Thread_lecture.join();
    Thread_apply_filter.join();
}
