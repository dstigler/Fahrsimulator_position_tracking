#pragma once

#include "opencv2\opencv.hpp"

using namespace std;

bool has_target_color(double* target_color_max, double* target_color_min, CvScalar& color_pxl) {

	uint8_t green = 0, blue = 0, red = 0, c4 = 0;
	green = uint8_t(color_pxl.val[0]),
		blue = uint8_t(color_pxl.val[1]),
		red = uint8_t(color_pxl.val[2]),
		c4 = uint8_t(color_pxl.val[3]);
	//target_color = RBG

	if (red >= target_color_min[0] && red <= target_color_max[0] &&
		green >= target_color_min[1] && green <= target_color_max[1] &&
		blue >= target_color_min[2] && blue <= target_color_max[2]) {
		return true;
	}

	return false;
}

void findNeighbors(int x, int y, double* target_color_max,
	double* target_color_min,
	std::map<std::pair<int, int>, bool>& stack) {

	// TODO FIND ERROR
	if ((stack.find(std::make_pair(x, y)) == stack.end()) &&
		has_target_color(target_color_max, target_color_min, cvGet2D(color, y, x / 1.33335))) {
		stack[(std::make_pair(x, y))] = true;

		if (x > 1) {
			findNeighbors(x - 1, y, target_color_max, target_color_min, stack);
		}
		if (y > 1) {
			findNeighbors(x, y - 1, target_color_max, target_color_min, stack);
		}
		if (x < color->width - 1) {
			findNeighbors(x + 1, y, target_color_max, target_color_min, stack);
		}
		if (y < color->height - 1) {
			findNeighbors(x, y + 1, target_color_max, target_color_min, stack);
		}
	}
}

void region_growing(int* start, double* target_color_max, double* target_color_min) {
	std::map<std::pair<int, int>, bool> stack;

	try
	{
		findNeighbors(start[0], start[1], target_color_max, target_color_min, stack);

	}
	catch (const std::exception &e)
	{
		std::cout << "Exception at findNeighbors-call" << std::endl;
	}
	long int sum_x = 0;
	long int sum_y = 0;
	if (!stack.empty()) {
		for (auto a : stack) {
			sum_x += a.first.first;
			sum_y += a.first.second;
		}
		start[0] = sum_x / stack.size();
		start[1] = sum_y / stack.size();
	}
}
std::vector<int*> get_seed_coordinates2(double* target_color_max, double* target_color_min, int* target_color) {
	std::vector<int*> cont;
	int* best_pos = new int[2]{ 0, 0 };
	long int min_error = 255 * 255 * 255;
	int i = 0;
	double red_sum = 0.0;
	double blue_sum = 0.0;
	double green_sum = 0.0;


	for (int x = 0; x < color->width; x++) {
		for (int y = 0; y < color->height; y++) {
			CvScalar color_pxl = cvGet2D(color, y, x);
			uint8_t green = uint8_t(color_pxl.val[0]),
				blue = uint8_t(color_pxl.val[1]),
				red = uint8_t(color_pxl.val[2]),
				c4 = uint8_t(color_pxl.val[3]);
			//target_color = RBG

			if (red >= target_color_min[0] && red <= target_color_max[0] &&
				green >= target_color_min[1] && green <= target_color_max[1] &&
				blue >= target_color_min[2] && blue <= target_color_max[2]) {

				int x2 = x*1.333;
				int *a = new int[2]{ x2, y };
				cont.push_back(a);
				if (abs(red - target_color[0]) * abs(blue - target_color[1]) * abs(green - target_color[2]) < min_error) {
					min_error = abs(red - target_color[0]) * abs(blue - target_color[1]) * abs(green - target_color[2]);
					best_pos[0] = x2;
					best_pos[1] = y;
				}

				i++;
				//red_sum += red;
				//green_sum += green;
				//blue_sum += blue;
				//std::cout << "( R:" << red << ", G: " << green << ", B: " << blue << " ) found";
			}


			/*if(x == x_mid && y == y_mid){
			std::cout << "Middle-Color: ( R:" <<red << ", G: " << green << ", B: " << blue << std::endl;
			}*/

		}
	}

	//region_growing(best_pos, target_color_max, target_color_min, color);

	//std::cout << "Points found: " << i << endl;

	return cont;

}
int* get_seed_coordinates3(double* target_color_max, double* target_color_min, int* target_color) {


	int* best_pos = new int[2]{ 1, 1 };
	long int min_error = 255 * 255 * 255;
	int i = 0;
	double red_sum = 0.0;
	double blue_sum = 0.0;
	double green_sum = 0.0;


	for (int x = 0; x < color->width; x++) {
		for (int y = 0; y < color->height; y++) {
			CvScalar color_pxl = cvGet2D(color, y, x);
			uint8_t green = uint8_t(color_pxl.val[0]),
				blue = uint8_t(color_pxl.val[1]),
				red = uint8_t(color_pxl.val[2]),
				c4 = uint8_t(color_pxl.val[3]);
			//target_color = RBG

			if (red >= target_color_min[0] && red <= target_color_max[0] &&
				green >= target_color_min[1] && green <= target_color_max[1] &&
				blue >= target_color_min[2] && blue <= target_color_max[2]) {

				int x2 = x*1.333;

				if (abs(red - target_color[0]) * abs(blue - target_color[1]) * abs(green - target_color[2]) < min_error) {
					min_error = abs(red - target_color[0]) * abs(blue - target_color[1]) * abs(green - target_color[2]);
					best_pos[0] = x2;
					best_pos[1] = y;
				}
			}
		}
	}

	region_growing(best_pos, target_color_max, target_color_min);

	return best_pos;
}



void findColorAndMark(int* rgb_target, std::string s = "unknown", double toleranceFactor = generalTolerance) {

	double range = toleranceFactor * 255;
	double* rgb_min = new double[3]{ max(0.0, rgb_target[0] - range), max(0.0, rgb_target[1] - range), max(0.0, rgb_target[2] - range) };
	double* rgb_max = new double[3]{ min(255.0, rgb_target[0] + range), min(255.0, rgb_target[1] + range), min(255.0, rgb_target[2] + range) };
	cv::Point textPos(0, 0);

	/*
	std::vector<int*> result = get_seed_coordinates2(rgb_max, rgb_min, rgb_target, color);
	cv::Mat output_frame(cv::cvarrToMat(color));
	for (auto a : result) {

	cv::Point *target = new cv::Point(int(0.5 + a[0] * 0.75), a[1]);

	cvCircle(color, *target, 2, cv::Scalar(169, 169, 169));
	//cvCircle(color, *target, 2, cv::Scalar(rgb_target[1], rgb_target[2], rgb_target[0]));
	if (textPos.x < target->x &&textPos.y < target->y) {
	textPos.x = target->x + 2;
	textPos.y = target->y + 2;
	}
	delete target;
	}
	for (auto e : result) {
	delete e;
	}
	*/

	int * a = get_seed_coordinates3(rgb_max, rgb_min, rgb_target);
	cv::Mat output_frame(cv::cvarrToMat(color));


	cv::Point *target = new cv::Point(int(0.5 + a[0] * 0.75), a[1]);

	cvCircle(color, *target, 1, cv::Scalar(0, 0, 0));
	double* angle = GetAngleFromColorIndex(a[0], a[1]);
	double* realcoord = Get3DCoordinates(angle, depthImg);
	//cvCircle(color, *target, 2, cv::Scalar(rgb_target[1], rgb_target[2], rgb_target[0]));
	if (textPos.x < target->x &&textPos.y < target->y) {
		textPos.x = target->x + 2;
		textPos.y = target->y + 2;
	}
	delete target;





	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
	std::ostringstream os;
	os << realcoord[2];
	std::string str = os.str();
	std::string outputStr = s.append(" ").append(str);
	//std::cout << "X: " << realcoord[0] << " Y: " << realcoord[1] << " Z: " << realcoord[2] << std::endl;
	cvPutText(color, outputStr.c_str(), textPos, &font, cv::Scalar(0.0, 0.0, 0.0));
	//std::cout << result[0][1] << " " << result[0][0] << std::endl;
	//std::cout << result.size() << std::endl;

	cvCircle(color, cv::Point(320, 240), 3, cv::Scalar(0, 255, 0));
	delete rgb_max;
	delete rgb_min;

	delete[] a;
}