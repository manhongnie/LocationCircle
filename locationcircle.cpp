#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

//pointx，pointy 起始指定点坐标
//w1,h1 起始方框宽 长
//intervalx，intervaly 每个小图的xy间隔
//numx，numy 每张整图的xy方向上小图的个数
//radiusk 去除范围外的噪点，针对不同的产品

//void ContoursRemoveNoise(Mat &gray, double pArea);
//void NaiveRemoveNoise(Mat &gray, double pNum);
void findpointposition(Mat &src, Mat &srck, int pointx, int pointy, int w1, int h1, int intervalx, int intervaly, int numx, int numy, int radiusk);
int main()
{
	Mat src = imread("D:\\img\\GEITU\\1.bmp");
	//3.bmp 690,810,320,310,372,370
	//2.bmp 820,825,320,310,372,367
	//1.bmp 695,785,320,310,372,367
	//4.bmp 805,850,320,310,372,367
	//5.bmp 675,820,320,320,372,370
	//6.bmp 790,750,320,320,372,370
	if (src.empty()) {
		printf("could not load image file...");
		return -1;
	}
	Mat srck;
	findpointposition(src, srck, 695, 785, 320, 310, 372, 367, 18, 10, 135);
	namedWindow("xianshi");
	imshow("xianshi", src);
	waitKey(0);
}
void findpointposition(Mat &src, Mat &srck, int pointx, int pointy, int w1, int h1, int intervalx, int intervaly, int numx, int numy, int radiusk)
{
	vector<Mat>arryk;
	cv::split(src, arryk);
	srck = arryk[1];
	Mat gray, binary;
	int height = src.rows;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Rect> rects;
	vector<Point2f>poins;
	Point points;
	points.x = pointx;
	points.y = pointy;
	for (size_t i = 0; i < numy; i++)
	{
		if ((points.y + h1) >(numy * intervaly + pointy + 1))
		{
			continue;
		}
		if (points.y < pointy)
		{
			continue;
		}
		for (size_t j = 0; j < numx; j++)
		{
			if ((points.x + w1) >(numx * intervalx + pointx + 1))
			{
				points.x = pointx;
			}
			if (points.x < pointx)
			{
				continue;
			}
			gray = srck(Rect(points.x, points.y, w1, h1));
			Mat se = getStructuringElement(MORPH_ELLIPSE, Size(5, 5), Point(-1, -1));   //MORPH_ELLIPSE
			Mat kk = getStructuringElement(MORPH_ELLIPSE, Size(1, 5), Point(-1, -1));
			adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 25, 10);  //ADAPTIVE_THRESH_GAUSSIAN_C
			//NaiveRemoveNoise(gray, 5);
			double pNum = 5;
			int mh, nh, nValue, nCount;
			int nColsk = gray.cols;
			int nRowsk = gray.rows;
			for (int iw = 0; iw < nRowsk; ++iw) {
				gray.at<uchar>(iw, 0) = 255;
				gray.at<uchar>(iw, nColsk - 1) = 255;
			}
			for (int jw = 0; jw < nColsk; ++jw) {
				gray.at<uchar>(0, jw) = 255;
				gray.at<uchar>(nRowsk - 1, jw) = 255;
			}  
			for (int iv = 1; iv < nRowsk; ++iv)
				for (int jv = 1; jv < nColsk; ++jv)
				{
					nValue = gray.at<uchar>(iv, jv);
					if (nValue == 0)     
					{
						nCount = 0;
						for (mh = iv - 1; mh <= iv + 1; ++mh)
							for (nh = jv - 1; nh <= jv + 1; ++nh)
							{
								if (gray.at<uchar>(mh, nh) == 255)
									nCount++;
							}
						if (nCount >= pNum)
							gray.at<uchar>(iv, jv) = 255;
					}
				}
			//ContoursRemoveNoise(gray, 10);
			double pArea = 10;
			int colorr = 1;
			int nRows = gray.rows;
			int nCols = gray.cols;

			for (int ix = 0; ix < nRows; ++ix)
				for (int jx = 0; jx < nCols; ++jx) {
					if (!gray.at<uchar>(ix, jx)) { 
						floodFill(gray, Point(jx, ix), Scalar(colorr));   //注意  point是(_x,_y)形式 所以注意反写行列
						colorr++;
					}
				}
			int ColorCount[255] = { 0 };

			for (int im = 0; im < nRows; ++im) {
				for (int jm = 0; jm < nCols; ++jm) {
					if (gray.at<uchar>(im, jm) != 255)
						ColorCount[gray.at<uchar>(im, jm)]++;
				}
			}
			for (int in = 0; in < nRows; ++in) {
				for (int jn = 0; jn < nCols; ++jn) {
					if (ColorCount[gray.at<uchar>(in, jn)] <= pArea)
						gray.at<uchar>(in, jn) = 255;
					else
						gray.at<uchar>(in, jn) = 0;
				}
			}
			morphologyEx(gray, binary, MORPH_GRADIENT, se);
			Mat mask;
			vector<vector<Point>> contoursd;
			vector<Vec4i> hierarchyd;
			cv::findContours(binary, contoursd, hierarchyd, RETR_LIST, CHAIN_APPROX_NONE);
			int max_indsx = 0;
			int max_isv = 0;
			for (size_t t = 1; t < contoursd.size(); t++)
			{
				if (contoursd[t].size()<100)
				{
					continue;
				}
				if (contoursd[max_indsx].size() < contoursd[t].size()) {
					max_indsx = t;
				}
				if (boundingRect(contoursd[max_isv]).width * boundingRect(contoursd[max_isv]).height < boundingRect(contoursd[t]).height * boundingRect(contoursd[t]).width)
				{
					max_isv = t;
				}
			}
			if (contourArea(contoursd[max_indsx]) < contourArea(contoursd[max_isv])) {
				max_indsx = max_isv;
			}
			RotatedRect rrts = fitEllipse(contoursd[max_indsx]);
			Point centersk = rrts.center;
			Mat dstr = Mat::zeros(binary.size(), binary.type());
			bitwise_not(dstr, dstr);
			circle(dstr, centersk, radiusk, Scalar(0, 0, 0), -1, 8, 0);
			subtract(binary, dstr, mask);
			cv::dilate(mask, mask, kk);
			cv::findContours(mask, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
			int max_in = 0;
			int max_is = 0;
			vector<double> areas;
			for (size_t tu = 1; tu < contours.size(); tu++) {
				if (contours[tu].size()<10)
				{
					continue;
				}
				double area = contourArea(contours[tu]);
				if (area < 200) {
					continue;
				}
				if (contours[max_in].size() < contours[tu].size()) {
					max_in = tu;
				}
				if (boundingRect(contours[max_is]).width * boundingRect(contours[max_is]).height <boundingRect(contours[tu]).height * boundingRect(contours[tu]).width)
				{
					max_is = tu;
				}
			}
			if (contourArea(contours[max_in]) < contourArea(contours[max_is])) {
				max_in = max_is;
			}
			RotatedRect rrt = fitEllipse(contours[max_in]);
			rrt.center.x = rrt.center.x + points.x;
			rrt.center.y = rrt.center.y + points.y;
			Point centers = rrt.center;
			ellipse(src, rrt, Scalar(0, 0, 255), 2, 8);
			points.x = points.x + intervalx;
		}
		points.y = points.y + intervaly;
	}
}
//void NaiveRemoveNoise(Mat &gray, double pNum) {
//	double pNum = 5;
//	int mh, nh, nValue, nCount;
//	int nColsk = gray.cols;
//	int nRowsk = gray.rows;
//
//	//set boundry to be white  
//	for (int iw = 0; iw < nRowsk; ++iw) {
//		gray.at<uchar>(iw, 0) = 255;
//		gray.at<uchar>(iw, nColsk - 1) = 255;
//	}
//	for (int jw = 0; jw < nColsk; ++jw) {
//		gray.at<uchar>(0, jw) = 255;
//		gray.at<uchar>(nRowsk - 1, jw) = 255;
//	}
//
//	//if the neighbor of a point is white but it is black, delete it  
//	for (int iv = 1; iv < nRowsk; ++iv)
//		for (int jv = 1; jv < nColsk; ++jv)
//		{
//			nValue = gray.at<uchar>(iv, jv);
//			if (nValue == 0)      //fine a black point
//			{
//				nCount = 0;
//				for (mh = iv - 1; mh <= iv + 1; ++mh)
//					for (nh = jv - 1; nh <= jv + 1; ++nh)
//					{
//						if (gray.at<uchar>(mh, nh) == 255)
//							nCount++;
//					}
//				if (nCount >= pNum)
//					gray.at<uchar>(iv, jv) = 255;
//			}
//		}
//}

//void ContoursRemoveNoise(Mat &gray, double pArea)
//{
//	double pArea = 10;
//	int colorr = 1;
//	int nRows = gray.rows;
//	int nCols = gray.cols;
//
//	for (int ix = 0; ix < nRows; ++ix)
//		for (int jx = 0; jx < nCols; ++jx) {
//			if (!gray.at<uchar>(ix, jx)) {
//				//FloodFill each point in connect area using different color  
//				floodFill(gray, Point(jx, ix), Scalar(colorr));   //注意  point是(_x,_y)形式 所以注意反写行列
//				colorr++;
//			}
//		}
//
//	int ColorCount[255] = { 0 };
//
//	for (int im = 0; im < nRows; ++im) {
//		for (int jm = 0; jm < nCols; ++jm) {
//			//caculate the area of each area  
//			if (gray.at<uchar>(im, jm) != 255)
//				ColorCount[gray.at<uchar>(im, jm)]++;
//		}
//	}
//
//	//get rid of noise point  
//	for (int in = 0; in < nRows; ++in) {
//		for (int jn = 0; jn < nCols; ++jn) {
//			if (ColorCount[gray.at<uchar>(in, jn)] <= pArea)
//				gray.at<uchar>(in, jn) = 255;
//			else
//				gray.at<uchar>(in, jn) = 0;
//		}
//	}
//}