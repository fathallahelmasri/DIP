//============================================================================
// Name        : Dip5.cpp
// Author      : Ronny Haensch
// Version     : 2.0
// Copyright   : -
// Description : 
//============================================================================

#include "Dip5.h"

// uses structure tensor to define interest points (foerstner)
void Dip5::getInterestPoints(Mat& img, double sigma, vector<KeyPoint>& points){
	
	Mat FDKernel = createFstDevKernel(sigma);
	Mat resultsImg;
	Mat Gx, Gy;

	//Calculate directional gradients
	filter2D(img, Gx, -1, FDKernel);
	//imshow("resultsX", Gx);
	//imwrite("resultsX.png", Gx);

	filter2D(img, Gy, -1, FDKernel.t());
	//imshow("resultsY", Gy);
	//imwrite("resultsY.png", Gy);
	
	Mat Gxx = Gx.mul(Gx);
	Mat Gyy = Gy.mul(Gy);
	Mat Gxy = Gx.mul(Gy);

	//Average (Gaussian Window)
	GaussianBlur(Gxy, Gxy, cv::Size(3,3), 1);
	//imshow("resultsxy", Gxy);
	//imwrite("resultsxy.png", Gxy);

	//Trace of structure tensor
	Mat trace;
	add(Gxx, Gyy, trace);
	//imshow("structure tensor", trace);
	//imwrite("structure tensor.png", trace);

	//Determinant of structure tensor
	Mat de1, de2, determinant;
	de1 = Gxx.mul(Gyy);
	de2 = Gxy.mul(Gxy);
	determinant = de1 - de2;
	//imshow("determinant", determinant);
	//imwrite("determinant.png", determinant);

	//weight calculation
	Mat weight = determinant / trace;
	float meanW = mean(weight)[0];
	weight = nonMaxSuppression(weight);
	threshold(weight, weight, 0.5*meanW, 1, THRESH_TOZERO);
	//imshow("weight", weight);
	//imwrite("weight.png", weight);
	
	//isotropy calculation
	Mat iso;
	iso = 4 * determinant / trace.mul(trace);
	iso = nonMaxSuppression(iso);
	threshold(iso, iso, 0.5, 255, THRESH_TOZERO);
	//imshow("iso", iso);
	//imwrite("iso.png", iso);
		
	//keypoints found
	for (int i = 0; i < iso.rows; i++){
		for (int j = 0; j < iso.cols; j++){
			if (iso.at<float>(i, j)!=0&&weight.at<float>(i,j)!=0){
				points.push_back(KeyPoint(j, i, 3));
			}
		}
	}

}

// creates kernel representing fst derivative of a Gaussian kernel in x-direction
/*
sigma	standard deviation of the Gaussian kernel
return	the calculated kernel
*/
Mat Dip5::createFstDevKernel(double sigma){

	int kSize = round(sigma * 3) * 2 - 1;

	Mat gaussKernel = getGaussianKernel(kSize, sigma, CV_32FC1);
	gaussKernel = gaussKernel * gaussKernel.t();
	
	Mat Gx = Mat::zeros(kSize, kSize, CV_32FC1);

	for (int i = 0; i < kSize; i++){
		for (int j = 0; j < kSize; j++){		
			Gx.at<float>(i, j) = (-(i-1) / (sigma*sigma))*gaussKernel.at<float>(i, j);
		}
	}	
	return Gx;

}

/* *****************************
  GIVEN FUNCTIONS
***************************** */

// function calls processing function
/*
in		:  input image
points	:	detected keypoints
*/
void Dip5::run(Mat& in, vector<KeyPoint>& points){
   this->getInterestPoints(in, this->sigma, points);
}

// non-maxima suppression
// if any of the pixel at the 4-neighborhood is greater than current pixel, set it to zero
Mat Dip5::nonMaxSuppression(Mat& img){

	Mat out = img.clone();
	
	for(int x=1; x<out.cols-1; x++){
		for(int y=1; y<out.rows-1; y++){
			if ( img.at<float>(y-1, x) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x-1) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>(y, x+1) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
			if ( img.at<float>( y+1, x) >= img.at<float>(y, x) ){
				out.at<float>(y, x) = 0;
				continue;
			}
		}
	}
	return out;
}

// Function displays image (after proper normalization)
/*
win   :  Window name
img   :  Image that shall be displayed
cut   :  whether to cut or scale values outside of [0,255] range
*/
void Dip5::showImage(Mat& img, const char* win, int wait, bool show, bool save){
  
    Mat aux = img.clone();

    // scale and convert
    if (img.channels() == 1)
		normalize(aux, aux, 0, 255, CV_MINMAX);
		aux.convertTo(aux, CV_8UC1);
    // show
    if (show){
      imshow( win, aux);
      waitKey(wait);
    }
    // save
    if (save)
      imwrite( (string(win)+string(".png")).c_str(), aux);
}
