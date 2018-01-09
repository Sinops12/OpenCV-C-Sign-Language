
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
/*���� ���� ���� �߿��� �ν��� ���� �ߵǴ�
value = 134 <- ������ �ణ ���� ���ϋ�
value = 127 <- ���� ȸ���� ���� �����϶�
value = 140 <- ������ �Ͼ���϶�
value > 150(��� ���� x)�̸� �ν� �ȵ� or 120 ����(�հ��� x)�̸� �ν� �ȵ�*/


//��ǥ�� ���͸� �����Ѵ�.
bool cwComp(Point p1, Point p2) {
	return p1.x < p2.x;
}

//����� �����ӿ� ǥ���մϴ�.
void displayGesture(Mat frame, int feature[]) {
	string result = "�ν� �Ҽ� �����ϴ�.";
	int b0 = feature[0];
	int b1 = feature[1];
	int b2 = feature[2];
	int b3 = feature[3];
	int b4 = feature[4];
	if ((b0 == 1 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 0))
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 1 && b3 == 1 && b4 == 1)
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 0)
		result = "��";
	else if (b0 == 0 && b1 == 0 && b2 == 0 && b3 == 1 && b4 == 1)
		result = "��";
	else if (b0 == 0 && b1 == 0 && b2 == 1 && b3 == 1 && b4 == 1)
		result = "��";
	else if (b0 == 0 && b1 == 1 && b2 == 1 && b3 == 0 && b4 == 0)
		result = "��";
	else if (b0 == 0 && b1 == 1 && b2 == 1 && b3 == 1 && b4 == 0)
		result = "��";
	else if (b0 == 1 && b1 == 1 && b2 == 1 && b3 == 0 && b4 == 1)
		result = "��";
	else if ((b0 == 0 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 0) || (b0 == 0 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 1))
		result = "��";
	else if (b0 == 0 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 1)
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1)
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 0)
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1)
		result = "��";
	else if (b0 == 1 && b1 == 0 && b2 == 1 && b3 == 0 && b4 == 0)
		result = "��";
	else if ((b0 == 1 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 1) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 0 && b4 == 0) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 0) || (b0 == 1 && b1 == 1 && b2 == 0 && b3 == 1 && b4 == 1))
		result = "��";
	else
		result = "�ν� ���� ���߽��ϴ�.";

	putText(frame, result, Point(frame.cols - 100, 80), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 4);
	cout << " result : " << result << endl;
}

int main()
{


	VideoCapture cap(0);
	Mat frame;
	namedWindow("live feed", 1); // �հ��� �� �� �ν��� ���� �ߵǴ� �������� ã�� ���� Ʈ���� 
	createTrackbar("Set Threshold Value", "live feed", &threshold_val, 256);
	
	if (!cap.isOpened())
		return -1;
	cap >> frame;
	while (true) {
		
		/*-�չٴ� ������ ���簢���� �����Ѵ�.
		- ������ �����ϱ� ���� ����þ� �� ����
		- �չٴ��� �����ϱ� ���� �Ӱ� ���� �����Ͻʽÿ�(�Ӱ� ���� �����ϱ� ���� Ʈ�� �� ����)
		- ���� ����*/
		cap >> frame;
		Rect roi_ = Rect(50, 50, 200, 200); /*�պκ��� ���� ����*/
		Mat roi = frame(roi_);

		cvtColor(roi, roi, CV_BGR2GRAY);// ���� ���� ���� ���� �ϱ����� BGR���� GRAY SCALE�� ���� �ߴ�.
		GaussianBlur(roi, roi, Size(3, 3), 5, 5); // ����þ� ���� �����Ͽ� �ֺ� ������ �����ߴ�.
		threshold(roi, roi, threshold_val, 256, CV_THRESH_TOZERO);
		Mat se = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		dilate(roi, roi, se); //��â ������ ����ؼ� ������ ���Ž�Ų��

							  /*-���� ����
							  �չٴ��� �߷� �߽�
							  */
		int orientation = 0;
		for (int i = 0; i < roi.cols; i++) { //���ɿ����� �߽����� ����Ѵ�.
			if (roi.at<uchar>(roi.rows - 1, i) > 0) {
				orientation = 0;
				break;
			}
			else if (roi.at<uchar>(i, roi.cols - 1) > 0) {
				orientation = 1;
				break;
			}
		}
		/*��� ���*/
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

		/*�հ��� �� ����
		- ����� ���ͷκ��� �չٴ��� �߿��� ��踦 ã�´�.
		���� ��迡 ���� �ٰ��� �ٻ�� ����
		- ��ũ�� �����մϴ�(���� y�� �� ���� ������ ����Ʈ���� �����˴ϴ�)
		�ߺ� ��ũ�� Ȯ���Ͻʽÿ�.
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

				// �� ������ ǥ�õǴ� ��
			}
		}


		// ���� �� �۾� : ���� ���� (���� ��ȣ)

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
						// ���� �ȿ� ���� �������� ������ ���� �Ķ���
					}
				}
			}

			// ������ ��ũ�� �ɷ� ����.
			vector<Point> peaks0;
			for (int i = 0; i < peaks_.size(); i++) {
				int nexti = (i + 1) % peaks_.size();
				if (abs(peaks_[i].y - peaks_[nexti].y) < 15 && abs(peaks_[i].x - peaks_[nexti].x) < 15) {
					continue;
				}
				peaks0.push_back(peaks_[i]);
				circle(frame, peaks_[i] + Point(roi_.x, roi_.y), 20, Scalar(0, 0, 255));

				// ������ ��ũ�� �ɷ����� ������
			}
			//cout << "peaks: " << peaks0.size() << '\n';
			int feature[5];
			bool detected = false;

			//���� �� �հ����� ���� �հ����� Ư¡ ���ͷ� ��ũ ��ȯ
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

