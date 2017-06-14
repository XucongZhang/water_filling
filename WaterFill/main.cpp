#include "config.h"
#include "WaterFill.h"

void main()
{
	CWaterFill m_water;
	Mat waterImg(240, 320, CV_16UC1);
	Mat img_16bit(240,320, CV_16UC1);//for some median calculation
	Mat img_8bit(240, 320, CV_8UC1);//for some median calculation

	cv::Mat img = imread("C:\\HeadData-CBSR\\dataset1\\Dep\\dep-9.png", CV_LOAD_IMAGE_ANYDEPTH); //load the depth image

	int img_width = img.cols;
	int img_height = img.rows;
	cv::Mat show_img = cv::Mat::zeros(img_height, img_width, CV_8UC1);

	show_img = m_water.Sixth2Eight(img, 30); // need to change to be 8bit image to show
	cv::imshow("win", show_img);//show the original depth image, it is already after backgroud subtraction
	
	m_water.Initialise();
	// run the core function, fill the water into the depth map. The threshold 10 means:
	// every time in a position we drop a water, how much value we want to put on this pixel. It could ba a optimazed parameter.
	waterImg = m_water.WaterDrop(img, 10);
	
	img_16bit = m_water.WaterFilter(waterImg, 8000);//filter out the position that less than 8000 (each water means 4000, so here means less than two layers)
	img_8bit = m_water.Sixth2Eight(img_16bit, 30);// change to be 8 bit to show
	imshow("waterImg", img_8bit); // show the result

	cv::waitKey(0);
}