#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "cvaux.h"
#include <stdio.h>
#include <cv.h>  
#include<math.h>


#define IMG_SIZE_X 1280
#define IMG_SIZE_Y 720
 
using namespace std;
using namespace cv;

static int size1 = 60;
static int sizeLocation = 150;
static int IS_LOCATED = 0;
static int sumOfPixel = 0;
static int X0, X1, Y0, Y1;
static Point lastPoint1=(-1,-1), lastPoint2 = (-1, -1), lastPoint3 = (-1, -1);
int CH1, CH2;

void drawrReferenceLine(Mat image, Point Poi);
Point getLocationFromImage(Mat image);
Mat reBuiltImage(Mat image, Mat  backImage);
Mat reBuiltBackgroundImage(Mat backImage);
void drawFilledCircle(Mat img, Point center);
void drawLine(Mat img, Point start, Point end);
Point getLocationFromArea(Mat image, Point lastLocation);
Point reviseLocation(Point baseLocation);
Point trackROI(Mat image, Mat imageOutput, Point lastLocation);
void adjustARR();

void adjustARR() 
{
	double x, y;
	int size = 5;
	x = sumOfPixel;
	y = sizeLocation*sizeLocation * 4;
	while ((int(y / x) != size)&&(sumOfPixel!=0))
	{
		if (y / x > size)
		{
			sizeLocation--;
			y = sizeLocation*sizeLocation * 4;
			if (sizeLocation < 10)
				break;
		}
		else if (y / x < size)
		{
			{
				sizeLocation++;
				y = sizeLocation*sizeLocation * 4;
			}
		}
		
    }
  
}

Point trackROI(Mat image,Mat imageOutput,Point lastLocation)
{
	Point Poi;
	if (IS_LOCATED)
	{
		printf("%d %d\n", sumOfPixel, sizeLocation);
		adjustARR();
		Poi = getLocationFromArea(imageOutput, lastLocation);
		
//		Poi = reviseLocation(Poi);
		printf("1\n");
	
		
	}

	if (!IS_LOCATED)
	{
		Poi = getLocationFromImage(imageOutput);
	//	Poi = reviseLocation(Poi);
		printf("0");
	}
	drawrReferenceLine(image, Poi);
	drawFilledCircle(image, Point(IMG_SIZE_X/2, IMG_SIZE_Y/2));

	if (Poi.x > IMG_SIZE_X/2)
		CH1 = -1;
	else if (Poi.x < IMG_SIZE_X/2)
		CH1 = 1;
	else
		CH1 = 0;
	printf("CH1=%d   Poi.x=%d\n", CH1, Poi.x);
	drawLine(image, Poi, Point(Poi.x + CH1 * 20, Poi.y));

	if (Poi.y > IMG_SIZE_Y/2)
		CH2 = -1;
	else if (Poi.y< IMG_SIZE_Y/2)
		CH2 = 1;
	else
		CH2 = 0;
	printf("CH2=%d   Poi.y=%d\n", CH2, Poi.y);

	drawLine(image, Poi,Point(Poi.x, Poi.y+CH2*20));
	drawLine(image, Poi, Point(IMG_SIZE_X / 2, IMG_SIZE_Y / 2));

	imshow("imageOutput", imageOutput);
	imshow("imageORI", image);
	return(Poi);

}


/**************************************************************/
//函数名称：reBuiltImage
//函数功能：传入单帧图像，将本针图像去除背景并且
//          处理成高对比度的黑白图像输出
//传入参数：image,backImage
//返回参数：imageOutput （处理后的图）
//最后修改时间：2016年10月19日14：43                            
/**************************************************************/
Mat reBuiltImage(Mat image, Mat  backImage)
{

	Mat gray_image = Mat::zeros(image.size(), image.type());
	Mat imageOutput = Mat::zeros(image.size(), image.type());
	cvtColor(image, gray_image, CV_BGRA2GRAY);
	threshold(gray_image, imageOutput, 80, 255, THRESH_BINARY);

	absdiff(imageOutput, backImage, imageOutput);
	medianBlur(imageOutput, imageOutput, 5);
	return imageOutput;
	
}

/**************************************************************/
//函数名称：reBuiltBackgroundImage
//函数功能：对传入的背景图进行灰度化处理，二值化处理
//传入参数：backImage（背景图）
//返回参数：backImage2 （处理后的背景图）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Mat reBuiltBackgroundImage(Mat backImage)
{
	Mat backImageGRY = Mat::zeros(backImage.size(), backImage.type());
	Mat backImage2 = Mat::zeros(backImage.size(), backImage.type());
	cvtColor(backImage, backImageGRY, CV_BGRA2GRAY);
	//threshold(gray_image, imageOutput, 100, 255, THRESH_BINARY);
	threshold(backImageGRY, backImage2, 80, 255, THRESH_BINARY);
	return backImage2;
}



/**************************************************************/
//函数名称：reviseLocation
//函数功能：对传入的坐标进行滤波处理，减少
//          坐标点抖动
//传入参数：baseLocation（未经滤波的坐标）
//返回参数：Point（xx,yy） （滤波后的坐标值）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point reviseLocation(Point baseLocation)
{
	
	if (lastPoint1.x == -1)
	{
		lastPoint1.x = baseLocation.x;
		lastPoint1.y = baseLocation.y;
		return  lastPoint1;
	}
	else if ((lastPoint1.x != -1) && (lastPoint2.x == -1))
	{
		lastPoint2.x = baseLocation.x;
		lastPoint2.y = baseLocation.y;
		int xx, yy;
		xx = (lastPoint1.x + lastPoint2.x ) / 2;
		yy = (lastPoint1.y + lastPoint2.y ) / 2;
		return (Point(xx, yy));
	}

	else if ((lastPoint1.x != -1) && (lastPoint2.x != -1) && (lastPoint3.x == -1))
	{
		lastPoint3.x = baseLocation.x;
		lastPoint3.y = baseLocation.y;
		int xx, yy;
		xx = (lastPoint1.x + lastPoint2.x + lastPoint3.x) / 3;
		yy = (lastPoint1.y + lastPoint2.y + lastPoint3.y) / 3;
		return (Point(xx, yy));
	}
	else
	{
		int xx, yy;
		if (((abs(baseLocation.x - lastPoint3.x)) <= 100) && ((abs(baseLocation.y - lastPoint3.y)) <= 100))
		{
	
			lastPoint1.x = lastPoint2.x;
			lastPoint2.x = lastPoint3.x;
			lastPoint3.x = baseLocation.x;
			lastPoint1.y = lastPoint2.y;
			lastPoint2.y = lastPoint3.y;
			lastPoint3.y = baseLocation.y;
			xx = (lastPoint1.x + lastPoint2.x + lastPoint3.x) / 3;
			yy = (lastPoint1.y + lastPoint2.y + lastPoint3.y) / 3;
			return (Point(xx, yy));

		}
	}

}



/**************************************************************/
//函数名称：getLocationFromImage
//函数功能：在整个图像中搜索目标的位置 
//传入参数：image（二值化去背景的图）
//返回参数：Poi （查找到的坐标）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point getLocationFromImage (Mat image)
{
	Point Poi=(0,0),conpoi=(0,0);
	int i, j, x = 0, y = 0;
	int maxi=-1, maxj=-1,mini=9999,minj=9999;
	sumOfPixel = 0;
	for (i = 0; (i< IMG_SIZE_Y)&&(i+3<=IMG_SIZE_Y); i=i+3)
		for (j =0; (j < IMG_SIZE_X)&&(j+3<= IMG_SIZE_X); j=j+3)
		 {
			if (image.at<uchar>(i, j) == 255)
			{
				y = y + i;
				x = x + j;
				sumOfPixel++;
			}
		}
	if (sumOfPixel != 0)
	{
		Poi.x = x / sumOfPixel;
		Poi.y = y / sumOfPixel;
		IS_LOCATED = 1;
	
	}
	else
	{
		IS_LOCATED = 0;
	}
	 return Poi;
}


/**************************************************************/
//函数名称：getLocationFromImage
//函数功能：在追踪区域中搜索目标的位置 
//传入参数：image （二值化去背景的图） x0,x1,y0,y1(追踪区域坐标)
//返回参数：Poi （查找到的坐标）
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
Point getLocationFromArea(Mat image, Point lastLocation)
{
	Point Poi = (0, 0), conpoi = (0, 0);
	int i, j, x = 0, y = 0;
	sumOfPixel = 0;
	int x0, x1, y0, y1;
	if ((lastLocation.x - sizeLocation) >= 0) x0 = lastLocation.x - sizeLocation;
	else x0 = 0;
	if ((lastLocation.x + sizeLocation) <= IMG_SIZE_X) x1 = lastLocation.x + sizeLocation;
	else x1 = IMG_SIZE_X;
	if ((lastLocation.y - sizeLocation) >= 0) y0 = lastLocation.y - sizeLocation;
	else y0 = 0;
	if ((lastLocation.y + sizeLocation) <= IMG_SIZE_Y) y1 = lastLocation.y + sizeLocation;
	else y1 = IMG_SIZE_Y;


	for (i = y0; i< y1; i++ )
		for (j = x0; j < x1; j++ )
		{
			if (image.at<uchar>(i, j) == 255)
			{
				y = y + i;
				x = x + j;
				sumOfPixel++;
			}
		}
	if (sumOfPixel > 20)
	{
		Poi.x = x / sumOfPixel;
		Poi.y = y / sumOfPixel;
		IS_LOCATED = 1;

	}
	else
	{
		IS_LOCATED = 0;
	}
	return Poi;
}

/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一个正方形方框.以传入点为圆心，size为边长
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawrReferenceLine(Mat image,Point Poi)
{
	drawFilledCircle(image, Poi);
	int SIZE;
	if (IS_LOCATED)
		SIZE = sizeLocation;
	else
		SIZE = size1;

	X0 = Poi.x - SIZE;
	X1 = Poi.x + SIZE;
	Y0 = Poi.y - SIZE;
	Y1 = Poi.y + SIZE;
	drawLine(image, Point(X0, Y0), Point(X1, Y0));
	drawLine(image, Point(X0, Y0), Point(X0, Y1));
	drawLine(image, Point(X0, Y1), Point(X1, Y1));
	drawLine(image, Point(X1, Y0), Point(X1, Y1));
}
 
/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一个实心圆点
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawFilledCircle(Mat img, Point center)
{
	int thickness = -1;
	int lineType = 8;

	circle(img,
		center,
		5,
		Scalar(0, 0, 255),
		thickness,
		lineType);
}



/**************************************************************/
//函数名称：drawrReferenceLine
//函数功能：在图像上画一条线
//传入参数：image  Poi
//最后修改时间：2016年10月24日11：24                            
/**************************************************************/
void drawLine(Mat img, Point start , Point end)
{
	int thickness = 2;
	int lineType = 8;
	int colorR=0, colorG=0, colorB=0;
	if (IS_LOCATED)
	{
		colorG = 255;
		colorR = 0;
	}
	if (!IS_LOCATED)
	{
		colorG = 0;
		colorR = 255;
	}
	line(img,
		start,
		end,
		Scalar(colorB, colorG, colorR),
		thickness,
		lineType);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//


int main()
{


	int CH1, CH2;
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//载入视频
	VideoCapture capture(0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//定义中心点


	Mat backImg, image,tem, reImg;
	int nFrmNum = 0,count=0;
	Point lastLocation,location;
	namedWindow("imageOutput", CV_WINDOW_NORMAL);
	namedWindow("imageORI", CV_WINDOW_NORMAL);
	
	//while (pFrame = cvQueryFrame(pCapture))
	while(1)
	{
		Mat pFrame;
		Mat pFrame2;
		capture >> pFrame;
		nFrmNum++;

		if (nFrmNum <= 10)
		{		
			tem = pFrame;
			tem.copyTo(backImg);
			backImg = reBuiltBackgroundImage(backImg);
		}
		else 
		{
			image = pFrame;
			reImg =reBuiltImage(image, backImg);
			location=trackROI(image, reImg, lastLocation);


			lastLocation = location;
		}
		//if (waitKey(1) >= 0) break;
	}
	return 0;

}


