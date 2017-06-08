#pragma once

double GetRadianFromDegree(double angleInDegree) {
	return angleInDegree * PI / 180.0;
}

double GetDegreeFromRadian(double angleInRadian) {
	return angleInRadian * 180.0 / PI;
}

//return 2 angles, for horizontal (x) and vertical (y)
double* GetAngleFromColorIndex(int colorX, int colorY) {
	double coordMidX = COLOR_WIDTH / 2.0 - 0.5;
	double coordMidY = COLOR_HEIGHT / 2.0 - 0.5;
	double* returnArr = new double[2]{ -100, -100 };

	double width = tan(GetRadianFromDegree(fovColorX / 2.0)) * 2;
	double height = tan(GetRadianFromDegree(fovColorY / 2.0)) * 2;
	double widthStep = width / (COLOR_WIDTH - 1);
	double heightStep = height / (COLOR_HEIGHT - 1);

	double centeredX = colorX - coordMidX;
	double centeredY = colorY - coordMidY;

	double trueAngleX = GetDegreeFromRadian(atan(centeredX * widthStep));
	double trueAngleY = GetDegreeFromRadian(atan(centeredY * heightStep));

	//cout << " for coords (" << colorX << ";" << colorY << ") the degrees are (" << trueAngleX << ";" << trueAngleY << ")" << endl;

	returnArr[0] = trueAngleX;
	returnArr[1] = trueAngleY;
	return returnArr;
}

double* Get3DCoordinates(double* angles, int** depthArr) {

	double* realWorldCoords = new double[5]{ -1000, -1000, -1000, 1, 1 };

	double colorAngleX = angles[0];
	double colorAngleY = angles[1];

	double width = tan(GetRadianFromDegree(fovDepthX / 2.0)) * 2;
	double height = tan(GetRadianFromDegree(fovDepthY / 2.0)) * 2;
	double widthStep = width / (DEPTH_WIDTH - 1);
	double heightStep = height / (DEPTH_HEIGHT - 1);

	double coordMidX = DEPTH_WIDTH / 2.0 - 0.5;
	double coordMidY = DEPTH_HEIGHT / 2.0 - 0.5;

	double distX = tan(GetRadianFromDegree(colorAngleX));
	double distY = tan(GetRadianFromDegree(colorAngleY));


	//calc index position in depth array
	int idxDepthX = (int)(distX / widthStep + coordMidX + 0.5);
	int idxDepthY = (int)(distY / heightStep + coordMidY + 0.5);

	//check range of index (is in FoV)
	if (idxDepthX >= 0 && idxDepthX < DEPTH_WIDTH && idxDepthY >= 0 && idxDepthY < DEPTH_HEIGHT) {
		double depthValZ = depthArr[idxDepthX][idxDepthY];

		double realWorldZ = depthValZ / 10.0; //convert from mm to cm
		double realWorldX = tan(GetRadianFromDegree(colorAngleX)) * realWorldZ;
		double realWorldY = tan(GetRadianFromDegree(colorAngleY)) * realWorldZ;
		realWorldCoords[0] = realWorldX;
		realWorldCoords[1] = realWorldY;
		realWorldCoords[2] = realWorldZ;

		//std::cout << "3D pos in cm: (" << realWorldX << ";" << realWorldY << ";" << realWorldZ << ")" << std::endl;
	}
	return realWorldCoords;
}

double GetLength(double* p1, double* p2) {
	double l1 = (p2[0] - p1[0]) * (p2[0] - p1[0]);
	double l2 = (p2[1] - p1[1]) * (p2[1] - p1[1]);
	double l3 = (p2[2] - p1[2]) * (p2[2] - p1[2]);
	return sqrt(l1 + l2 + l3);
}