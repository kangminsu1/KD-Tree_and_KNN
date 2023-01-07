#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <pcl/io/pcd_io.h>
#include <pcl/common/common.h>
#include <opencv2/opencv.hpp>

#include "header_kdtree.h"

// for PCD SIZE SCALE CHANGE

// for sample.pcd
// #define PCD_SIZE_WIDTH 25
// #define PCD_SIZE_HEIGHT 15

// for CornerMap.pcd
#define PCD_SIZE_WIDTH 5
#define PCD_SIZE_HEIGHT 4


// Point array 생성하는 Class
class mypoint : public std::array<double, 2> {
    public:

        static const int DIM = 2;

        mypoint() {}
        mypoint(double x, double y){
            (*this)[0] = x;
            (*this)[1] = y;
        }

        operator cv::Point2d() const {
            return cv::Point2d((*this)[0], (*this)[1]);
        }
};

int main(int argc, char** argv){
    const int sample = argc > 1 ? std::stoi(argv[1]) : 0;
    srand(sample);

    //0이면 임의로 만들고, 1이면 pcd를 불러온다
    bool num;
    std::cout << "Choose Senario. 0=sample, 1=pcd.\n Choose: ";
    std::cin >> num;

    int width, height;
    std::vector<mypoint> points;

    if (!num){
        int npoints;
        // Random Point를 만들 Width, Height 선택
        std::cout << "Choose the Width & Height. ex) W&H: 500 500\n W&H: ";
        std::cin >> width >> height;
        std::cout << "Choose the Random Points. ex) Points: 100\n Points: ";
        std::cin >> npoints; // Random Point 개수 선책
        
        for(int i = 0; i < npoints; i++){
            const int x = rand() % width;
            const int y = rand() % height;
            points.push_back(mypoint(x, y));
        }
    }
    else{
        //Point Cloud Data 불러옴
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
        cloud = pcl::PointCloud<pcl::PointXYZ>::Ptr (new pcl::PointCloud<pcl::PointXYZ>);
        // pcl::io::loadPCDFile<pcl::PointXYZ> ("../pcd/sample.pcd", *cloud);
        pcl::io::loadPCDFile<pcl::PointXYZ> ("../pcd/CornerMap.pcd", *cloud);
        pcl::PointXYZ minPt, maxPt;
        pcl::getMinMax3D (*cloud, minPt, maxPt);
        
        width = (maxPt.x - minPt.x) * PCD_SIZE_WIDTH;
        height = (maxPt.y - minPt.y) * PCD_SIZE_HEIGHT;
        
        for(auto& pp: *cloud){
            points.push_back(mypoint((pp.x - minPt.x) * PCD_SIZE_WIDTH, (pp.y - minPt.y) * PCD_SIZE_HEIGHT));
        }
    }
    // Alignment Points
    kd::kdtree<mypoint> KDTREE(points);

    // Select Point for finding another Points
    int dx, dy;
    printf("Select Position for fine nearest point in width: %d, height: %d\n", width, height);
    printf("Example -> Point: 300 300\n Point: ");
    std::cin >> dx >> dy;
    const mypoint query(dx, dy);

    // Nearest Neigbor Search
    const int idx = KDTREE.nnsearch(query);


    // visualization---------------------------------------------------
    cv::Mat img = cv::Mat::zeros(cv::Size(width, height), CV_8UC3);
    
    //class operator function 추가해서 cv::circle 생성
    for (const auto& pt : points){
        cv::circle(img, cv::Point2d(pt), 2, cv::Scalar(0, 0, 255), -1);

        // if you want to check the point pos data, open the code
        // if(!num){
        //     std::string pos = "(" + std::to_string((int)pt[0]) + ", " + std::to_string((int)pt[1]) + ")";
        //     cv::putText(img, pos.c_str(), cv::Point(pt[0], pt[1]), 0, 0.15, cv::Scalar(255, 255, 0), 1, 1);
        // }
    }
    
    const cv::Mat I0 = img.clone();
    std::string mine = "[My Point]";
	cv::circle(I0, cv::Point2d(query), 3, cv::Scalar(0, 255, 0), -1);
    cv::putText(I0, mine.c_str(), cv::Point(query[0], query[1]), 3, 1.0, cv::Scalar(255, 0, 0), 1, 1);
	cv::circle(I0, cv::Point2d(points[idx]), 1, cv::Scalar(255, 255, 0), -1);
	cv::line(I0, cv::Point2d(query), cv::Point2d(points[idx]), cv::Scalar(255, 0, 255));

    // // k-nearest neigbors search
    int k;
    printf("Choose the K Points: ");
    std::cin >> k;

	const cv::Mat I1 = img.clone();
	const std::vector<int> knnIndices = KDTREE.knnsearch(query, k);
	for (int i : knnIndices)
	{
		cv::circle(I1, cv::Point2d(points[i]), 1, cv::Scalar(255, 255, 0), -1);
		cv::line(I1, cv::Point2d(query), cv::Point2d(points[i]), cv::Scalar(255, 0, 0));
	}

    cv::imshow("KD-TREE", I0);
	cv::imshow("K-nearest neigbors search", I1);
    cv::waitKey(0);

    // Nearest Neighbor

    return 0;
}