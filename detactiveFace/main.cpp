//
//  main.cpp
//  detactiveFace
//
//  Created by Kazuo Sato on 2014/11/21.
//  Copyright (c) 2014年 flyingCircus. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>

//boost
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

//openCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

namespace fs = boost::filesystem;
std::string dirctoryPath = "/Users/KSato/Documents/imageProcessing/";
std::string dirctoryStrPath = "/Users/KSato/Documents/imageProcessing/detactiveFace/";
std::string testDetectiveFace = "detectiveFace";
boost::system::error_code error;
void mkdirFolder(std::string);
void getFileNameInDirectory(std::string dir, std::vector<std::string>& fileList);
void displayImage(cv::Mat& img, std::string frameName);
int detactiveFaceAnalysis(cv::Mat& img, bool marking);
void moveFile(std::string path1, std::string path2);

int main(int argc, const char * argv[]) {

    std::vector<std::string> fileNameList;
    std::vector<cv::Mat> imgList;
    int faceCount = 0;
    
    //選り分けた画像を保管するフォルダ
    mkdirFolder(dirctoryStrPath + "faceImage");
    mkdirFolder(dirctoryStrPath + "notFaceImage");
    
    getFileNameInDirectory(dirctoryStrPath + "srcImage", fileNameList);
    
    for(int i = 0; i < fileNameList.size() ;i++)
        imgList.push_back(cv::imread(dirctoryStrPath + "srcImage/" + fileNameList.at(i), 1));
  
    //顔認識
    for(int i = 0; i < imgList.size() ;i++){
        faceCount = detactiveFaceAnalysis(imgList.at(i), true);
        if(faceCount > 0){
            moveFile(dirctoryStrPath + "srcImage/" + fileNameList.at(i),
                     dirctoryStrPath + "faceImage/" + fileNameList.at(i));
        }else{
            moveFile(dirctoryStrPath + "srcImage/" + fileNameList.at(i),
                     dirctoryStrPath + "notFaceImage/" + fileNameList.at(i));
        }
    }
    
    std::cout << "仕分け終了" << std::endl;
    cv::waitKey(0);
    
    return 0;
}

/*
 * 顔認識
 * 返り値は顔の数
 */
int detactiveFaceAnalysis(cv::Mat& img, bool marking){

    double scale = 1.0;
    cv::Mat gray, smallImg(cv::saturate_cast<int>(img.rows/scale), cv::saturate_cast<int>(img.cols/scale), CV_8UC1);
    // グレースケール画像に変換
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    // 処理時間短縮のために画像を縮小
    cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
    cv::equalizeHist( smallImg, smallImg);
    
    // 分類器の読み込み
    std::string cascadeName = "/usr/local/Cellar/opencv/2.4.9/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"; // Haar-like
    cv::CascadeClassifier cascade;
    if(!cascade.load(cascadeName))
        return -1;
    
    std::vector<cv::Rect> faces;
    /// マルチスケール（顔）探索xo
    // 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
    cascade.detectMultiScale(smallImg, faces,
                             1.1, 2,
                             CV_HAAR_SCALE_IMAGE,
                             cv::Size(30, 30));
    
    // 結果の描画
    if(marking){
        std::vector<cv::Rect>::const_iterator r = faces.begin();
        for(; r != faces.end(); ++r) {
            cv::Point center;
            int radius;
            center.x = cv::saturate_cast<int>((r->x + r->width*0.5)*scale);
            center.y = cv::saturate_cast<int>((r->y + r->height*0.5)*scale);
            radius = cv::saturate_cast<int>((r->width + r->height)*0.25*scale);
            cv::circle(img, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
        }
    }
    return (int)faces.size();
}


/*
 * 画像を表示
 *
 */
void displayImage(cv::Mat& img, std::string frameName){
    cv::namedWindow(frameName, CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    cv::imshow(frameName, img);
}


/*
 * フォルダ作成
 *
 */
void mkdirFolder(std::string path){
    const bool result = fs::exists(path, error);
    if (!result || error) {
        std::cout << path  + "フォルダ作成" << std::endl;
        fs::create_directory(path);
    }
}

/*
 * フォルダ内のファイル名を取得
 *
 */
void getFileNameInDirectory(std::string dir, std::vector<std::string>& fileList){
    const fs::path path(dir);
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::directory_iterator(path),
                                                    fs::directory_iterator())) {
        if (!fs::is_directory(p)){
            fileList.push_back(p.filename().c_str());
        }
    }
}

/*
 *  ファイル移動
 *
 */
void moveFile(std::string path1, std::string path2){
    const fs::path path(path1);
    const fs::path dest(path2);
    
    try {
        fs::rename(path, dest);
    }
    catch (fs::filesystem_error& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }
}