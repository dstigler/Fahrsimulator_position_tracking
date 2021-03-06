#include "stdafx.h"

#include <winsock2.h>
#include <windows.h>
#include <thread>

#include <vector>
#include <map>
#include <utility>

#include "Globals.h"
#include "ColorDetection.h"

#include <string>
#include <iostream>

#include "KinectLayer.h"

#include <opencv2/opencv.hpp>
#include "opencv2/world.hpp"
#include "opencv2/highgui.hpp"

#include "Server.h"
#include "ColorPixel.h"
#include "XMLWriter.h"

using namespace std;

map<string, ColorPixel> colorMap;
XMLWriter* writer = nullptr;

int drawColor() {
	for (const auto e : pointVec) {
		circle(color, cv::Point2d(e.second[3], e.second[4]), 2.0 / resize_factor, cv::Scalar(0.0, 0.0, 0.0), -1);
	}

	bool** visited_array = new bool*[COLOR_WIDTH];
	for (int i = 0; i < COLOR_WIDTH; i++) {
		visited_array[i] = new bool[COLOR_HEIGHT];
	}

	for (auto& p : colorMap) {
		const auto adapted_color = find_color_and_mark(p.second, visited_array, p.first);
		p.second.red = adapted_color.red;
		p.second.green = adapted_color.green;
		p.second.blue = adapted_color.blue;
		p.second.x = adapted_color.x;
		p.second.y = adapted_color.y;
	}


	for (int i = 0; i < COLOR_WIDTH; i++) {
		delete visited_array[i];
	}
	delete[] visited_array;

	resize(color, color, cv::Size(color.cols * resize_factor, color.rows * resize_factor));
	imshow("color image", color);

	return 0;
}

static void addToColorMap(const ColorPixel pixel) {
	colorMap["C" + to_string(colorMap.size() + 1)] = pixel;
	writer->AddPixel("C" + to_string(colorMap.size()), colorMap.at("C" + to_string(colorMap.size())));
}

static ColorPixel add_point(const int x, const int y) {
	cv::Vec4b& color_val = color.at<cv::Vec4b>(y, x);

	const uint8_t blue = uint8_t(color_val[0]), // B
		green = uint8_t(color_val[1]), // G
		red = uint8_t(color_val[2]); // R

	cout << "Color: B: " << static_cast<int>(blue) << " G: " << static_cast<int>(green) << " R: " << static_cast<int>(red)
		<< endl;

	double* real_world_pos = new double[5]{-1000, -1000, -1000, 1, 1};
	CameraSpacePoint* camera_space_point = new CameraSpacePoint();
	const float fx = x;
	const float fy = y;
	kinect.coordinateMapper->MapDepthPointToCameraSpace(DepthSpacePoint{fx, fy}, kinect.getDepthForPixel(fx, fy),
	                                                    camera_space_point);
	real_world_pos[0] = camera_space_point->X;
	real_world_pos[1] = camera_space_point->Y;
	real_world_pos[2] = camera_space_point->Z;
	real_world_pos[3] = x;
	real_world_pos[4] = y;
	stringstream tmp;
	tmp << "P" << pointVec.size() + 1 << "(x: " << x << " y: " << y << ")";
	pointVec.push_back(make_pair(tmp.str(), real_world_pos));

	const int rec_x = x > 25 ? (x < COLOR_WIDTH - 25 ? x - 25 : COLOR_WIDTH - 50) : 1;
	const int rec_y = y > 25 ? (y < COLOR_HEIGHT - 25 ? y - 25 : COLOR_HEIGHT - 50) : 1;

	cout << "Real World: " << setprecision(6) << "P" << pointVec.size() << " X:" << real_world_pos[0] << " Y:" <<
		real_world_pos[1] << " Z:" << real_world_pos[2] << endl;

	return ColorPixel{red, green, blue, rec_x, rec_y};
}

static void remove_point() {
	if (!pointVec.empty()) {
		pointVec.pop_back();
		writer->RemovePixel("C" + to_string(colorMap.size()));
		colorMap.erase(colorMap.find("C" + to_string(colorMap.size())));
	}
}

static void onClick(const int event, const int x, const int y, int f, void*) {
	if (event == CV_EVENT_LBUTTONDOWN) {
		addToColorMap(add_point(x / resize_factor, y / resize_factor));
	}
	else if (event == CV_EVENT_RBUTTONDOWN) {
		remove_point();
	}
}

int main() {
	cv::namedWindow("color image", CV_WINDOW_AUTOSIZE);
	cv::setMouseCallback("color image", onClick);

	kinect.setDepth();
	kinect.setRGB(color);
	//while (color.at<cv::Vec4b>(100, 100) == cv::Vec4b(0, 0, 0, 0)) {// Check if Matrix is filled now
	//	kinect.setDepth();
	//	kinect.setRGB(color);
	//}

	writer = new XMLWriter("Points.xml");
	colorMap = *(writer->getPixels());

	for (const auto p : colorMap) {
		add_point(p.second.x, p.second.y);
	}
	thread server_thread(&startServer);
	cout << "Main thread" << endl;
	while (true) {
		drawColor();

		kinect.setDepth();
		kinect.setRGB(color);

		const int c = cvWaitKey(1);
		if (c == 27 || c == 'q' || c == 'Q')
			break;
	}

	run = false;
	server_thread.join();

	cv::destroyAllWindows();
	return 0;
}
