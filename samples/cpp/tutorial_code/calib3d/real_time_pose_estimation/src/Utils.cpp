/*
 * Utils.cpp
 *
 *  Created on: Mar 28, 2014
 *      Author: Edgar Riba
 */

#include <iostream>

#include "PnPProblem.h"
#include "ModelRegistration.h"
#include "Utils.h"

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/3d.hpp>
#include <opencv2/flann.hpp>
#if defined (HAVE_OPENCV_XFEATURES2D)
#include <opencv2/xfeatures2d.hpp>
#endif

// For text
const int fontFace = cv::FONT_ITALIC;
const double fontScale = 0.75;
const int thickness_font = 2;

// For circles
const int lineType = 8;
const int radius = 4;

// Draw a text with the question point
void drawQuestion(cv::Mat image, cv::Point3f point, cv::Scalar color)
{
    std::string x = IntToString((int)point.x);
    std::string y = IntToString((int)point.y);
    std::string z = IntToString((int)point.z);

    std::string text = " Where is point (" + x + ","  + y + "," + z + ") ?";
    cv::putText(image, text, cv::Point(25,50), fontFace, fontScale, color, thickness_font, 8);
}

// Draw a text with the number of entered points
void drawText(cv::Mat image, std::string text, cv::Scalar color)
{
    cv::putText(image, text, cv::Point(25,50), fontFace, fontScale, color, thickness_font, 8);
}

// Draw a text with the number of entered points
void drawText2(cv::Mat image, std::string text, cv::Scalar color)
{
    cv::putText(image, text, cv::Point(25,75), fontFace, fontScale, color, thickness_font, 8);
}

// Draw a text with the frame ratio
void drawFPS(cv::Mat image, double fps, cv::Scalar color)
{
    std::string fps_str = cv::format("%.2f FPS", fps);
    cv::putText(image, fps_str, cv::Point(500,50), fontFace, fontScale, color, thickness_font, 8);
}

// Draw a text with the frame ratio
void drawConfidence(cv::Mat image, double confidence, cv::Scalar color)
{
    std::string conf_str = IntToString((int)confidence);
    std::string text = conf_str + " %";
    cv::putText(image, text, cv::Point(500,75), fontFace, fontScale, color, thickness_font, 8);
}

// Draw a text with the number of entered points
void drawCounter(cv::Mat image, int n, int n_max, cv::Scalar color)
{
    std::string n_str = IntToString(n);
    std::string n_max_str = IntToString(n_max);
    std::string text = n_str + " of " + n_max_str + " points";
    cv::putText(image, text, cv::Point(500,50), fontFace, fontScale, color, thickness_font, 8);
}

// Draw the points and the coordinates
void drawPoints(cv::Mat image, std::vector<cv::Point2f> &list_points_2d, std::vector<cv::Point3f> &list_points_3d, cv::Scalar color)
{
    for (unsigned int i = 0; i < list_points_2d.size(); ++i)
    {
        cv::Point2f point_2d = list_points_2d[i];
        cv::Point3f point_3d = list_points_3d[i];

        // Draw Selected points
        cv::circle(image, point_2d, radius, color, -1, lineType );

        std::string idx = IntToString(i+1);
        std::string x = IntToString((int)point_3d.x);
        std::string y = IntToString((int)point_3d.y);
        std::string z = IntToString((int)point_3d.z);
        std::string text = "P" + idx + " (" + x + "," + y + "," + z +")";

        point_2d.x = point_2d.x + 10;
        point_2d.y = point_2d.y - 10;
        cv::putText(image, text, point_2d, fontFace, fontScale*0.5, color, thickness_font, 8);
    }
}

// Draw only the 2D points
void draw2DPoints(cv::Mat image, std::vector<cv::Point2f> &list_points, cv::Scalar color)
{
    for( size_t i = 0; i < list_points.size(); i++)
    {
        cv::Point2f point_2d = list_points[i];

        // Draw Selected points
        cv::circle(image, point_2d, radius, color, -1, lineType );
    }
}

// Draw an arrow into the image
void drawArrow(cv::Mat image, cv::Point2i p, cv::Point2i q, cv::Scalar color, int arrowMagnitude, int thickness, int line_type, int shift)
{
    //Draw the principle line
    cv::line(image, p, q, color, thickness, line_type, shift);
    const double PI = CV_PI;
    //compute the angle alpha
    double angle = atan2((double)p.y-q.y, (double)p.x-q.x);
    //compute the coordinates of the first segment
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle + PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle + PI/4));
    //Draw the first segment
    cv::line(image, p, q, color, thickness, line_type, shift);
    //compute the coordinates of the second segment
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle - PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle - PI/4));
    //Draw the second segment
    cv::line(image, p, q, color, thickness, line_type, shift);
}

// Draw the 3D coordinate axes
void draw3DCoordinateAxes(cv::Mat image, const std::vector<cv::Point2f> &list_points2d)
{
    cv::Scalar red(0, 0, 255);
    cv::Scalar green(0,255,0);
    cv::Scalar blue(255,0,0);
    cv::Scalar black(0,0,0);

    cv::Point2i origin = list_points2d[0];
    cv::Point2i pointX = list_points2d[1];
    cv::Point2i pointY = list_points2d[2];
    cv::Point2i pointZ = list_points2d[3];

    drawArrow(image, origin, pointX, red, 9, 2);
    drawArrow(image, origin, pointY, green, 9, 2);
    drawArrow(image, origin, pointZ, blue, 9, 2);
    cv::circle(image, origin, radius/2, black, -1, lineType );
}

// Draw the object mesh
void drawObjectMesh(cv::Mat image, const Mesh *mesh, PnPProblem *pnpProblem, cv::Scalar color)
{
    std::vector<std::vector<int> > list_triangles = mesh->getTrianglesList();
    for( size_t i = 0; i < list_triangles.size(); i++)
    {
        std::vector<int> tmp_triangle = list_triangles.at(i);

        cv::Point3f point_3d_0 = mesh->getVertex(tmp_triangle[0]);
        cv::Point3f point_3d_1 = mesh->getVertex(tmp_triangle[1]);
        cv::Point3f point_3d_2 = mesh->getVertex(tmp_triangle[2]);

        cv::Point2f point_2d_0 = pnpProblem->backproject3DPoint(point_3d_0);
        cv::Point2f point_2d_1 = pnpProblem->backproject3DPoint(point_3d_1);
        cv::Point2f point_2d_2 = pnpProblem->backproject3DPoint(point_3d_2);

        cv::line(image, point_2d_0, point_2d_1, color, 1);
        cv::line(image, point_2d_1, point_2d_2, color, 1);
        cv::line(image, point_2d_2, point_2d_0, color, 1);
    }
}

// Computes the norm of the translation error
double get_translation_error(const cv::Mat &t_true, const cv::Mat &t)
{
    return cv::norm( t_true - t );
}

// Computes the norm of the rotation error
double get_rotation_error(const cv::Mat &R_true, const cv::Mat &R)
{
    cv::Mat error_vec, error_mat;
    error_mat = -R_true * R.t();
    Rodrigues(error_mat, error_vec);

    return cv::norm(error_vec);
}

// Converts a given Rotation Matrix to Euler angles
// Convention used is Y-Z-X Tait-Bryan angles
// Reference code implementation:
// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
cv::Mat rot2euler(const cv::Mat & rotationMatrix)
{
    cv::Mat euler(3,1,CV_64F);

    double m00 = rotationMatrix.at<double>(0,0);
    double m02 = rotationMatrix.at<double>(0,2);
    double m10 = rotationMatrix.at<double>(1,0);
    double m11 = rotationMatrix.at<double>(1,1);
    double m12 = rotationMatrix.at<double>(1,2);
    double m20 = rotationMatrix.at<double>(2,0);
    double m22 = rotationMatrix.at<double>(2,2);

    double bank, attitude, heading;

    // Assuming the angles are in radians.
    if (m10 > 0.998) { // singularity at north pole
        bank = 0;
        attitude = CV_PI/2;
        heading = atan2(m02,m22);
    }
    else if (m10 < -0.998) { // singularity at south pole
        bank = 0;
        attitude = -CV_PI/2;
        heading = atan2(m02,m22);
    }
    else
    {
        bank = atan2(-m12,m11);
        attitude = asin(m10);
        heading = atan2(-m20,m00);
    }

    euler.at<double>(0) = bank;
    euler.at<double>(1) = attitude;
    euler.at<double>(2) = heading;

    return euler;
}

// Converts a given Euler angles to Rotation Matrix
// Convention used is Y-Z-X Tait-Bryan angles
// Reference:
// https://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm
cv::Mat euler2rot(const cv::Mat & euler)
{
    cv::Mat rotationMatrix(3,3,CV_64F);

    double bank = euler.at<double>(0);
    double attitude = euler.at<double>(1);
    double heading = euler.at<double>(2);

    // Assuming the angles are in radians.
    double ch = cos(heading);
    double sh = sin(heading);
    double ca = cos(attitude);
    double sa = sin(attitude);
    double cb = cos(bank);
    double sb = sin(bank);

    double m00, m01, m02, m10, m11, m12, m20, m21, m22;

    m00 = ch * ca;
    m01 = sh*sb - ch*sa*cb;
    m02 = ch*sa*sb + sh*cb;
    m10 = sa;
    m11 = ca*cb;
    m12 = -ca*sb;
    m20 = -sh*ca;
    m21 = sh*sa*cb + ch*sb;
    m22 = -sh*sa*sb + ch*cb;

    rotationMatrix.at<double>(0,0) = m00;
    rotationMatrix.at<double>(0,1) = m01;
    rotationMatrix.at<double>(0,2) = m02;
    rotationMatrix.at<double>(1,0) = m10;
    rotationMatrix.at<double>(1,1) = m11;
    rotationMatrix.at<double>(1,2) = m12;
    rotationMatrix.at<double>(2,0) = m20;
    rotationMatrix.at<double>(2,1) = m21;
    rotationMatrix.at<double>(2,2) = m22;

    return rotationMatrix;
}

// Converts a given string to an integer
int StringToInt ( const std::string &Text )
{
    std::istringstream ss(Text);
    int result;
    return ss >> result ? result : 0;
}

// Converts a given float to a string
std::string FloatToString ( float Number )
{
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

// Converts a given integer to a string
std::string IntToString ( int Number )
{
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

void createFeatures(const std::string &featureName, int numKeypoints, cv::Ptr<cv::Feature2D> &detector, cv::Ptr<cv::Feature2D> &descriptor)
{
    if (featureName == "ORB")
    {
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
    }
    else if (featureName == "KAZE")
    {
#if defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::KAZE::create();
        descriptor = cv::xfeatures2d::KAZE::create();
#else
        std::cout << "xfeatures2d module is not available." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
    else if (featureName == "AKAZE")
    {
#if defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::AKAZE::create();
        descriptor = cv::xfeatures2d::AKAZE::create();
#else
        std::cout << "xfeatures2d module is not available." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
    else if (featureName == "BRISK")
    {
#if defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::BRISK::create();
        descriptor = cv::xfeatures2d::BRISK::create();
#else
        std::cout << "xfeatures2d module is not available." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
    else if (featureName == "SIFT")
    {
        detector = cv::SIFT::create();
        descriptor = cv::SIFT::create();
    }
    else if (featureName == "SURF")
    {
#if defined (OPENCV_ENABLE_NONFREE) && defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::SURF::create(100, 4, 3, true);   //extended=true
        descriptor = cv::xfeatures2d::SURF::create(100, 4, 3, true); //extended=true
#else
        std::cout << "xfeatures2d module is not available or nonfree is not enabled." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
    else if (featureName == "BINBOOST")
    {
#if defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::KAZE::create();
        descriptor = cv::xfeatures2d::BoostDesc::create();
#else
        std::cout << "xfeatures2d module is not available." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
    else if (featureName == "VGG")
    {
#if defined (HAVE_OPENCV_XFEATURES2D)
        detector = cv::xfeatures2d::KAZE::create();
        descriptor = cv::xfeatures2d::VGG::create();
#else
        std::cout << "xfeatures2d module is not available." << std::endl;
        std::cout << "Default to ORB." << std::endl;
        detector = cv::ORB::create(numKeypoints);
        descriptor = cv::ORB::create(numKeypoints);
#endif
    }
}

cv::Ptr<cv::DescriptorMatcher> createMatcher(const std::string &featureName, bool useFLANN)
{
    if (featureName == "ORB" || featureName == "BRISK" || featureName == "AKAZE" || featureName == "BINBOOST")
    {
        if (useFLANN)
        {
            cv::Ptr<cv::flann::IndexParams> indexParams = cv::makePtr<cv::flann::LshIndexParams>(6, 12, 1); // instantiate LSH index parameters
            cv::Ptr<cv::flann::SearchParams> searchParams = cv::makePtr<cv::flann::SearchParams>(50);       // instantiate flann search parameters
            return cv::makePtr<cv::FlannBasedMatcher>(indexParams, searchParams);
        }
        else
        {
            return cv::DescriptorMatcher::create("BruteForce-Hamming");
        }

    }
    else
    {
        if (useFLANN)
        {
            return cv::DescriptorMatcher::create("FlannBased");
        }
        else
        {
            return cv::DescriptorMatcher::create("BruteForce");
        }
    }
}
