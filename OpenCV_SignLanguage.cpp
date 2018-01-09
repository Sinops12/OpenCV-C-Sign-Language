
#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<algorithm>
#include <stdlib.h>
#include <fstream>
using namespace std;
using namespace cv;

int threshold_val = 128;
/*이진 영상 값들 중에서 인식이 제일 잘되는
value = 134 <- 벽면이 약간 누런 색일떄
value = 127 <- 조금 회색빛 도는 벽면일때
value = 140 <- 벽면이 하얀색일때
value > 150(배경 검출 x)이면 인식 안됨 or 120 이하(손검출 x)이면 인식 안됨*/


//좌표의 벡터를 정렬한다.
bool cwComp(Point p1, Point p2) {
	return p1.x < p2.x;
}

//결과를 프레임에 표시합니다.
void displayGesture(Mat frame, int feature[]) {
	string result = "인식 할수 없습니다.";
	int b0 = feature[0];
	int b1 = feature[1];
	int b2 = feature[2];
	int b3 = feature[3];
	int b4 = feature[4];
	if ((b0 == 1 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 0))
		result = "ㅏ";
	else if (b0 == 1 && b1 == 0 && b2 == 1 && b3 == 1 && b4 == 1)
		result = "ㄱ";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 0)
		result = "ㄴ";
	else if (b0 == 0 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 1)
		result = "ㄷ";
	else if (b0 == 0 && b1 == 0 && b2 == 1 && b3 == 1 && b4 == 1)
		result = "ㄹ";
	else if (b0 == 0 && b1 == 1 && b2 == 1 && b3 == 0 && b4 == 0)
		result = "ㅁ";
	else if (b0 == 0 && b1 == 1 && b2 == 1 && b3 == 1 && b4 == 0)
		result = "ㅂ";
	else if (b0 == 1 && b1 == 1 && b2 == 1 && b3 == 0 && b4 == 1)
		result = "ㅇ";
	else if ((b0 == 0 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 0) || (b0 == 0 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 1))
		result = "ㅎ";
	else if (b0 == 0 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 1)
		result = "ㅅ";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1)
		result = "ㅔ";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 0)
		result = "ㅣ";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1)
		result = "ㅗ";
	else if (b0 == 1 && b1 == 0 && b2 == 1 && b3 == 0 && b4 == 0)
		result = "ㅛ";
	else if ((b0 == 1 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 1) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 0) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 0) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 1))
		result = "ㅕ";
	else
		result = "인식 하지 못했습니다.";

	putText(frame, result, Point(frame.cols - 100, 80), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 4);
	cout << " result : " << result << endl;
}

int main()
{


	VideoCapture cap(0);
	Mat frame;
	namedWindow("live feed", 1); // 손검출 된 후 인식이 제일 잘되는 이진값을 찾기 위한 트랙바 
	createTrackbar("Set Threshold Value", "live feed", &threshold_val, 256);
	
	if (!cap.isOpened())
		return -1;
	cap >> frame;
	while (true) {
		
		/*-손바닥 영역의 직사각형을 정의한다.
		- 잡음을 제거하기 위해 가우시안 블러 적용
		- 손바닥을 구분하기 위해 임계 값을 수행하십시오(임계 값을 조정하기 위해 트랙 바 정의)
		- 소음 제거*/
		cap >> frame;
		Rect roi_ = Rect(50, 50, 200, 200); /*손부분의 관심 영역*/
		Mat roi = frame(roi_);

		cvtColor(roi, roi, CV_BGR2GRAY);// 단지 배경과 손을 검출 하기위해 BGR에서 GRAY SCALE로 변경 했다.
		GaussianBlur(roi, roi, Size(3, 3), 5, 5); // 가우시안 블러를 적용하여 주변 잡음을 제거했다.
		threshold(roi, roi, threshold_val, 256, CV_THRESH_TOZERO);
		Mat se = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		dilate(roi, roi, se); //팽창 연산을 사용해서 잡음을 제거시킨다

							  /*-방향 감지
							  손바닥의 중력 중심
							  */
		int orientation = 0;
		for (int i = 0; i < roi.cols; i++) { //관심영역의 중심점을 계산한다.
			if (roi.at<uchar>(roi.rows - 1, i) > 0) {
				orientation = 0;
				break;
			}
			else if (roi.at<uchar>(i, roi.cols - 1) > 0) {
				orientation = 1;
				break;
			}
		}
		/*명암 대비*/
		float cx = 0.0, cy = 0.0;
		float sumi = 1.0;
		for (int i = 0; i < roi.rows; i++) {
			for (int j = 0; j < roi.cols; j++) {
				cy += roi.at<uchar>(i, j) * i;
				cx += roi.at<uchar>(i, j) * j;
				sumi += roi.at<uchar>(i, j);
			}
		}
		cx /= sumi;
		cy /= sumi;

		/*손가락 끝 감지
		- 경계의 벡터로부터 손바닥의 중요한 경계를 찾는다.
		위의 경계에 대한 다각형 근사법 적용
		- 피크를 검출합니다(상위 y는 두 개의 인접한 포인트에서 조정됩니다)
		중복 피크를 확인하십시오.
		*/
		vector<vector <Point> > contours;
		Mat temp = roi.clone();
		vector<Vec4i> heiarchy;
		findContours(temp, contours, heiarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

		int c_size = 0, c_idx = 0;
		for (int i = 0; i < contours.size(); i++) {
			if (contours.size() > c_size) {
				c_idx = i;
				c_size = contours.size();
			}
		}

		vector<Point> boundary0;
		vector<Point> peaks_;
		if (!contours.empty()) {
			boundary0 = contours[c_idx];
		}

		vector<Point> boundary;
		if (!boundary0.empty()) {
			approxPolyDP(boundary0, boundary, 2, false);

			for (int i = 0; i < boundary.size(); i++) {
				circle(frame, boundary[i] + Point(roi_.x, roi_.y), 5, Scalar(100, 200, 100));

				// 손 영역에 표시되는 원
			}
		}


		// 수행 할 작업 : 수평 방향 (숫자 기호)

		if (orientation == 0) {
			for (int i = 0; (i + 1) < boundary.size(); i++) {
				Point p = boundary[i];
				if (cy > p.y) {
					bool isPeak = true;
					int indxl = (i - 1) % boundary.size();
					int indxr = (i + 1) % boundary.size();
					if (boundary[indxl].y < p.y || boundary[indxr].y < p.y) {
						isPeak = false;
					}
					if (isPeak) {
						peaks_.push_back(p);
						//circle(frame, p+Point(roi_.x, roi_.y), 20, Scalar(255, 0, 0));
						putText(frame, format("[%02d]", p), p + Point(roi_.x, roi_.y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 255));
						// 영역 안에 손의 꼭지점이 존재할 경우는 파란색
					}
				}
			}

			// 여분의 피크를 걸러 낸다.
			vector<Point> peaks0;
			for (int i = 0; i < peaks_.size(); i++) {
				int nexti = (i + 1) % peaks_.size();
				if (abs(peaks_[i].y - peaks_[nexti].y) < 15 && abs(peaks_[i].x - peaks_[nexti].x) < 15) {
					continue;
				}
				peaks0.push_back(peaks_[i]);
				circle(frame, peaks_[i] + Point(roi_.x, roi_.y), 20, Scalar(0, 0, 255));

				// 여분의 피크를 걸러내면 빨간색
			}
			//cout << "peaks: " << peaks0.size() << '\n';
			int feature[5];
			bool detected = false;

			//융기 된 손가락과 접힌 손가락의 특징 벡터로 피크 변환
			sort(peaks0.begin(), peaks0.end(), cwComp);

			if (peaks0.size() == 4) {

				Rect thumbRect(0, cy, roi_.width, roi_.height - cy);
				Mat thumbRoi = roi(thumbRect);
				vector< vector<Point> > thumbContours;
				vector<Vec4i> tHeirachy;
				temp = thumbRoi.clone();
				findContours(temp, thumbContours, tHeirachy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
				int tc_size = 0, tc_idx = 0;
				for (int i = 0; i < thumbContours.size(); i++) {
					if (thumbContours.size() > tc_size) {
						tc_idx = i;
						tc_size = thumbContours.size();
					}
				}

				vector<Point> tboundary0;
				if (!thumbContours.empty()) {
					tboundary0 = thumbContours[tc_idx];
				}
				double perimeter = arcLength(tboundary0, true);
				double area = contourArea(tboundary0);
				double compactness = perimeter * perimeter / (4 * 3.14*area);
				double distArray[4];
				//cout<<"[ ";
				for (int i = 0; i < peaks0.size(); i++) {
					//	cout<<peaks0[i].y<<" ";
					distArray[i] = sqrt((cy - peaks0[i].y)*(cy - peaks0[i].y) + (cx - peaks0[i].x)*(cx - peaks0[i].x));
				}
				//cout<<"]==>[ ";
				double max = 0.0;
				for (int i = 0; i < 4; i++) {
					if (distArray[i] > max)
						max = distArray[i];
				}
				feature[4] = compactness > 1.8 ? 1 : 0;
				for (int i = 0; i < 4; i++) {
					if (distArray[i] > 0.75*max)
						feature[i] = 1;
					else
						feature[i] = 0;
				}
				for (int i = 0; i<5; i++) {
				cout<<feature[i]<<" ";
				}
				cout<<"]\n";
				detected = true;
			}
			if (detected) {
				displayGesture(frame, feature);
			}
			else {
				putText(frame, "Waiting for sign...", Point(frame.cols - 200, 80), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
			}
			line(frame, Point(roi_.x, cy + roi_.y), Point(roi_.x + roi_.width, cy + roi_.y), Scalar(0, 255, 0), 2);
		}

		line(frame, Point(cx + roi_.x, cy + roi_.y), Point(cx + roi_.x, cy + roi_.y), Scalar(0, 0, 255), 5);
		rectangle(frame, roi_, Scalar(255, 0, 0), 2);
		imshow("live feed", frame);
		imshow("ROI", roi);
		waitKey(10);
	}
	waitKey(0);

	return 0;
}

