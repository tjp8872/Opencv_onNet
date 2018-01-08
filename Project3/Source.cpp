#pragma once
#include <stdio.h>     
#include <stdlib.h>
#include <iostream>
#include <sstream>  
#include <fstream>
#include<vector>
#include <WS2tcpip.h>
#include<opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>
#include <Windows.h>
#include "libxl.h"



#include <windows.h>
#include <winsock.h>
#include <string.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#define PORT 1111
#define BACKLOG 10
#define TRUE 1



const int MIN_CONTOUR_AREA2 = 2000;
const int MIN_CONTOUR_AREA = 250;
const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;
const int MIN_z = 300000;
const int MIN_x = 5800;
static int h = 0;
static 	std::string str;

typedef struct {

	std::string id = "";
	int socre = 0;
	cv::Mat pic;
	static::std::string ans;

}Student_T;


class ContourWithData {
public:
	// member variables ///////////////////////////////////////////////////////////////////////////
	std::vector<cv::Point> ptContour;           // contour
	cv::Rect boundingRect;                      // bounding rect for contour
	float fltArea;                              // area of contour

												///////////////////////////////////////////////////////////////////////////////////////////////
	bool checkIfContourIsValid() {                              // obviously in a production grade program
		if (fltArea < MIN_CONTOUR_AREA) return false;           // we would have a much more robust function for 
		return true;                                            // identifying if a contour is valid !!
	}

	bool checkIfContourIsValid2() {                              // 字
		if (fltArea > MIN_CONTOUR_AREA2) return false;           // we would have a much more robust function for 
		return true;                                            // identifying if a contour is valid !!
	}

	bool checkIfContourIsValid4() {                              // 小框
		if (fltArea > MIN_x) return false;           // we would have a much more robust function for 
		return true;                                            // identifying if a contour is valid !!
	}

	bool checkIfContourIsValid3() {                              // 大框
		if (fltArea > MIN_z) return false;           // we would have a much more robust function for 
		return true;                                            // identifying if a contour is valid !!
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	static bool sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight) {      // this function allows us to sort
		return(cwdLeft.boundingRect.x < cwdRight.boundingRect.x);                                                   // the contours from left to right
	}

};



static std::string a(cv::Mat &input) {



	cv::Mat matClassificationInts;      // we will read the classification numbers into this variable as though it is a vector

	cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);        // open the classifications file



	fsClassifications["classifications"] >> matClassificationInts;      // read classifications section into Mat classifications variable
	fsClassifications.release();                                        // close the classifications file

																		// read in training images ////////////////////////////////////////////////////////////

	cv::Mat matTrainingImagesAsFlattenedFloats;         // we will read multiple images into this single image variable as though it is a vector

	cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);          // open the training images file



	fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;           // read images section into Mat training images variable
	fsTrainingImages.release();                                                 // close the traning images file

																				// train //////////////////////////////////////////////////////////////////////////////

	cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());            // instantiate the KNN object

																				// finally we get to the call to train, note that both parameters have to be of type Mat (a single Mat)
																				// even though in reality they are multiple images / numbers
	kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);



	cv::Mat matGrayscale;
	cv::Mat matBlurred;
	cv::Mat matThresh;
	cv::Mat matThreshCopy;
	cv::cvtColor(input, matGrayscale, CV_BGR2GRAY);         // convert to grayscale

															// blur
	cv::GaussianBlur(matGrayscale,              // input image
		matBlurred,                // output image
		cv::Size(5, 5),            // smoothing window width and height in pixels
		0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

								   // filter image from grayscale to black and white
	cv::adaptiveThreshold(matBlurred,                           // input image
		matThresh,                            // output image
		255,                                  // make pixels that pass the threshold full white
		cv::ADAPTIVE_THRESH_GAUSSIAN_C,       // use gaussian rather than mean, seems to give better results
		cv::THRESH_BINARY_INV,                // invert so foreground will be white, background will be black
		11,                                   // size of a pixel neighborhood used to calculate threshold value
		2);                                   // constant subtracted from the mean or weighted mean


	std::vector<std::vector<cv::Point> > ptContours;        // declare a vector for the contours
	std::vector<cv::Vec4i> v4iHierarchy;
	std::vector<ContourWithData> allContoursWithData;
	std::vector<ContourWithData> validContoursWithData;
	matThreshCopy = matThresh.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image
	cv::findContours(matThreshCopy,          // input image, make sure to use a copy since the function will modify this image in the course of finding contours
		ptContours,                             // output contours
		v4iHierarchy,                           // output hierarchy
		cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
		cv::CHAIN_APPROX_SIMPLE);



	//ROI

	for (int i = 0; i < ptContours.size(); i++) {               // for each contour
		ContourWithData contourWithData;                                                    // instantiate a contour with data object
		contourWithData.ptContour = ptContours[i];                                          // assign contour to contour with data
		contourWithData.boundingRect = cv::boundingRect(contourWithData.ptContour);         // get the bounding rect
		contourWithData.fltArea = cv::contourArea(contourWithData.ptContour);               // calculate the contour area
		allContoursWithData.push_back(contourWithData);                                     // add contour with data object to list of all contours with data
	}

	for (int i = 0; i < allContoursWithData.size(); i++) {                      // for all contours
		if (allContoursWithData[i].checkIfContourIsValid() && allContoursWithData[i].checkIfContourIsValid4()) {                   // check if valid
			validContoursWithData.push_back(allContoursWithData[i]);            // if so, append to valid contour list
		}
	}
	// sort contours from left to right
	std::sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByBoundingRectXPosition);

	std::string strFinalString;         // declare final string, this will have the final number sequence by the end of the program

	for (int i = 0; i < validContoursWithData.size(); i++) {            // for each contour

																		// draw a green rect around the current char
		cv::rectangle(input,                            // draw rectangle on original image
			validContoursWithData[i].boundingRect,        // rect to draw
			cv::Scalar(0, 0, 0),                        // green
			8);                                           // thickness




		cv::Mat matROI = matThresh(validContoursWithData[i].boundingRect);          // get ROI image of bounding rect

		cv::Mat matROIResized;
		cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     // resize image, this will be more consistent for recognition and storage




		cv::Mat matROIFloat;
		matROIResized.convertTo(matROIFloat, CV_32FC1);             // convert Mat to float, necessary for call to find_nearest

		cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);

		cv::Mat matCurrentChar(0, 0, CV_32F);

		kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     // finally we can call find_nearest !!!

		float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);

		
		strFinalString = strFinalString + char(float(fltCurrentChar));        // append current char to full string



	}



	//第三段去框後圈
	cv::cvtColor(input, matGrayscale, CV_BGR2GRAY);         // convert to grayscale

															// blur
	cv::GaussianBlur(matGrayscale,              // input image
		matThresh,                // output image
		cv::Size(5, 5),            // smoothing window width and height in pixels
		0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

								   // filter image from grayscale to black and white

	matThreshCopy = matThresh.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image

	std::vector<std::vector<cv::Point> > ptContours2;        // declare a vector for the contours
	std::vector<cv::Vec4i> v4iHierarchy2;
	std::vector<ContourWithData> allContoursWithData2;           // declare empty vectors,
	std::vector<ContourWithData> validContoursWithData2;

	//cv::imshow("madddddt22ROI", matThresh);
	cv::findContours(matThreshCopy,          // input image, make sure to use a copy since the function will modify this image in the course of finding contours
		ptContours2,                             // output contours
		v4iHierarchy2,                           // output hierarchy
		cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
		cv::CHAIN_APPROX_SIMPLE);



	//ROI

	for (int i = 0; i < ptContours2.size(); i++) {               // for each contour
		ContourWithData contourWithData2;                                                    // instantiate a contour with data object
		contourWithData2.ptContour = ptContours2[i];                                          // assign contour to contour with data
		contourWithData2.boundingRect = cv::boundingRect(contourWithData2.ptContour);         // get the bounding rect
		contourWithData2.fltArea = cv::contourArea(contourWithData2.ptContour);               // calculate the contour area
		allContoursWithData2.push_back(contourWithData2);                                     // add contour with data object to list of all contours with data
	}

	for (int i = 0; i < allContoursWithData2.size(); i++) {                      // for all contours
		if (allContoursWithData2[i].checkIfContourIsValid() && allContoursWithData2[i].checkIfContourIsValid2()) {                   // check if valid
			validContoursWithData2.push_back(allContoursWithData2[i]);            // if so, append to valid contour list
		}
	}
	// sort contours from left to right
	std::sort(validContoursWithData2.begin(), validContoursWithData2.end(), ContourWithData::sortByBoundingRectXPosition);

	// declare final string, this will have the final number sequence by the end of the program

	for (int i = 0; i < validContoursWithData2.size(); i++) {            // for each contour

																		 // draw a green rect around the current char
		cv::rectangle(input,                            // draw rectangle on original image
			validContoursWithData2[i].boundingRect,        // rect to draw
			cv::Scalar(0, 255, 0),                        // green
			2);                                           // thickness




		cv::Mat matROI = matThresh(validContoursWithData2[i].boundingRect);          // get ROI image of bounding rect

		cv::Mat matROIResized;
		cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     // resize image, this will be more consistent for recognition and storage
																									//cv::imshow("matROI", matROI);                               // show ROI image for reference

		cv::Mat matROIFloat;
		matROIResized.convertTo(matROIFloat, CV_32FC1);             // convert Mat to float, necessary for call to find_nearest

		cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);

		cv::Mat matCurrentChar(0, 0, CV_32F);

		kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     // finally we can call find_nearest !!!

		float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);

		
			

		str = str + char(float(fltCurrentChar));        // append current char to full string



	}
	std::string s = std::to_string(h);
//	cv::imshow(s, input);
	h++;
	return str;
}
using namespace System::IO;
using namespace System;
using namespace System::Collections;
using namespace std;
using namespace cv;
using namespace System::Runtime::InteropServices;
using namespace libxl;
using namespace System;

using namespace System::Collections;



void main()
{


	

	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cout << "Can't Initialize winsock! Quitting" << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cout << "Can't create a socket! Quitting" << endl;
		return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(1111);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	printf("server bind!\n");
	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Create the master file descriptor set and zero it
	fd_set master;
	FD_ZERO(&master);

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	FD_SET(listening, &master);

	// this will be changed by the \quit command (see below, bonus not in video!)
	bool running = true;

	while (running)
	{
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == listening)
			{
				cout << "accept success" << endl;
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				





				char buf[4096];

				string num;
				string id;
				string mode;
				string lon;
				const int result = recv(client, buf, sizeof(buf) + 1, 0);
				
					// use length (in bytes) returned by 'recv()'
					// since buffer is not null terminated.
					string s(buf, result);
					Sleep(1000);
					// 's' is in UTF-8 no converstion to wide strings
					// should be necessary.
				//	cout << "message: '" << s << "'." << endl;

					string str2("@");
					size_t found, nfound, n2found, n3found, n4found,f;


					//found = @位置
					found = s.find(str2);
					//nfound = 第二個@位置
					nfound = s.find(str2, found + 1);
					n2found = s.find(str2, nfound + 1);
					n3found = s.find(str2, n2found + 1);
					n4found = s.find(str2, n3found + 1);
					num = num.assign(s, found + 1, nfound - found - 1);
					id = id.assign(s, nfound + 1, n2found - nfound - 1);
					mode = mode.assign(s, n2found + 1, n3found - n2found - 1);
					lon = lon.assign(s, n3found + 1, n4found - n3found - 1);
					cout <<"Number: "<<num<<", ID: "<<id<<", Mode: "<< mode <<", Size: "<< lon <<" Bytes"<< endl;

		
				
				int z_i = std::stoi(lon); //input for socket
				int FileCounter = 0;
				bool flg = true;
				char * fileComplete;
				char * filesizeBuffer;

				FILE *temp;

				int iResult;
				int size = z_i  ;
				int receiveBuffer = 0;
				int desiredRecBuffer =   36*10240 ;
				int aa = 0;
				//int desiredRecBuffer = DEFAULT_BUFLEN ;
				fileComplete = (char*)malloc(sizeof(char)* size);
				while (desiredRecBuffer > 0)
				{
					iResult = recv(client, fileComplete + receiveBuffer, desiredRecBuffer, 0);
					//iResult = recv( ClientSocket, fileComplete + receiveBuffer , fileSize , 0 );
					
					if (iResult <  1)
					{
						//printf("Reveive Buffer Error  %d \n", WSAGetLastError());
					
						
					}
					else
					{
						receiveBuffer += iResult;
						desiredRecBuffer = size - receiveBuffer;
					//	printf("Reveived Data size :  %d \n", iResult);
						aa  += iResult;
					//	cout << aa <<"   "<< desiredRecBuffer << endl;
					}
					
				
				}
				
				std::string z1 = num;
				char const *num_c = z1.c_str();

				char filenamezz[256];
				strcpy(filenamezz, "img_");
				strcat(filenamezz, num_c);
				strcat(filenamezz, ".jpg");

				cout << "image get"<< endl;
				if (mode == "Score_Mode") {

					FILE *File = fopen(filenamezz, "wb");
					fwrite(fileComplete, 1, size, File);
					free(fileComplete);
					fclose(File);


				}

				else {

					FILE *File = fopen("sim.png", "wb");
					fwrite(fileComplete, 1, size, File);
					free(fileComplete);
					fclose(File);
				}
				

				
				//opencv start



				std::vector<ContourWithData> allContoursWithData;           // declare empty vectors,
				std::vector<ContourWithData> validContoursWithData;         // we will fill these shortly

																			// read in training classifications ///////////////////////////////////////////////////

				cv::Mat matClassificationInts;      // we will read the classification numbers into this variable as though it is a vector

				cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);        // open the classifications file



				fsClassifications["classifications"] >> matClassificationInts;      // read classifications section into Mat classifications variable
				fsClassifications.release();                                        // close the classifications file

																					// read in training images ////////////////////////////////////////////////////////////

				cv::Mat matTrainingImagesAsFlattenedFloats;         // we will read multiple images into this single image variable as though it is a vector

				cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);          // open the training images file



				fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;           // read images section into Mat training images variable
				fsTrainingImages.release();                                                 // close the traning images file

																							// train //////////////////////////////////////////////////////////////////////////////

				cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());            // instantiate the KNN object

																							// finally we get to the call to train, note that both parameters have to be of type Mat (a single Mat)
																							// even though in reality they are multiple images / numbers
				kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);

				// test ///////////////////////////////////////////////////////////////////////////////




				if (mode == "Score_Mode") {


					Book* book = xlCreateBook();
					if (book)
					{
						if (book->load(L"Student.xls"))
						{

						}
						else {
							Sheet* sheet = book->addSheet(L"Sheet1");
							if (sheet)
							{
								sheet->writeStr(2, 1, L"座號");
								sheet->writeStr(2, 2, L"學號");
								sheet->writeStr(2, 3, L"分數");
								sheet->writeStr(2, 4, L"答案");

								//座號
								for (int i = 0; i < 51; i++) {
									sheet->writeNum(i + 3, 1, i);
								}
								sheet->writeStr(3, 2, L"Teacher");
								sheet->writeStr(3, 3, L"--");
							}

							book->save(L"Student.xls");


						}

						book->release();
					}


				


					Student_T student[50];
					int num_i = std::stoi(num); //input for socket
					cv::Mat r = cv::imread(filenamezz, CV_LOAD_IMAGE_GRAYSCALE);


					//student[num_i].pic = r; //input for socket

					cv::Mat remat;
					if (r.cols > 1250)
						cv::resize(r, remat, cv::Size(1250, r.rows * ((float)1250 / r.cols)));
					else
						remat = r.clone();
					student[num_i].id = id;






					cv::Mat matGrayscale;           //
					cv::Mat matBlurred;             // declare more image variables
					cv::Mat matThresh;              //
					cv::Mat matThreshCopy;          //


					threshold(remat, matGrayscale, 120, 255, THRESH_BINARY);       // convert to grayscale

																				   // blur
					cv::GaussianBlur(matGrayscale,              // input image
						matBlurred,                // output image
						cv::Size(5, 5),            // smoothing window width and height in pixels
						0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

												   // filter image from grayscale to black and white
					cv::adaptiveThreshold(matBlurred,                           // input image
						matThresh,                            // output image
						255,                                  // make pixels that pass the threshold full white
						cv::ADAPTIVE_THRESH_GAUSSIAN_C,       // use gaussian rather than mean, seems to give better results
						cv::THRESH_BINARY_INV,                // invert so foreground will be white, background will be black
						11,                                   // size of a pixel neighborhood used to calculate threshold value
						2);                                   // constant subtracted from the mean or weighted mean

					matThreshCopy = matThresh.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image

					std::vector<std::vector<cv::Point> > ptContours3;        // declare a vector for the contours
					std::vector<cv::Vec4i> v4iHierarchy3;                    // declare a vector for the hierarchy (we won't use this in this program but this may be helpful for reference)

					cv::findContours(matThreshCopy,             // input image, make sure to use a copy since the function will modify this image in the course of finding contours
						ptContours3,                             // output contours
						v4iHierarchy3,                           // output hierarchy
						cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
						cv::CHAIN_APPROX_SIMPLE);               // compress horizontal, vertical, and diagonal segments and leave only their end points



																// 擷取答案長框
					cv::Mat m;

					for (int i = 0; i < ptContours3.size(); i++) {
						if (cv::contourArea(ptContours3[i]) > 80000) {
							cv::Rect boundingRect2 = cv::boundingRect(ptContours3[i]);
							m = matThresh(boundingRect2);


							// 把多餘的點存進去了

						}

					}





					//算角度
					IplImage* src;
					src = &IplImage(m);

					IplImage* dst = cvCreateImage(cvGetSize(src), 8, 1);
					IplImage* color_dst = cvCreateImage(cvGetSize(src), 8, 3);
					CvMemStorage* storage = cvCreateMemStorage(0);
					CvSeq* lines = 0;
					int i;
					IplImage* src1 = cvCreateImage(cvSize(src->width, src->height), IPL_DEPTH_8U, 1);

					cvCopy(src, src1);
					cvCanny(src1, dst, 50, 200, 3);

					cvCvtColor(dst, color_dst, CV_GRAY2BGR);



					lines = cvHoughLines2(dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 10, 30, 10);
					int dis = 0;
					int max = 0;
					int j = 0;
					CvPoint* line;
					CvPoint pointOne;
					CvPoint pointTwo;
					int x[1003] = { 0 };
					for (i = 0; i < lines->total; i++)
					{
						line = (CvPoint*)cvGetSeqElem(lines, i);

						dis = (line[1].y - line[0].y)*(line[1].y - line[0].y) + (line[1].x - line[0].x)*(line[1].x - line[0].x);

						x[4 * i] = line[0].x;
						x[4 * i + 1] = line[0].y;
						x[4 * i + 2] = line[1].x;
						x[4 * i + 3] = line[1].y;
						if (dis>max)
						{
							max = dis;
							j = i;
						}
						// cvLine( color_dst, line[0], line[1], CV_RGB(255,0,0), 3, 8 );
					}
					pointOne.x = x[4 * j];
					pointOne.y = x[4 * j + 1];
					pointTwo.x = x[4 * j + 2];
					pointTwo.y = x[4 * j + 3];
					cvLine(color_dst, pointOne, pointTwo, CV_RGB(255, 0, 0), 3, 8);    //画出最长的直线


					double Angle = 0.0;

					Angle = atan2(fabs(pointTwo.y - pointOne.y), fabs(pointTwo.x - pointOne.x));   //得到最长直线与水平夹角

					if (pointTwo.x>pointOne.x && pointTwo.y>pointOne.y)
					{
						Angle = -Angle;
					}

					Angle = Angle * 180 / CV_PI;



					//bug
					//轉角度
					//cv::Mat m = cv::cvarrToMat(src);


					Mat dst2 = Mat::zeros(m.rows, m.cols, m.type());

					//選定幾何轉換前後相對的三個點
					Point2f srcTri[3];
					srcTri[0] = Point2f(0, 0);
					srcTri[1] = Point2f(m.cols - 1, 0);
					srcTri[2] = Point2f(0, m.rows - 1);

					Point2f dstTri[3];
					dstTri[0] = Point2f(0, m.rows*0.3);
					dstTri[1] = Point2f(m.cols*0.8, 0);
					dstTri[2] = Point2f(m.cols*0.1, m.rows*0.9);

					//設定旋轉中心、旋轉角度和縮放倍率
					cv::Point center = cv::Point(dst2.cols / 2, dst2.rows / 2);
					double angle = -Angle;
					double scale = 1;

					Mat rot_mat = getRotationMatrix2D(center, angle, scale);
					warpAffine(m, dst2, rot_mat, dst2.size());

					//imshow("Affine_2", dst2);



					Mat imgP = dst2;
					int C = imgP.cols;
					int R = imgP.rows;
					int aP = 10000000;
					int bP = 10000000;
					int cP = 10000000;
					int dP = 10000000;
					int ax, ax2, ax3, ax4;
					int ay, ay2, ay3, ay4;



					Mat bwimg = imgP > 127;
					vector<vector<cv::Point> > contours;

					findContours(bwimg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

					for (unsigned int i = 0; i<contours.size(); i++) {
						approxPolyDP(Mat(contours[i]), contours[i], 10, true);


						for (unsigned int j = 0; j<contours[i].size(); j++)
						{
							//cout << "Point(x,y)=" << contours[i][j].x << "," << contours[i][j].y << endl;
							//circle(imgP, contours[i][j], 3, Scalar(255, 255, 255), FILLED, LINE_AA);

							int d = sqrt(pow(contours[i][j].x, 2) + pow(contours[i][j].y, 2));
							if (d < aP &&  contours[i][j].x != 0) {
								aP = d;
								ax = contours[i][j].x;
								ay = contours[i][j].y;

							}

							int d2 = sqrt(pow(C - contours[i][j].x, 2) + pow(contours[i][j].y, 2));
							if (d2 < bP &&  contours[i][j].x != 0 && contours[i][j].y != 0) {
								bP = d2;
								ax2 = contours[i][j].x;
								ay2 = contours[i][j].y;

							}

							int d3 = sqrt(pow(contours[i][j].x, 2) + pow(R - contours[i][j].y, 2));
							if (d3 < cP &&  contours[i][j].x != 0 && contours[i][j].y != 0) {
								cP = d3;
								ax3 = contours[i][j].x;
								ay3 = contours[i][j].y;

							}

							int d4 = sqrt(pow(C - contours[i][j].x, 2) + pow(R - contours[i][j].y, 2));
							if (d4 < dP &&  contours[i][j].x != 0 && contours[i][j].y != 0) {
								dP = d4;
								ax4 = contours[i][j].x;
								ay4 = contours[i][j].y;

							}


						}

					}
				

					Mat tranP;
					cv::Point2f pts1[] = { cv::Point2f(ax, ay),cv::Point2f(ax2, ay2),cv::Point2f(ax3, ay3),cv::Point2f(ax4, ay4) };
					cv::Point2f pts2[] = { cv::Point2f(0,0),cv::Point2f(C,0),cv::Point2f(0,R),cv::Point2f(C,R) };
					// 透視變換行列計算
					cv::Mat perspective_matrix = cv::getPerspectiveTransform(pts1, pts2);
					cv::Mat dst_img;
					// 變換
					cv::warpPerspective(imgP, tranP, perspective_matrix, imgP.size(), cv::INTER_LINEAR);




				







			









					IplImage* pImg2;
					pImg2 = &IplImage(tranP);





					int nWidth = pImg2->width;

					int nHeight = pImg2->height;

					int nChannels = pImg2->nChannels;

					int nStep = pImg2->widthStep;

					for (int i = 0; i<nHeight; i++)

						for (int j = 0; j<nWidth; j++)

							for (int k = 0; k<nChannels; k++)

							{

								pImg2->imageData[i*nStep + j*nChannels + k] = 255 - pImg2->imageData[i*nStep + j*nChannels + k];

							}


					cv::Mat white_i = cv::cvarrToMat(pImg2);





				

					char filename[256];
					strcpy(filename, "stu_");
					strcat(filename, num_c);
					strcat(filename, ".png");

					imwrite(filename, white_i);
					/*
					std::vector<int> quality;
					quality.push_back(CV_IMWRITE_JPEG_QUALITY);
					quality.push_back(90);
					imwrite(filename, white_i, quality);
					*/
					Mat h1 = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);





					cv::Mat matGrayscales;

					threshold(h1, matGrayscales, 120, 255, THRESH_BINARY);

					cv::Mat matBlurreds;             // declare more image variables
					cv::Mat matThreshs;              //
					cv::Mat matThreshCopys;          //

					cv::GaussianBlur(matGrayscales,
						matBlurreds,                // output image
						cv::Size(5, 5),            // smoothing window width and height in pixels
						0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

												   // filter image from grayscale to black and white
					cv::adaptiveThreshold(matBlurreds,                           // input image
						matThreshs,                            // output image
						255,                                  // make pixels that pass the threshold full white
						cv::ADAPTIVE_THRESH_GAUSSIAN_C,       // use gaussian rather than mean, seems to give better results
						cv::THRESH_BINARY_INV,                // invert so foreground will be white, background will be black
						11,                                   // size of a pixel neighborhood used to calculate threshold value
						2);                                   // constant subtracted from the mean or weighted mean

					matThreshCopys = matThreshs.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image

					std::vector<std::vector<cv::Point> > ptContourss;        // declare a vector for the contours
					std::vector<cv::Vec4i> v4iHierarchys;                    // declare a vector for the hierarchy (we won't use this in this program but this may be helpful for reference)

					cv::findContours(matThreshCopys,             // input image, make sure to use a copy since the function will modify this image in the course of finding contours
						ptContourss,                             // output contours
						v4iHierarchys,                           // output hierarchy
						cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
						cv::CHAIN_APPROX_SIMPLE);               // compress horizontal, vertical, and diagonal segments and leave only their end points



																// 擷取答案長框
					cv::Mat dt2;

					for (int i = 0; i < ptContourss.size(); i++) {
						if (cv::contourArea(ptContourss[i]) > 10000) {
							cv::Rect boundingRects = cv::boundingRect(ptContourss[i]);
							dt2 = matThreshs(boundingRects);
						}

					}


					//imshow("Affine_x", dt2);



					cv::imwrite(filename, dt2);







					IplImage *img = cvLoadImage(filename);
					IplImage *sub_image1 = nullptr, *sub_image2 = nullptr, *sub_image3 = nullptr, *sub_image4 = nullptr;

					CvRect rc1 = cvRect(0, 0, img->width, img->height / 4);
					CvRect rc2 = cvRect(0, img->height / 4, img->width, img->height / 4);
					CvRect rc3 = cvRect(0, img->height / 2, img->width, img->height / 4);
					CvRect rc4 = cvRect(0, img->height * 3 / 4, img->width, img->height / 4);

					sub_image1 = cvCreateImage(cvSize(rc1.width, rc1.height), IPL_DEPTH_8U, 3);
					sub_image2 = cvCreateImage(cvSize(rc2.width, rc2.height), IPL_DEPTH_8U, 3);
					sub_image3 = cvCreateImage(cvSize(rc3.width, rc3.height), IPL_DEPTH_8U, 3);
					sub_image4 = cvCreateImage(cvSize(rc4.width, rc4.height), IPL_DEPTH_8U, 3);



					cvDrawRect(img, cvPoint(rc1.x, rc1.y), cvPoint(rc1.x + rc1.width, rc1.y + rc1.height), 0, 0, 0);
					cvDrawRect(img, cvPoint(rc2.x, rc2.y), cvPoint(rc2.x + rc2.width, rc2.y + rc2.height), 0, 0, 0);
					cvDrawRect(img, cvPoint(rc3.x, rc3.y), cvPoint(rc3.x + rc3.width, rc3.y + rc3.height), 0, 0, 0);
					cvDrawRect(img, cvPoint(rc4.x, rc4.y), cvPoint(rc4.x + rc4.width, rc4.y + rc4.height), 0, 0, 0);

					cvSetImageROI(img, rc1);
					cvCopy(img, sub_image1);

					cvSetImageROI(img, rc2);
					cvCopy(img, sub_image2);

					cvSetImageROI(img, rc3);
					cvCopy(img, sub_image3);

					cvSetImageROI(img, rc4);
					cvCopy(img, sub_image4);

					cvResetImageROI(img);

					cv::Mat cut1 = cv::cvarrToMat(sub_image2);
					cv::Mat cut2 = cv::cvarrToMat(sub_image4);

					a(cut1);
					a(cut2);


					Book* books = xlCreateBook();
					if (books)
					{
						if (books->load(L"Student.xls"))
						{
							Sheet* sheet = books->getSheet(0);
							if (sheet)
							{



								//答案
								string usernames = str;
								wstring wideusernames;
								for (int i = 0; i < usernames.length(); ++i)
									wideusernames += wchar_t(usernames[i]);
								const wchar_t* StuAns = wideusernames.c_str();
								sheet->writeStr(num_i + 3, 4, StuAns);


								//for teacher
								if (num_i == 0) {
									//MessageBox::Show("input OK");
									char filename_t[] = "T.txt";
									fstream fp;
									fp.open(filename_t, ios::out);

									fp << str << endl;
									fp.close();

									string ero = "@0@--@--@--@"+str+"\n@";
									const char *sendbuf = ero.c_str();
									send(client, sendbuf, (int)strlen(sendbuf) + 1, 0);
									cout << "T_Ans:" << str << endl;
									cout << "listening... \n";
								}



								//for student
								else {





									//學號
									string usernames2 = student[num_i].id;
									wstring wideusernames2;
									for (int i = 0; i < usernames2.length(); ++i)
										wideusernames2 += wchar_t(usernames2[i]);
									const wchar_t* StuAns2 = wideusernames2.c_str();
									sheet->writeStr(num_i + 3, 2, StuAns2);




									//對答案
									System::String^ tAns = "T.txt";
									System::String^ tAns_s = File::ReadAllText(tAns);

									int err = 0;
									int f[30];
									int j = 0;
									std::string what[30];

									for (int i = 0; i < 30; i++) {
										f[i] = 0;
										//what[i] = "";

										if (tAns_s[i] != str[i]) {
											err++;
											f[j] = i + 1;
											//what[j] = str[i];
											j++;
										}
									}


									char filename2[] = "StuF.txt";
									fstream fp;
									fp.open(filename2, ios::out);

									for (int i = 0; i <= j; i++) {
										fp << f[i] << endl;
									}
									fp.close();



									int score = 100;
									score = score - err * 4;
									if (score < 0) score = 0;

									sheet->writeNum(num_i + 3, 3, score);


									char filename1[] = "Stu.txt";

									fp.open(filename1, ios::out);
									fp << "Number : " << num << endl;
									fp << "ID : " << student[num_i].id << endl;
									fp << "Your Ans : " << str << endl;
									fp.close();


									const char* chars = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(tAns_s)).ToPointer();
									string tch = chars;



									std::string score_s = std::to_string(score);
										/*
									std::string str2="";
									for (int i = 0; i < 30; i++) {
										if (i % 5 == 0)
											str2 = str2 + "0";
										str2 =str2+ str[i];
									}

									std::string str3="";
									for (int i = 0; i < 30; i++) {
										if (i % 5 == 0)
											str3 = str3 + "0";
										str3 = str3 + tch[i];
									}
									*/
									string ero = "@" +num + "@" +id +"@" + score_s + "@" + str + "@" + tch + "@";
									char filenamez[] = "result.txt";
									fp.open(filenamez, ios::out);
									fp << ero << endl;
									fp.close();





									const char *sendbuf = ero.c_str();
									send(client, sendbuf, (int)strlen(sendbuf) + 1, 0);



									System::String^ strfilenames = "Stu.txt";
									System::String^ Readfiles = File::ReadAllText(strfilenames);
									//	MessageBox::Show(Readfiles);
									//	MessageBox::Show("Your score:" + Convert::ToString(score));

									//	MessageBox::Show(ero);

									cout << "Score:" << score << " ,Stu_Ans:" << str << " ,T_Ans:"<< tch << endl;
									cout << "listening... \n";

								}

							}

						}

						books->save(L"Student.xls");
						books->release();



					}



					str = "";
					h = 0;


					break;
				}



				else if (mode == "Simple_Mode") {
					std::vector<ContourWithData> allContoursWithData_sim;           // declare empty vectors,
					std::vector<ContourWithData> validContoursWithData_sim;

					cv::Mat matTestingNumbers_sim = cv::imread("sim.png", CV_LOAD_IMAGE_GRAYSCALE);       // read in the test numbers image

					if (matTestingNumbers_sim.empty()) {                                // if unable to open image
						std::cout << "error: image not read from file\n\n";         // show error message on command line
																					// and exit program
					}

					cv::Mat matGrayscale_sim;           //
					cv::Mat matBlurred_sim;             // declare more image variables
					cv::Mat matThresh_sim;              //
					cv::Mat matThreshCopy_sim;          //

					cv::threshold(matTestingNumbers_sim, matGrayscale_sim, 120, 255, THRESH_BINARY);          // convert to grayscale

																											  // blur
					cv::GaussianBlur(matGrayscale_sim,              // input image
						matBlurred_sim,                // output image
						cv::Size(5, 5),            // smoothing window width and height in pixels
						0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

												   // filter image from grayscale to black and white
					cv::adaptiveThreshold(matBlurred_sim,                           // input image
						matThresh_sim,                            // output image
						255,                                  // make pixels that pass the threshold full white
						cv::ADAPTIVE_THRESH_GAUSSIAN_C,       // use gaussian rather than mean, seems to give better results
						cv::THRESH_BINARY_INV,                // invert so foreground will be white, background will be black
						11,                                   // size of a pixel neighborhood used to calculate threshold value
						2);                                   // constant subtracted from the mean or weighted mean

					matThreshCopy_sim = matThresh_sim.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image

					std::vector<std::vector<cv::Point> > ptContours_sim;        // declare a vector for the contours
					std::vector<cv::Vec4i> v4iHierarchy_sim;                    // declare a vector for the hierarchy (we won't use this in this program but this may be helpful for reference)

					cv::findContours(matThreshCopy_sim,             // input image, make sure to use a copy since the function will modify this image in the course of finding contours
						ptContours_sim,                             // output contours
						v4iHierarchy_sim,                           // output hierarchy
						cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
						cv::CHAIN_APPROX_SIMPLE);               // compress horizontal, vertical, and diagonal segments and leave only their end points

					for (int i = 0; i < ptContours_sim.size(); i++) {               // for each contour
						ContourWithData contourWithData_sim;                                                    // instantiate a contour with data object
						contourWithData_sim.ptContour = ptContours_sim[i];                                          // assign contour to contour with data
						contourWithData_sim.boundingRect = cv::boundingRect(contourWithData_sim.ptContour);         // get the bounding rect
						contourWithData_sim.fltArea = cv::contourArea(contourWithData_sim.ptContour);               // calculate the contour area
						allContoursWithData_sim.push_back(contourWithData_sim);                                     // add contour with data object to list of all contours with data
					}

					for (int i = 0; i < allContoursWithData_sim.size(); i++) {                      // for all contours
						if (allContoursWithData_sim[i].checkIfContourIsValid()) {                   // check if valid
							validContoursWithData_sim.push_back(allContoursWithData_sim[i]);            // if so, append to valid contour list
						}
					}
					// sort contours from left to right
					std::sort(validContoursWithData_sim.begin(), validContoursWithData_sim.end(), ContourWithData::sortByBoundingRectXPosition);

					std::string strFinalString_sim;         // declare final string, this will have the final number sequence by the end of the program

					for (int i = 0; i < validContoursWithData_sim.size(); i++) {            // for each contour

																							// draw a green rect around the current char
						cv::rectangle(matTestingNumbers_sim,                            // draw rectangle on original image
							validContoursWithData_sim[i].boundingRect,        // rect to draw
							cv::Scalar(0, 0, 255),                        // green
							2);                                           // thickness

						cv::Mat matROI_sim = matThresh_sim(validContoursWithData_sim[i].boundingRect);          // get ROI image of bounding rect

						cv::Mat matROIResized_sim;
						cv::resize(matROI_sim, matROIResized_sim, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     // resize image, this will be more consistent for recognition and storage

						cv::Mat matROIFloat_sim;
						matROIResized_sim.convertTo(matROIFloat_sim, CV_32FC1);             // convert Mat to float, necessary for call to find_nearest

						cv::Mat matROIFlattenedFloat_sim = matROIFloat_sim.reshape(1, 1);

						cv::Mat matCurrentChar_sim(0, 0, CV_32F);

						kNearest->findNearest(matROIFlattenedFloat_sim, 1, matCurrentChar_sim);     // finally we can call find_nearest !!!

						float fltCurrentChar_sim = (float)matCurrentChar_sim.at<float>(0, 0);

						strFinalString_sim = strFinalString_sim + char(int(fltCurrentChar_sim));        // append current char to full string
					}

					std::cout  << "numbers read = " << strFinalString_sim << "\n";       // show the full string

				//	cv::imshow("matTestingNumbers", matTestingNumbers_sim);     // show input image with green boxes drawn around found digits
					fstream fp;
					std::string ero = "@--@--@--@--@" + strFinalString_sim +"\n@";
					cout << "listening... \n";
					char filenamez[] = "result.txt";
					fp.open(filenamez, ios::out);
					fp << ero << endl;
					fp.close();

					const char *sendbuf = ero.c_str();
					send(client, sendbuf, (int)strlen(sendbuf) + 1, 0);
					Sleep(500);

					//MessageBox::Show("simple_mode done");

					break;
				}





			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\')
					{
						// Is the command quit? 
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}

						// Unknown command
						continue;
					}

					// Send message to other clients, and definiately NOT the listening socket

					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock)
						{
							ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(listening, &master);
	closesocket(listening);

	// Message to let users know what's happening.
	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");































	}

	
	
































static std::string toss(System::String ^ s) {
	// convert .NET System::String to std::string
	const char* cstr = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	std::string sstr = cstr;
	Marshal::FreeHGlobal(System::IntPtr((void*)cstr));
	return sstr;
}

char* ConvertString2Char(System::String^ str) {
	char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(str);
	return str2;
}
