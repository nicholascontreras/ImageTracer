#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <Windows.h>
#include <chrono>
#include <thread>

#include <iostream>
#include <stdlib.h>
#include <time.h>

const char* WINDOW_NAME = "Image Tracer - Preview";
const int BEEP_PITCH = 523;
const int BEEP_LENGTH = 200;

cv::Mat src_image;
cv::Mat edges_image;

int low_threshold = 50;
int high_threshold = 100;

void run_canny();
void draw_image();

int main(int argc, char** argv) {
	srand(time(nullptr));

	// Open file dialog
	OPENFILENAMEW open_file_dialog;
	wchar_t file_path_wide[MAX_PATH];
	ZeroMemory(&open_file_dialog, sizeof(open_file_dialog));
	open_file_dialog.lStructSize = sizeof(open_file_dialog);
	open_file_dialog.hwndOwner = nullptr;
	open_file_dialog.lpstrFile = file_path_wide;
	open_file_dialog.lpstrFile[0] = '\0';
	open_file_dialog.nMaxFile = MAX_PATH;
	open_file_dialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display dialog
	GetOpenFileNameW(&open_file_dialog);

	// Convert widechar (unicode) to standard 8-bit (ascii) char
	char file_path[MAX_PATH];
	wcstombs_s(nullptr, file_path, file_path_wide, MAX_PATH);

	src_image = cv::imread(file_path, cv::IMREAD_COLOR);
	if (src_image.empty()) {
		std::cout << "Could not find or open the image!" << std::endl;
		return 2;
	}

	// Crop to square
	int crop_size = src_image.rows < src_image.cols ? src_image.rows : src_image.cols;
	src_image = src_image(cv::Rect(0, 0, crop_size, crop_size));

	// Resize and pre-process image
	cv::resize(src_image, src_image, cv::Size(480, 480));
	cv::cvtColor(src_image, src_image, cv::COLOR_BGR2GRAY);
	cv::blur(src_image, src_image, cv::Size(3, 3));

	cv::namedWindow(WINDOW_NAME);
	run_canny();

	cv::createTrackbar("Low", WINDOW_NAME, &low_threshold, 1000, [](int new_value, void* userdata) {
		run_canny();
	});
	cv::createTrackbar("High", WINDOW_NAME, &high_threshold, 1000, [](int new_value, void* userdata) {
		run_canny();
	});

	while (true) {
		char key = (char)cv::waitKey(50);
		if (key == 'q') {
			break;
		} else if (key == 'd') {
			draw_image();
		}

		if (!cv::getWindowProperty(WINDOW_NAME, cv::WindowPropertyFlags::WND_PROP_VISIBLE)) {
			break;
		}
	}
	return 0;
}

void run_canny() {
	cv::Canny(src_image, edges_image, low_threshold, high_threshold);
	cv::imshow(WINDOW_NAME, edges_image);
}

void draw_image() {
	std::cout << "Beginning drawing process" << std::endl;
	Beep(BEEP_PITCH, BEEP_LENGTH * 3);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	for (int i = 0; i < 3; i++) {
		Beep(BEEP_PITCH, BEEP_LENGTH);
		std::this_thread::sleep_for(std::chrono::milliseconds(BEEP_LENGTH * 3));
	}

	POINT top_left;
	GetCursorPos(&top_left);
	std::cout << "Top left mouse position recorded" << std::endl;

	Beep(BEEP_PITCH, BEEP_LENGTH * 3);
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	for (int i = 0; i < 3; i++) {
		Beep(BEEP_PITCH, BEEP_LENGTH);
		std::this_thread::sleep_for(std::chrono::milliseconds(BEEP_LENGTH * 3));
	}

	POINT bottom_right;
	GetCursorPos(&bottom_right);
	std::cout << "Bottom right mouse position recorded" << std::endl;

	Beep(BEEP_PITCH, BEEP_LENGTH * 3);

	// Crop the selected rectangle to a square and center our drawing inside the selected rectangle
	int requested_width = bottom_right.x - top_left.x;
	int requested_height = bottom_right.y - top_left.y;
	int actual_size = requested_width < requested_height ? requested_width : requested_height;
	int actual_x = top_left.x + (requested_width - actual_size) / 2;
	int actual_y = top_left.y + (requested_height - actual_size) / 2;

	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);

	INPUT input_sequence[3] = { 0 };
	input_sequence[0].type = INPUT_MOUSE;
	input_sequence[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	input_sequence[1].type = INPUT_MOUSE;
	input_sequence[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input_sequence[2].type = INPUT_MOUSE;
	input_sequence[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	int points_drawn = 0;

	for (int y = 0; y < actual_size; y++) {
		for (int x = 0; x < actual_size; x++) {
			int mat_row = (int)(((double)y / actual_size) * edges_image.rows);
			int mat_col = (int)(((double)x / actual_size) * edges_image.cols);

			if (edges_image.at<bool>(mat_row, mat_col)) {
				int scaled_x = (int)((((double)actual_x + x) / screen_rect.right) * 65535);
				int scaled_y = (int)((((double)actual_y + y) / screen_rect.bottom) * 65535);
				input_sequence[0].mi.dx = scaled_x;
				input_sequence[0].mi.dy = scaled_y;

				SendInput(3, input_sequence, sizeof(INPUT));

				points_drawn++;
				if (points_drawn > 5) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					points_drawn = 0;
				}
			}
		}
	}

	std::cout << "Drawing complete" << std::endl;
	Beep(BEEP_PITCH, BEEP_LENGTH * 3);
}