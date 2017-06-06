#ifndef __COMMON_H
#define __COMMON_H

    #include "lstStructs.h"
    #include <QString>
    #include <QDebug>
    #include <QTime>
    #include <QEventLoop>
    #include <QCoreApplication>

    //#include <highgui.h>
    #include <opencv2/highgui/highgui.hpp>

    #include <customQMatrix3x3.h>

    #include <QFileInfo>

    QPoint *calibPoint( QPoint *point, lstDoubleAxisCalibration *calib );

    double funcDet2x2(double **M);

    double vectorMax(double *vector, int len);
    double vectorMaxQListQString(QList<QString> lst);

    QImage bilinearInterpolationQImage(QImage img);

    //void funcSourcePixToDiffPix(strDiffPix *diffPix, lstDoubleAxisCalibration *calSett );

    void funcPrintCalibration(lstDoubleAxisCalibration *calibSettings);

    bool funcGetCalibration(lstDoubleAxisCalibration* calibSettings);

    void funcObtSettings( structSettings * lstSettings);

    void funcShowMsg( QString title, QString msg );

    QString funcShowSelDir(QString path);

    void funcShowFileError(int error, QString fileName);

    void funcPrintFirst(int n, int max, char *buffer);

    void funcObtainIP(char* host);

    bool funcIsIP( std::string ipCandidate );

    int connectSocket( structCamSelected *camSelected );    

    void QtDelay( unsigned int ms );

    int xyToIndex(int x, int y, int w);

    int *indexToxy( int index, int w, int h );

    int funcShowMsgYesNo( QString title, QString msg );

    bool funcGetRaspParamFromXML(structRaspcamSettings *raspcamSettings , QString filePath);

    void funcRGB2XYZ(colSpaceXYZ *spaceXYZ, float Red, float Green, float Blue);

    bool saveFile( QString fileName, QString contain );

    //IplImage *funcGetImgFromCam(int usb, int stabMs );

    QString readAllFile( QString filePath );

    int fileIsValid(QString fileContain);

    QString readFileParam(QString fileName);

    bool funGetSquareXML( QString fileName, squareAperture *squareParam );

    QImage* IplImage2QImage(IplImage *iplImg);

    colorAnalyseResult *funcAnalizeImage(QImage *img );

    linearRegresion *funcCalcLinReg(float *X);

    linearRegresion funcLinearRegression(double *X, double *Y, int numItems );

    QImage funcRotateImage(QString filePath, float rotAngle);

    void funcQStringToSensitivities(QString txt, QList<double> *p);

    void funcTransPix(calcAndCropSnap *calStruct , int w, int h, int W, int H);

    bool saveBinFile(unsigned long datasize, unsigned char *dataPtr, QString directory);

    int funcDeleteFile( QString fileName );

    QString funcRemoveFileNameFromPath( QString Path );

    customQMatrix3x3 matMultiply(QMatrix3x4 *M1, QMatrix4x3 *M2);
    QMatrix3x4 matMultiply(customQMatrix3x3 *M1, QMatrix3x4 *M2);
    QVector3D matMultiply(QMatrix3x4 *M1, QVector4D *M2);


    void funcOpenFolder(QString path);

    int funcAccountFilesInDir(QString Dir);

    void funcClearDirFolder(QString path);

    int funcPrintRectangle(QString title, squareAperture *rectangle);

    QList<QFileInfo> funcListFilesInDir(QString Dir);

    void calcDiffProj(strDiffProj *diffProj, lstDoubleAxisCalibration *daCalib);

    QString timeToQString(unsigned int totMilli);

    bool funGetSlideSettingsXML(QString fileName, strReqImg *reqImg );

    int saveBinFile_From_u_int8_T( std::string fileName, uint8_t *data, size_t len);

    int fileExists(QString fileName );

    int funcExecuteCommand( QString command );

    //inline int align(int size, int align);
    //IplImage *QImageToIplImage(const QImage * qImage);
    //QImage *IplImageToQImage(const IplImage * iplImage, uchar **data,
    //                         double mini, double maxi);


    //
    //Variable area
    //
    static cameraResolution* camRes = (cameraResolution*)malloc(sizeof(cameraResolution));



#endif // __COMMON_H
