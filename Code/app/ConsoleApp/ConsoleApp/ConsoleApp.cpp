#include "opencv2/highgui/highgui.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/videoio_c.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime> 
#include <format>


using namespace cv;
using namespace std;
COMMTIMEOUTS timeouts = { 0 };
char  ReadData;        //temperory Character
DWORD NoBytesRead;     // Bytes read by ReadFile()
DWORD dwEventMask;
uint8_t Buffer[1024];
int counter = 0;
int m_x = 0;
int m_y = 0;
float Gain = 1.0;
int saved_images = 0;
int saved_videos = 0;
boolean Rec = false;
boolean freez = false;
boolean avg = false;
boolean arrow = false;
int freez_state = 0;
int avg_state = 0;
int print_state = 0;
int arrow_state = 0;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x; coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    return;
}
void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    cursorInfo.dwSize = 1;
    SetConsoleCursorInfo(out, &cursorInfo);
}

void mouse_callback(int  event, int  x, int  y, int  flag, void* param)
{
    if (event == EVENT_MOUSEMOVE && x <= 512 && y <= 512) {
        m_x = x / 32;
        m_y = y / 32;
    }
}

void arrow_draw(Mat& img, Scalar color, uint16_t val[], int cal[], float strenght) {
    for (int y = 1; y < 15; y++) {
        for (int x = 1; x < 15; x++) {
            float dx = 0;
            float dy = 0;
            for (int yn = -1; yn != 2; yn++) {
                for (int xn = -1; xn != 2; xn++) {
                    float diff = ((float)val[y * 16 + x] - (float)cal[y * 16 + x] / 10.92) - ((float)val[(y + yn) * 16 + (x + xn)] - (float)cal[(y + yn) * 16 + (x + xn)] / 10.92);
                    double angle = atan2(yn, xn);
                    dx += cos(angle) * diff;
                    dy += sin(angle) * diff;
                }
            }
            arrowedLine(img, Point(x * 32 + 16, y * 32 + 16), Point(x * 32 + 16 + dx / strenght, y * 32 + 16 + dy / strenght), color, 1, 8, 0, 0.1);
        }
    }
}

int main()
{
    ShowConsoleCursor(false);
    namedWindow("Output");
    setMouseCallback("Output", mouse_callback);
    String port = "COM3";
    HANDLE handlePort_;
    handlePort_ = CreateFile(wstring(port.begin(), port.end()).c_str(),  // Specify port device: default "COM1"
        GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
        0,                                  // the devide isn't shared.
        NULL,                               // the object gets a default security.
        OPEN_EXISTING,                      // Specify which action to take on file. 
        0,                                  // default.
        NULL);                              // default.
    
    if (handlePort_ == INVALID_HANDLE_VALUE)
    {
        cout << "\n Port can't be opened\n\n";
        return FALSE;
    }
    cout << "port opened \n";


    DCB config_ = { 0 };
    config_.DCBlength = sizeof(config_);
    if (GetCommState(handlePort_, &config_) == 0)
    {
        cout << "get config has problem" << endl;
        return FALSE;
    }

    config_.BaudRate = 921600;  // Specify buad rate of communicaiton.
    config_.StopBits = ONESTOPBIT;  // Specify stopbit of communication.
    config_.Parity = NOPARITY;      // Specify parity of communication.
    config_.ByteSize = 8;  // Specify  byte of size of communication.

    if (SetCommState(handlePort_, &config_) == 0)
    {
        cout << "set config has problem" << endl;
        return FALSE;
    }
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    
    if (SetCommTimeouts(handlePort_, &timeouts) == FALSE)
    {
        cout << "error setting timeout";
        return FALSE;
    }
    
    SetCommMask(handlePort_, EV_RXCHAR);
    if (WaitCommEvent(handlePort_, &dwEventMask, NULL) == FALSE)
    {
        cout << "error setting wait";
        return FALSE;
    }
    gotoxy(0, 0);
    cout << "Frame Rate: \n";
    cout << "Gain: \n";
    cout << "Max Value: \n";
    cout << "Min Value: \n";
    cout << "Mouse Val: \n";
    cout << "Frame: \n";
    cout << "Pause (space): \n";
    cout << "Take image (P): \n";
    cout << "Start recording (R): \n";
    cout << "Stop recodfing (T): \n";
    cout << "Show field lines (E): \n";
    cout << "Average (A): \n";
    cout << "Write cal data (C) \n";
    cout << "Read cal data (V)  \n";
    chrono::milliseconds ms = chrono::duration_cast <chrono::milliseconds> (chrono::system_clock::now().time_since_epoch());
    int frame = 0;
    VideoWriter Video;
    uint64_t avg_val[256] = {};
    int avg_frame = 0;
    float Calibration_multiplier = 1.456;
    float DC_offset = 41;
    int mid_point[256];
    ifstream calFile;
    calFile.open("cal.txt", ios::in);
    if (calFile.is_open()) {
        for (int i = 0; i < 256; i++) {
            calFile >> mid_point[i];
        }
    }
    calFile.close();
    while (1) {
        do {
            ReadFile(handlePort_, &ReadData, sizeof(ReadData), &NoBytesRead, NULL);
            if (counter < 1024) {
                Buffer[counter] = ReadData;
            }
            ++counter;
        } while (NoBytesRead > 0);
        --counter;
        //cout << "Bytes read" << counter << endl;
        if (counter == 512) {
            uint16_t val[256] = { 0 };
            uint16_t max = 0;
            uint16_t min = 65535;
            for (int i = 0; i < counter / 2; i++) {
                if (!avg) {
                    val[i] = (uint16_t)Buffer[i * 2] << 6 | Buffer[i * 2 + 1];
                    val[i] += DC_offset;
                    val[i] *= Calibration_multiplier;
                }
                else {
                    avg_val[i] += (uint16_t)Buffer[i * 2] << 6 | Buffer[i * 2 + 1];
                    val[i] = avg_val[i] / avg_frame;
                    val[i] += DC_offset;
                    val[i] *= Calibration_multiplier;
                }
                if (max < val[i] - mid_point[i] / 10.92) {
                    max = val[i] - mid_point[i] / 10.92;
                }
                if (min > val[i] - mid_point[i] / 10.92) {
                    min = val[i] - mid_point[i] / 10.92;
                }
            }
            uint16_t temp_val[256];
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    temp_val[y * 16 + 15 - x] = val[y * 16 + x];
                }
            }
            for (int i = 0; i < 256; i++) {
                val[i] = temp_val[i];
            }
            
            Mat imageF(16, 16, CV_32FC3);
            int y = 0;
            int x = 0;
            float blue = 0.0;
            float green = 0.0;
            float red = 0.0;
            Vec3f intensity;
            float MID_POINT = 39967;

            for (y = 0; y < imageF.rows; y++)
            {
                for (x = 0; x < imageF.cols; x++)
                {
                    intensity = imageF.at<Vec3f>(Point(imageF.cols - x, y));
                    blue = (mid_point[y * 16 + x] - (float)val[y * 16 + x] * 10.92) * Gain / 65536.0;
                    red = ((float)val[y * 16 + x ] * 10.92 - mid_point[y * 16 + x]) * Gain / 65536.0;
                    green = 0.0;
                    intensity.val[0] = blue;
                    intensity.val[1] = green;
                    intensity.val[2] = red;
                    imageF.at<Vec3f>(Point(x, y)) = intensity;
                }
            }

            Mat infoBar(512, 192, CV_32FC3, Scalar(1.0, 1.0, 1.0));
            

            for (y = 0; y < infoBar.rows; y++)
            {
                for (x = 0; x < infoBar.cols; x++)
                {
                    intensity = infoBar.at<Vec3f>(Point(x, y));
                    if (x > 16 && x < 64) {
                        if (y < 16 || y > 502) {
                            blue = 1.0;
                            red = 1.0;
                            green = 1.0;
                        }
                        else if (y < 22 || y > 496) {
                            red = 0.0;
                            blue = 0.0;
                            green = 0.0;
                        }
                        else {
                            //blue = ((MID_POINT / 65536.0) - (float)y / (infoBar.rows - 32)) / (1 - (MID_POINT / 65536.0));
                            //red = ((float)y / (infoBar.rows - 32) - (MID_POINT / 65536.0)) / (1 - (MID_POINT / 65536.0));
                            blue = (0.5 - (float)y / (infoBar.rows - 32)) / (1 - 0.5);
                            red = ((float)y / (infoBar.rows - 32) - 0.5) / (1 - 0.5);
                            green = 0.0;
                        }
                    }
                    else if ((x > 10 && x <= 16) || (x >= 64 && x < 70)) {
                        if (y < 16 || y > 502) {
                            blue = 1.0;
                            red = 1.0;
                            green = 1.0;
                        }
                        else {
                            red = 0.0;
                            blue = 0.0;
                            green = 0.0;
                        }
                    }
                    else {
                        red = 1.0;
                        blue = 1.0;
                        green = 1.0;
                    }
                    intensity.val[0] = blue;
                    intensity.val[1] = green;
                    intensity.val[2] = red;
                    infoBar.at<Vec3f>(Point(x, y)) = intensity;
                }
            }

            float num_low = (0 - 1.7) / (0.00023333333 * Gain);
            float num_high = (2.45 - 1.7) / (0.0002333333 * Gain);
            cv::putText(infoBar, "Gauss", Point(80, 10), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
            cv::putText(infoBar, format("%.2f", num_low), Point(80, 26), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
            cv::putText(infoBar, format("%.2f", num_high), Point(80, 482), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
            cv::putText(infoBar, format("%.2f", (((float)val[m_y * 16 + m_x] - mid_point[m_y * 16 + m_x] / 10.92) * 2.45 / 4095) / 9.1 / 0.00023333333), Point(90, 256), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));

            cv::resize(imageF, imageF, Size(512, 512), 1, 1, INTER_LINEAR);
            if (arrow) {
                arrow_draw(imageF, Scalar(1.0, 1.0, 1.0), val, mid_point, 1000 / Gain);
            }
            if (GetKeyState('W') & 0x8000) {
                Gain += 0.5;
            }
            if (GetKeyState('S') & 0x8000) {
                Gain -= 0.5;
            }
            if (GetKeyState('R') & 0x8000) {
                if (!Rec) {
                    Video.open(format("Videos/video%d.avi", saved_videos), VideoWriter::fourcc('M', 'J', 'P', 'G'), 20, Size(512, 512), true);
                    saved_videos++;
                }
                Rec = true;
            }
            if (GetKeyState('T') & 0x8000) {
                if (Rec) {
                    Video.release();
                }
                Rec = false;
            }
            if (GetKeyState(' ') & 0x8000) {
                if (freez_state == 0) {
                    freez = ~freez;
                }
                freez_state = 1;
            }
            else {
                freez_state = 0;
            }
            if (GetKeyState('P') & 0x8000) {
                if (print_state == 0) {
                    Mat imgSave(512, 512, CV_8UC3);
                    imageF.convertTo(imgSave, CV_8UC3, 255);
                    imwrite(format("Images/image%d.jpeg", saved_images), imgSave);
                    saved_images++;
                }
                print_state = 1;
            }
            else {
                print_state = 0;
            }
            if (Rec) {
                Mat imgframe(Size(512, 512), CV_8UC3);
                imageF.convertTo(imgframe, CV_8UC3,255,0);
                //Video.write(imgframe);
                Video << imgframe;
                
            }
            if (GetKeyState('A') & 0x8000) {
                if (avg_state == 0) {
                    avg = ~avg;
                }
                avg_state = 1;
                avg_frame = 0;
                memset(avg_val, 0, 256 * sizeof(uint64_t));
            }
            else {
                avg_state = 0;
            }
            if (GetKeyState('E') & 0x8000) {
                if (arrow_state == 0) {
                    arrow = ~arrow;
                }
                arrow_state = 1;
            }
            else {
                arrow_state = 0;
            }
            if (GetKeyState('C') & 0x8000) {
                ofstream calFile;
                calFile.open("cal.txt", ios::out | ios::trunc);
                if (calFile.is_open()) {
                    for (int i = 0; i < 256; i++) {
                        calFile << (int)(val[i] * 10.92)<< "\n";
                    }
                }
                calFile.close();
            }
            if (GetKeyState('V') & 0x8000) {
                ifstream calFile;
                calFile.open("cal.txt", ios::in);
                if (calFile.is_open()) {
                    for (int i = 0; i < 256; i++) {
                        calFile >> mid_point[i];
                    }
                }
                calFile.close();
            }
            hconcat(imageF, infoBar, imageF);
            if (!freez) {
                cv::imshow("Output", imageF); 
            }
            cv::waitKey(1);
            frame++;
            avg_frame++;
            gotoxy(12, 0);
            cout << (float)(frame * 1000) / (std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()) - ms).count();
            gotoxy(12, 1);
            cout << Gain << "                    ";
            gotoxy(12, 2);
            cout << max << "(" << ((float)max * 2.45 / 4096) / 9.1 / 0.00023333333 << ")              ";
            gotoxy(12, 3);
            cout << min << "(" << ((float)min * 2.45 / 4096) / 9.1 / 0.00023333333 << ")              ";
            gotoxy(12, 4);
            cout << m_y << "    " << m_x << "    " << val[m_y * 16 + m_x] << "(" << (((float)val[m_y * 16 + m_x] - mid_point[m_y * 16 + m_x] / 10.92) * 2.45 / 4095) / 9.1 / 0.00023333333 << ")              ";
            gotoxy(12, 5);
            cout << frame; 
            gotoxy(24, 6);
            if (freez) cout << "PAUSED       ";
            else cout << "RUNNING     ";
            gotoxy(24, 7);
            cout << saved_images;
            gotoxy(24, 8);
            if (Rec) cout << "RECORDING          ";
            else cout << "NOT RECORDING    ";
            gotoxy(24, 9);
            cout << saved_videos;
            gotoxy(24, 10);
            if (arrow) cout << "ON ";
            else cout << "OFF";
            gotoxy(24, 11);
            if (avg) cout << "ON ";
            else cout << "OFF ";
        }
        counter = 0;
     }
    return 0;
}