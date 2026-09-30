#include <vector>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>


    std::mutex capture;
    //creation d'un vecteur de pointeurs de filters car meme taille : un cannyfilter ou un gaussfilter nont pas meme taille !
    cv::Mat image_passation;


class Filter {

    public:

        virtual ~Filter() = default;
        virtual void applyFilter(cv::Mat& image)=0;
};

class GaussFilter : public Filter {

    public:

        GaussFilter(const cv::Size& s = cv::Size(5,5)) : size(s){

        }


        void applyFilter(cv::Mat& image) override {
            cv::GaussianBlur(image, image, size, 0);
            std::cout << "Application du filtre de Gauss" <<std::endl;
        }

    private :

         cv::Size size;


};

class CannyFilter : public Filter {

    public:

        CannyFilter(double low, double high) : low_thresh(low), high_tresh(high){

        }

        void applyFilter(cv::Mat& image) override {

            cv::Canny(image,image,low_thresh, high_tresh);
            std::cout << "Application du filtre de Canny" <<std::endl;
        }
    
    private:

        double low_thresh = 50;
        double high_tresh = 150;
};




void Capture_Image(){

        cv::Mat temporaire;
        cv::VideoCapture Capture_Image(0);

        while(1){

        Capture_Image >> temporaire;

        if (!temporaire.empty()){
            std::lock_guard lock(capture);
            image_passation = temporaire.clone();
        }
        }

}

void Apply_filter(){

        std::vector<std::unique_ptr<Filter>> filters;

        filters.push_back(std::make_unique<GaussFilter>(cv::Size(5,5)));
        filters.push_back(std::make_unique<CannyFilter>(50,150));

        while(1){

        cv::Mat Captured_image;
        
        //tentative dacces à une variable modifiée par un mutex donc on met un lock guard
        {
        std::lock_guard lock(capture);
        if (!image_passation.empty()){
            Captured_Image = image_passation.clone();
        }
        }

        //boucle sur les filtres ajoutés au vecteur = plug and play
        if (!Captured_image.empty()){
        
            for (const std::unique_ptr<Filter>& filter : filters) {
                filter->applyFilter(Captured_image);
            }

            cv::imshow("image filtrée",Captured_image);
            cv::waitKey(1);

        }
        }

}

int main(){
 
    std::thread CaptureCamera(Capture_Image);
    std::thread ApplyFilter(Apply_filter);

    CaptureCamera.join();
    ApplyFilter.join();


    return 0;
    }


    












       
