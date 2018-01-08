#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
using namespace System::Runtime::InteropServices; 
namespace jacky_lib
{
	class mat2picture
	{
	public:
		mat2picture();
		System::Drawing::Bitmap^ Mat2Bimap(cv::Mat & colorImage);
	};
	class str2char
	{
	public:
		str2char();
		char * ConvertString2Char(System::String^ str) {	// Marshal method
			char* strs = (char*)(void*)Marshal::StringToHGlobalAnsi(str);
			return strs;
		}
	};
}

