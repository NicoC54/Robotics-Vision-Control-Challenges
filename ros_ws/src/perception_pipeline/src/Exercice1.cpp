#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>


class  Filter{

    public:
        virtual void apply_filter(cv::Mat& image) = 0;
        
        virtual ~Filter() = default;

};

class GaussianFilter : public Filter {
    public:

    cv::Size Size;

        GaussianFilter(cv::Size Size){
            this->Size = Size;
        };

        void apply_filter(cv::Mat& image) override{
            cv::GaussianBlur(image,image,Size,0);
            std::cout << "Gaussian blur applied" << std::endl;
        }
};


class CannyFilter : public Filter {
    public:

       int threshold_down = 50;
       int treshold_up = 150;

        CannyFilter(int threshold_down, int treshold_up){
            this->threshold_down = threshold_down;
            this->treshold_up = treshold_up;
        }
    
        void apply_filter(cv::Mat& image) override {
            cv::Mat blur_image;
            cv::GaussianBlur(image,blur_image,cv::Size(5,5),0);
            cv::Canny(blur_image, image, threshold_down,treshold_up);
            std::cout << "Canny filter applied" << std::endl;

        }
};


std::vector<std::unique_ptr<Filter>> filter_list;


        

std::mutex mutex_capture;

std::thread thread_capture_image;
std::thread thread_apply_filter;

cv::Mat image_being_treated;

cv::VideoCapture cap(0);

cv::Mat image_temporaire;
cv::Mat image_captured;

void ThreadCapture(){
while (true){

    cap >> image_temporaire; // on part du principe que c'est sur le port 0 quon a un flux
    if (!image_temporaire.empty()){
        std::lock_guard lock(mutex_capture);
        image_captured = image_temporaire;
    }


}
}
void ThreadApplyFilter(){
    filter_list.push_back(std::make_unique<CannyFilter>(50, 150));
    filter_list.push_back(std::make_unique<GaussianFilter>(cv::Size(5,5)));

    while(true){
        {
        std::lock_guard lock(mutex_capture);
        if (!image_captured.empty()){
            //on bloque l'accès à un autre thread tant que on a pas sauvegardé l'image la plus récente capturée
            
            image_being_treated = image_captured.clone();
            }
    }
    if (!image_being_treated.empty()){
        for (auto& filter : filter_list){
            filter->apply_filter(image_being_treated);
            
        }
        cv::imshow("Flux Robotique", image_being_treated);
        cv::waitKey(1);
       }
}
}

int main(){

    std::thread thread_capture_image(ThreadCapture);
    std::thread thread_apply_filter(ThreadApplyFilter);

    thread_capture_image.join();
    thread_apply_filter.join();


}