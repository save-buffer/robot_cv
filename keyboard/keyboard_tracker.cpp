#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <unordered_map>

using namespace cv;
//For compatibility with opencv2
namespace cv
{
    using std::vector;
}

//./keyboard_tracker /mnt/c/Users/Sasha/Downloads/keyboard.png

Mat color_corrected(Mat img)
{
    Mat lab_img;
    cvtColor(img, lab_img, CV_BGR2Lab);
    vector<Mat> lab_planes(3);
    split(lab_img, lab_planes);

    Ptr<CLAHE> clahe = createCLAHE();
    clahe->setClipLimit(4);
    Mat dst;
    clahe->apply(lab_planes[0], dst);
    dst.copyTo(lab_planes[0]);
    merge(lab_planes, lab_img);

    Mat corrected;
    cvtColor(lab_img, corrected, CV_Lab2BGR);
    return(corrected);
}

Mat threshold_image(Mat img)
{
    Mat hsv(img.rows, img.cols, CV_8UC3);
    cvtColor(img, hsv, CV_BGR2HSV);
    Mat thresh(img.rows, img.cols, CV_8UC1);
    inRange(hsv, Scalar(0.11 * 256, 0.60 * 256, 0.20 * 256), Scalar(0.14 * 256, 1.0 * 255.0, 1.0 * 256), thresh);
    return(thresh);
}

Mat morphed_img(Mat mask)
{
    Mat se21 = getStructuringElement(MORPH_RECT, Size(21, 21));
    Mat se11 = getStructuringElement(MORPH_RECT, Size(11, 11));

    morphologyEx(mask, mask, MORPH_CLOSE, se21);
    morphologyEx(mask, mask, MORPH_OPEN, se11);

    GaussianBlur(mask, mask, Size(15, 15), 0, 0);
    return(mask);   
}

Mat overall_filter(Mat img)
{
//    Mat corrected = color_corrected(img);
    Mat mask = threshold_image(img);
    Mat filtered = morphed_img(mask);
    return(filtered);
}

void hough_circles_identifier(Mat src)
{
    Mat hough_in = overall_filter(src);
    vector<Vec3f> circles;
    /// Apply the Hough Transform to find the circles
    HoughCircles(hough_in, circles, CV_HOUGH_GRADIENT, 1.1, hough_in.rows/10, 100, 40, 0, 0);

    printf("circles: %lu\n", circles.size());
    /// Draw the circles detected
    for(size_t i = 0; i < circles.size(); i++)
    {
	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	int radius = cvRound(circles[i][2]);
	// circle center
	circle(src, center, 3, Scalar(0,255,0), -1, 8, 0);
	// circle outline
	circle(src, center, radius, Scalar(0, 0, 255), 3, 8, 0);
    }

    /// Show your results
    namedWindow("Hough Circle Transform Demo", CV_WINDOW_AUTOSIZE);
    imshow("Hough Circle Transform Demo", hough_in);
}

/*
  take your post-processed images and get the connected components, draw a bounding square around them,
  take the inscribed circle of the bounding square, then calculate a coverage overlap
  that will give you a "percent like a circle" metric
  and you can tune that threshold to whatever is best for your application
 */
void connected_components_identifier(Mat src)
{
    Mat filtered = overall_filter(src);
    Mat labels;
    int components = connectedComponents(filtered, labels);
    printf("%d connected components\n", components);
    vector<Rect> rectangles;
    for(int i = 1; i < components; i++)
    {
	Mat component_i;
	inRange(labels, Scalar(i), Scalar(i), component_i);
	Rect r = boundingRect(component_i);
	
	Point center = (r.tl() + r.br()) / 2;
	int radius = abs(r.tl().y - center.y);
	Mat circ(component_i.size(), component_i.type());
	circ = Scalar(0, 0, 0);
	circle(circ, center, radius, i, -1);

	Mat intersection;
	bitwise_and(circ, component_i, intersection);
	Mat union_;
	bitwise_or(circ, component_i, union_);

	printf("intersection: %d\n", countNonZero(intersection));
	printf("union: %d\n", countNonZero(union_));

	float iou = (float)countNonZero(intersection) / (float)countNonZero(union_);

	printf("%.3f iou\n", iou);

#define CIRCLE_THRESH 0.8f

	if(iou >= CIRCLE_THRESH)
	    rectangles.push_back(r);

#undef CIRCLE_THRESH
    }

    for(Rect r : rectangles)
    {
	rectangle(src, r.tl(), r.br(), Scalar(255, 0, 0), 1);
    }
    
    normalize(labels, labels, 0, 255, NORM_MINMAX, CV_8U);
    namedWindow("Connected Components Transform", CV_WINDOW_AUTOSIZE);
    imshow("Connected Components Transform", src);
}

Mat fft(Mat gray)
{
    Mat fft;
    gray.convertTo(fft, CV_32F);
    dft(fft, fft, DFT_SCALE | DFT_COMPLEX_OUTPUT);
    return(fft);
    
    Mat padded;
    int m = getOptimalDFTSize(gray.rows);
    int n = getOptimalDFTSize(gray.cols);
    copyMakeBorder(gray, padded, m - gray.rows, 0, n - gray.cols, 0, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
    Mat complex;
    merge(planes, 2, complex);
    dft(complex, complex);
    split(complex, planes);
    magnitude(planes[0], planes[1], planes[0]);
    Mat mag = planes[0];
    mag += Scalar::all(1);
    log(mag, mag);
    mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));
    normalize(mag, mag, 0, 1, CV_MINMAX);
    return(mag);
}

Mat ifft(Mat fft)
{
    Mat ifft;
    dft(fft, ifft, DFT_INVERSE | DFT_REAL_OUTPUT);
    ifft.convertTo(ifft, CV_8U);
    return(ifft);
}

Mat switch_quadrants(Mat src)
{
    Mat mag;
    src.copyTo(mag);
    int cx = mag.cols / 2;
    int cy = mag.rows / 2;

    Mat q0(mag, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(mag, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(mag, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(mag, Rect(cx, cy, cx, cy)); // Bottom-Right
    
    Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);
        
    return(mag);
}

Mat high_pass(Mat src)
{
#define HIGHPASS_THRESH 0.0f
    Mat fft_img = fft(src);
    threshold(fft_img, fft_img, HIGHPASS_THRESH, 1.0f, THRESH_TOZERO);
    Mat orig = ifft(fft_img);
    return(orig);
#undef HIGHPASS_THRESH
}

void laplacian_keyboard_identifier(Mat src)
{
    Mat gray;    
    cvtColor(src, gray, COLOR_BGR2GRAY);

    GaussianBlur(gray, gray, Size(3, 3), 3);
    Mat orig;
    
    Laplacian(gray, orig, CV_8U, 13);

    Mat se1 = getStructuringElement(MORPH_RECT, Size(1, 1));
    Mat se3 = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat se5 = getStructuringElement(MORPH_RECT, Size(5, 5));
    Mat se7 = getStructuringElement(MORPH_RECT, Size(7, 7));
    Mat se9 = getStructuringElement(MORPH_RECT, Size(9, 9));
    Mat se11 = getStructuringElement(MORPH_RECT, Size(11, 11));
    Mat se13 = getStructuringElement(MORPH_RECT, Size(13, 13));

    morphologyEx(orig, orig, MORPH_ERODE, se5);
    morphologyEx(orig, orig, MORPH_DILATE, se5);

    Mat key_image;
    int components = connectedComponents(orig, key_image);
    printf("found %d components\n", components);
    for(int i = 1; i < components; i++)
    {
#define HW_THRESH 2.0f
	Mat component_i;
	inRange(key_image, Scalar(i), Scalar(i), component_i);
	Rect r = boundingRect(component_i);
	if((float)r.size().height / (float)r.size().width > HW_THRESH)
	{
	    component_i /= i;
	    component_i *= 255;
	    orig -= component_i;
	}
#undef HW_THRESH
    }
//    morphologyEx(orig, orig, MORPH_DILATE, se5);
    morphologyEx(orig, orig, MORPH_ERODE, se5);
    morphologyEx(orig, orig, MORPH_DILATE, se13);
    
    components = connectedComponents(orig, key_image);
    printf("found %d components\n", components);

    Mat color_orig;
    for(int i = 1; i < components; i++)
    {
	Mat component_i;
	inRange(key_image, Scalar(i), Scalar(i), component_i);
	Rect r = boundingRect(component_i);
	rectangle(src, r.tl(), r.br(), Scalar(0, 0, 255), 1);
    }
    namedWindow("Keyboard Identifier");
    namedWindow("Morphology");
    imshow("Keyboard Identifier", src);
    imshow("Morphology", orig);
}

Mat src;
Mat gray_orig;
Mat gray;
int thresh = 30;
int max_thresh = 255;

int blur_std = 0;
int max_blur_std = 20;

int blur_size = 1;
int max_blur_size = 10;

void contour_keyboard_tracker()
{
    Mat edges;
    Canny(gray, edges, thresh, thresh * 3, 3);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    findContours(edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

    Mat contour_img = Mat::zeros(edges.size(), CV_8UC3); 
    for(int i = 0; i < contours.size(); i++)
    {
	drawContours(contour_img, contours, i, Scalar(0, 0, 255), 1, 8, hierarchy, 0, Point());
    }

    Mat gray_contours;
    cvtColor(contour_img, gray_contours, COLOR_BGR2GRAY);
    
    Mat se1 = getStructuringElement(MORPH_RECT, Size(1, 1));
    Mat se3 = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat se5 = getStructuringElement(MORPH_RECT, Size(5, 5));
    Mat se7 = getStructuringElement(MORPH_RECT, Size(7, 7));
    Mat se9 = getStructuringElement(MORPH_RECT, Size(9, 9));
    Mat se11 = getStructuringElement(MORPH_RECT, Size(11, 11));
    Mat se13 = getStructuringElement(MORPH_RECT, Size(13, 13));
    
    morphologyEx(gray_contours, gray_contours, MORPH_DILATE, se5);
//    morphologyEx(gray_contours, gray_contours, MORPH_ERODE, se3);

    gray_contours = Scalar::all(255) - gray_contours;
    inRange(gray_contours, Scalar(255), Scalar(255), gray_contours);
    morphologyEx(gray_contours, gray_contours, MORPH_DILATE, se5);
    morphologyEx(gray_contours, gray_contours, MORPH_ERODE, se3);


    Mat key_image;
    int components = connectedComponents(gray_contours, key_image);
    printf("found %d components\n", components);
    for(int i = 1; i < components; i++)
    {
#define HW_THRESH 2.0f
	Mat component_i;
	inRange(key_image, Scalar(i), Scalar(i), component_i);
	Rect r = boundingRect(component_i);
	if((float)r.size().height / (float)r.size().width > HW_THRESH)
	{
	    component_i /= i;
	    component_i *= 255;
	    gray_contours -= component_i;
	}
#undef HW_THRESH
    }

    morphologyEx(gray_contours, gray_contours, MORPH_ERODE, se3);
    morphologyEx(gray_contours, gray_contours, MORPH_DILATE, se5);
    morphologyEx(gray_contours, gray_contours, MORPH_ERODE, se5);
    morphologyEx(gray_contours, gray_contours, MORPH_DILATE, se3);
    
    components = connectedComponents(gray_contours, key_image);
    printf("found %d components\n", components);

    Mat color_orig(src);
    for(int i = 1; i < components; i++)
    {
	Mat component_i;
	inRange(key_image, Scalar(i), Scalar(i), component_i);
	Rect r = boundingRect(component_i);
	rectangle(color_orig, r.tl(), r.br(), Scalar(0, 0, 255), 1);
    }

    
    namedWindow("Contours");
    namedWindow("Gray");
    namedWindow("Output");
    imshow("Contours", contour_img);
    imshow("Gray", gray_contours);
    imshow("Output", color_orig);
}

void keyboard_identifier(Mat src)
{
    contour_keyboard_tracker();
}

void thresh_callback(int, void *)
{
    contour_keyboard_tracker();
}

void blur_callback(int, void *)
{
    gray_orig.copyTo(gray);
    int real_blur_size = 2 * blur_size + 1;
    blur(gray, gray, Size(real_blur_size, real_blur_size));
//    GaussianBlur(gray, gray, Size(real_blur_size, real_blur_size), blur_std);
    contour_keyboard_tracker();
}

int main(int argc, char** argv)
{
    /// Read the image
    src = imread(argv[1], 1);
    
    if(!src.data)
	return(-1);

    cvtColor(src, gray_orig, COLOR_BGR2GRAY);

    namedWindow("Source");
    imshow("Source", src);
    createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
    createTrackbar(" Blur std:", "Source", &blur_std, max_blur_std, blur_callback);
    createTrackbar(" Blur size:", "Source", &blur_size, max_blur_size, blur_callback);
    blur_callback(0, 0);
		 
//    keyboard_identifier(src);
    
    waitKey(0);
    return(0);
}
