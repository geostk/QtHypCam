#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "__common.h"
#include "hypCamAPI.h"

#include <unistd.h>
#include <netdb.h>
#include <QFile>
#include <fstream>
#include <math.h>

//#include <netinet/in.h>
#include <QDebug>
//#include <QPixmap>
#include <QFileInfo>
#include <QFileDialog>
#include <QGraphicsPixmapItem>

#include "graphicsview.h"

#include <QRect>
#include <QRgb>
#include <QProgressBar>

#include <QFormLayout>

#include <selwathtocheck.h>

//OpenCV
#include <highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

//Custom
#include <customline.h>
#include <customrect.h>
#include <selcolor.h>
#include <gencalibxml.h>
#include <rotationfrm.h>
#include <recparamfrm.h>
//#include <generatehypercube.h>
//#include <validatecalibration.h>
#include <selwathtocheck.h>

#include <chosewavetoextract.h>
#include <QRgb>

structSettings *lstSettings = (structSettings*)malloc(sizeof(structSettings));

structCamSelected *camSelected = (structCamSelected*)malloc(sizeof(structCamSelected));

structRaspcamSettings *raspcamSettings = (structRaspcamSettings*)malloc(sizeof(structRaspcamSettings));

GraphicsView *myCanvas;
GraphicsView *canvasSpec;
GraphicsView *canvasCalib;
GraphicsView *canvasAux;

customLine *globalCanvHLine;
customLine *globalCanvVLine;
//customLine *globalTmpLine;

//QString imgPath = "/media/jairo/56A3-A5C4/DatosAVIRIS/CrearHSI/MyDatasets/Philips/HojasFotoVsHojasBiomasaJun2016/200Id/CROPED/100.tif";
//QString impPath = "./imgResources/CIE.png";

QList<QPair<int,int>> *lstBorder;
QList<QPair<int,int>> *lstSelPix;
QList<QPair<int,int>> *lstPixSelAux;

//int tmpRect[4];
calcAndCropSnap calStruct;
bool globaIsRotated;














MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->actionValidCal->trigger();

    funcObtSettings( lstSettings );

    /*
    int algo = 4;
    int *p;
    p = &algo;
    qDebug() << "bool: " << sizeof(bool);
    qDebug() << "char: " << sizeof(char);
    qDebug() << "int8: " << sizeof(u_int8_t);
    qDebug() << "int: " << sizeof(int);
    qDebug() << "int*: " << sizeof(p);
    printf("Puntero %p\n",p);
    printf("Puntero %d\n",p);
    printf("Puntero %d\n",*p);
    */


    /*
    //Fill IP prefix
    //if(_PRELOAD_IP){
        char cIP[15];
        funcObtainIP( cIP );
        QString qsIP(cIP);
        QStringList lstIP = qsIP.split('.');
        qsIP = lstIP.at(0) + "." +lstIP.at(1) + "."+lstIP.at(2) + ".";
        ui->txtIp->setText(qsIP);
    //}
    */


    //Initialize global settings
    camSelected->isConnected    = false;
    camSelected->On             = false;
    camSelected->stream         = false;

    //Initialize camera parameters
    ui->txtCamParamXMLName->setText("raspcamSettings");
    funcGetRaspParamFromXML( raspcamSettings, "./XML/camPerfils/raspcamSettings.xml" );
    funcIniCamParam( raspcamSettings );


    //Create Graphic View Widget
    myCanvas = new GraphicsView;
    canvasSpec = new GraphicsView;
    canvasCalib = new GraphicsView;
    canvasAux = new GraphicsView;
    //QString imgPath = "/media/jairo/56A3-A5C4/DatosAVIRIS/CrearHSI/MyDatasets/Philips/HojasFotoVsHojasBiomasaJun2016/25Id/15.png";
    //funcPutImageIntoGV( myCanvas, imgPath );

    //Initialize points container for free-hand pen tool
    lstBorder = new QList<QPair<int,int>>;
    lstSelPix = new QList<QPair<int,int>>;
    lstPixSelAux = new QList<QPair<int,int>>;

    /*
    //Connect to image
    lstBorder = new QList<QPair<int,int>>;
    lstSelPix = new QList<QPair<int,int>>;
    connect(
                myCanvas,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcAddPoint(QMouseEvent*) )
           );
    ui->progBar->setVisible(false);

    //Connect to calib double axis
    connect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcBeginRect(QMouseEvent*) )
           );
    connect(
                canvasCalib,
                SIGNAL( signalMouseReleased(QMouseEvent*) ),
                this,
                SLOT( funcCalibMouseRelease(QMouseEvent*) )
           );

    //Connect to spec
    connect(
                canvasSpec,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcBeginRect(QMouseEvent*) )
           );
    connect(
                canvasSpec,
                SIGNAL( signalMouseReleased(QMouseEvent*) ),
                this,
                SLOT( funcSpectMouseRelease(QMouseEvent*) )
           );
    */




    //Try to connect to the last IP
    QString lastIP = readAllFile( "./settings/lastIp.hypcam" );
    lastIP.replace("\n","");
    ui->txtIp->setText(lastIP);
    if(_AUTOCONNECT){
        ui->pbAddIp->click();
        ui->tableLstCams->selectRow(0);
        ui->pbConnect->click();
    }

    //Fill the lastsnapshots path as default
    QString lastSnapPath = readAllFile( _PATH_LAST_SNAPPATH );
    lastSnapPath.replace("\n","");
    ui->txtSnapPath->setText(lastSnapPath);

    //Set layout into spectometer
    QFormLayout *layout = new QFormLayout;
    ui->tab_5->setLayout(layout);

    if(_USE_CAM)ui->sbSpecUsb->setValue(1);
    else ui->sbSpecUsb->setValue(0);

    //Enable-Disable buttoms
    //..
    ui->toolBarDraw->setEnabled(false);

    ui->progBar->setValue(0);
    ui->progBar->setVisible(false);

    disableAllToolBars();

    loadImageIntoCanvasEdit(_PATH_DISPLAY_IMAGE, false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::disableAllToolBars(){
    ui->toolBarDraw->setVisible(false);
}

void MainWindow::funcSpectMouseRelease( QMouseEvent *e){
    funcEndRect(e, canvasSpec );
    funcDrawLines(0,0,0,0);
}

void MainWindow::funcCalibMouseRelease( QMouseEvent *e){
    funcEndRect( e, canvasCalib );
    //ui->pbClearCalScene->setText("Clear line");
}

void MainWindow::funcAddPoint( QMouseEvent *e ){

    //Extract pixel's coordinates
    QPair<int,int> tmpPixSel;
    tmpPixSel.first = e->x();
    tmpPixSel.second = e->y();
    lstPixSelAux->append(tmpPixSel);

    //Update politope
    funcUpdatePolitope();

}

bool MainWindow::funcUpdatePolitope(){

    int i, x1, y1, x2, y2, x0, y0;

    //If is the first point
    if( lstPixSelAux->count() < 2 ){
        x1  = lstPixSelAux->at(0).first - 3;//Point +/- error
        y1  = lstPixSelAux->at(0).second - 4;//Point +/- error
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(x1, y1, 5, 5 );
        item->setPen( QPen(Qt::white) );
        item->setBrush( QBrush(Qt::red) );
        canvasAux->scene()->addItem(item);
        return true;
    }

    //--------------------------------------------
    // Fill the border points
    //--------------------------------------------

    //Last two points
    i   = lstPixSelAux->count()-1;
    x0  = lstPixSelAux->at(0).first;
    y0  = lstPixSelAux->at(0).second;
    x1  = lstPixSelAux->at(i-1).first;
    y1  = lstPixSelAux->at(i-1).second;
    x2  = lstPixSelAux->at(i).first;
    y2  = lstPixSelAux->at(i).second;

    //Draw the line
    if( abs(x0-x2)<=6 && abs(y0-y2)<=6 ){//Politope closed
        funcCreateLine(false,x1,y1,x0,y0);
    }else{//Add line
        funcCreateLine(true,x1,y1,x2,y2);
    }

    //--------------------------------------------
    // Fill pixels inside politope
    //--------------------------------------------
    if( abs(x0-x2)<=6 && abs(y0-y2)<=6 ){
        funcFillFigure();
    }










    return true;
}

void MainWindow::funcFillFigure(){
    // Clear points
    //...
    //qDebug() << "Polytope closed: " << QString::number(lstBorder->count()) ;
    //while(ui->tableLstPoints->rowCount() > 0){
    //    ui->tableLstPoints->removeRow(0);
    //}
    //funcPutImageIntoGV( canvasAux, imgPath );
    canvasAux->scene()->clear();

    // Get max and min Y
    //...
    int i, minX, maxX, minY, maxY;
    minX = minY = INT_MAX;
    maxX = maxY = -1;
    for(i=0;i<lstBorder->count();i++){
        minY = ( lstBorder->at(i).second < minY )?lstBorder->at(i).second:minY;
        maxY = ( lstBorder->at(i).second > maxY )?lstBorder->at(i).second:maxY;
    }

    //Run range in Y
    int tmpY;
    //bool entro;
    QPair<int,int> tmpPair;
    for(tmpY=minY; tmpY<=maxY; tmpY++){
        //qDebug() << "tmpY: "<< QString::number(tmpY);
        //Get range in X
        //entro = false;
        minX = INT_MAX;
        maxX = -1;
        for(i=0;i<lstBorder->count();i++){
            if( lstBorder->at(i).second == tmpY ){
                //entro = true;
                minX = ( lstBorder->at(i).first < minX )?lstBorder->at(i).first:minX;
                maxX = ( lstBorder->at(i).first > maxX )?lstBorder->at(i).first:maxX;
                lstBorder->removeAt(i);
                //qDebug() << "minX: "<< QString::number(minX)<< "maxX: "<< QString::number(maxX);
            }
        }
        //qDebug() << "Linw: minX= "<< QString::number(minX)<< "maxX= "<< QString::number(maxX);

        //Draw line in X
        for(i=minX;i<=maxX;i++){
            tmpPair.first  = i;
            tmpPair.second = tmpY;
            lstSelPix->append( tmpPair );
            funcDrawPointIntoCanvas( tmpPair.first, tmpPair.second, 1, 1, "#FF0000", "#FFFFFF" );
        }
        //funcShowMsg("","jeha");
    }

    //Clear border
    lstBorder->clear();

    //Redrawn pixels
    for(i=0;i<lstSelPix->count();i++){
        funcDrawPointIntoCanvas(
                                    lstSelPix->at(i).first,
                                    lstSelPix->at(i).second,
                                    1,
                                    1,
                                    "#FF0000",
                                    "#FFFFFF"
                                );
    }

    lstPixSelAux->clear();

     disconnect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcAddPoint(QMouseEvent*) )
           );

     mouseCursorReset();


}





void MainWindow::funcCreateLine(bool drawVertex,
    int x1,
    int y1,
    int x2,
    int y2
){

    QPair<int,int> tmpPair;

    //Drawing the vertex
    if( drawVertex ){
        tmpPair.first = x2-3;
        tmpPair.second = y2-4;
        lstBorder->append(tmpPair);
        funcDrawPointIntoCanvas(tmpPair.first, tmpPair.second, 5, 5, "#FF0000", "#FFFFFF");
        //QGraphicsEllipseItem* item = new QGraphicsEllipseItem(tmpPair.first, tmpPair.second, 5, 5 );
        //item->setPen( QPen(QColor("#FFFFFF")) );
        //item->setBrush( QBrush(QColor("#FF0000")) );
        //myCanvas->scene()->addItem(item);
    }

    //Variables
    int i, yIni, yEnd;
    float m,b;

    //Compute slope
    m = (float)(y2-y1) / (float)(x2-x1);
    b = (float)y1-(m*(float)x1);

    //Obtain points in the border
    int xIni = (x1 <= x2)?x1:x2;
    int xEnd = (xIni == x1)?x2:x1;
    int lastY, j;
    if(xIni==xEnd){
        yIni = (y1<=y2)?y1:y2;
        yEnd = (y1==yIni)?y2:y1;
        for( j=yIni; j<=yEnd; j++ ){
            funcDrawPointIntoCanvas(xIni, j, 1, 1, "#FF0000", "#FFFFFF");
            //QGraphicsEllipseItem* item = new QGraphicsEllipseItem( xIni, j, 1, 1 );
            //item->setPen( QPen(QColor("#FF0000")) );
            //myCanvas->scene()->addItem(item);

            tmpPair.first = xIni;
            tmpPair.second = j;
            lstBorder->append(tmpPair);
        }
    }else{
        for( i=xIni; i<=xEnd; i++ ){
            //Add discrete value
            tmpPair.first = i;
            tmpPair.second = floor( (m*i) + b );
            lstBorder->append(tmpPair);

            funcDrawPointIntoCanvas(tmpPair.first, tmpPair.second, 1, 1, "#FF0000", "#FFFFFF");
            //QGraphicsEllipseItem* item = new QGraphicsEllipseItem( tmpPair.first, tmpPair.second, 1, 1 );
            //item->setPen( QPen(QColor("#FF0000")) );
            //myCanvas->scene()->addItem(item);

            //Complete the line
            if( (i == xIni) ){
                lastY = tmpPair.second;
            }else{
                yIni = (tmpPair.second <= lastY)?tmpPair.second:lastY;
                yEnd = (yIni == tmpPair.second)?lastY:tmpPair.second;
                if( abs(yIni-yEnd) > 1 ){
                    lastY = tmpPair.second;
                    for( j=yIni; j<=yEnd; j++ ){
                        tmpPair.first = i;
                        tmpPair.second = j;
                        lstBorder->append(tmpPair);

                        //QGraphicsEllipseItem* item = new QGraphicsEllipseItem( i, j, 1, 1 );
                        //item->setPen( QPen(QColor("#FF0000")) );
                        //myCanvas->scene()->addItem(item);
                        funcDrawPointIntoCanvas(i,j,1,1, "#FF0000", "#FFFFFF");

                        //funcShowMsg("","Ja");
                    }
                }
            }
        }
    }


}

void MainWindow::funcDrawPointIntoCanvas(
                                            int x,
                                            int y,
                                            int w,
                                            int h,
                                            QString color = "#FF0000",
                                            QString lineColor = "#FFFFFF"
){
    QGraphicsEllipseItem* item = new QGraphicsEllipseItem( x, y, w, h );
    item->setPen( QPen(QColor(color)) );
    canvasAux->scene()->addItem(item);
    item->setBrush( QBrush(QColor(lineColor)) );
}

void MainWindow::funcAddPoit2Graph(
                                        GraphicsView *tmpCanvas,
                                        int x,
                                        int y,
                                        int w,
                                        int h,
                                        QColor color,
                                        QColor lineColor
){
    QGraphicsEllipseItem* item = new QGraphicsEllipseItem( x, y, w, h );
    item->setPen( QPen(color) );
    tmpCanvas->scene()->addItem(item);
    item->setBrush( QBrush(lineColor) );
}


void MainWindow::funcIniCamParam( structRaspcamSettings *raspcamSettings )
{    
    QList<QString> tmpList;

    //Set AWB: off,auto,sun,cloud,shade,tungsten,fluorescent,incandescent,flash,horizon
    tmpList<<"none"<<"off"<<"auto"<<"sun"<<"cloud"<<"shade"<<"tungsten"<<"fluorescent"<<"incandescent"<<"flash"<<"horizon";
    ui->cbAWB->clear();
    ui->cbAWB->addItems( tmpList );
    ui->cbAWB->setCurrentText((char*)raspcamSettings->AWB);
    tmpList.clear();

    //Set Exposure: off,auto,night,nightpreview,backlight,spotlight,sports,snow,beach,verylong,fixedfps,antishake,fireworks
    tmpList<<"none"<<"off"<<"auto"<<"night"<<"nightpreview"<<"backlight"<<"spotlight"<<"sports"<<"snow"<<"beach"<<"verylong"<<"fixedfps"<<"antishake"<<"fireworks";
    ui->cbExposure->clear();
    ui->cbExposure->addItems( tmpList );
    ui->cbExposure->setCurrentText((char*)raspcamSettings->Exposure);
    tmpList.clear();

    //Gray YUV420
    //if( raspcamSettings->Format == 1 )ui->rbFormat1->setChecked(true);
    //if( raspcamSettings->Format == 2 )ui->rbFormat2->setChecked(true);

    //Brightness
    //ui->slideBrightness->setValue( raspcamSettings->Brightness );
    //ui->labelBrightness->setText( "Brightness: " + QString::number(raspcamSettings->Brightness) );

    //Sharpness
    //ui->slideSharpness->setValue( raspcamSettings->Sharpness );
    //ui->labelSharpness->setText( "Sharpness: " + QString::number(raspcamSettings->Sharpness) );

    //Contrast
    //ui->slideContrast->setValue( raspcamSettings->Contrast );
    //ui->labelContrast->setText( "Contrast: " + QString::number(raspcamSettings->Contrast) );

    //Saturation
    //ui->slideSaturation->setValue( raspcamSettings->Saturation );
    //ui->labelSaturation->setText( "Saturation: " + QString::number(raspcamSettings->Saturation) );

    //ShuterSpeed
    ui->slideShuterSpeed->setValue( raspcamSettings->ShutterSpeed );
    ui->slideShuterSpeedSmall->setValue( raspcamSettings->ShutterSpeedSmall );
    ui->labelShuterSpeed->setText( "Shuter Speed: " + QString::number(raspcamSettings->ShutterSpeed+raspcamSettings->ShutterSpeedSmall) );

    //ISO
    ui->slideISO->setValue( raspcamSettings->ISO );
    ui->labelISO->setText( "ISO: " + QString::number(raspcamSettings->ISO) );

    //ExposureCompensation
    //ui->slideExpComp->setValue( raspcamSettings->ExposureCompensation );
    //ui->labelExposureComp->setText( "Exp. Comp.: " + QString::number(raspcamSettings->ExposureCompensation) );

    //RED
    //qDebug() << "Red: " << raspcamSettings->Red;
    //ui->slideRed->setValue( raspcamSettings->Red );
    //ui->labelRed->setText( "Red: " + QString::number(raspcamSettings->Red) );

    //GREEN
    //ui->slideGreen->setValue( raspcamSettings->Green );
    //ui->labelGreen->setText( "Green: " + QString::number(raspcamSettings->Green) );

    //PREVIEW
    //if( raspcamSettings->Preview )ui->cbPreview->setChecked(true);
    //else ui->cbPreview->setChecked(false);

    //TRIGGER TIME
    ui->slideTriggerTime->setValue(raspcamSettings->TriggerTime);
    ui->labelTriggerTime->setText("Trigger time: " + QString::number(raspcamSettings->TriggerTime)+"s");

    //DENOISE EFX
    if( raspcamSettings->Denoise )ui->cbDenoise->setChecked(true);
    else ui->cbDenoise->setChecked(false);

    //COLORBALANCE EFX
    if( raspcamSettings->ColorBalance )ui->cbColorBalance->setChecked(true);
    else ui->cbColorBalance->setChecked(false);


}

void MainWindow::on_pbGetVideo_clicked()
{

    //Prepare socket variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    frameStruct *frame2Send     = (frameStruct*)malloc(sizeof(frameStruct));
    portno = lstSettings->tcpPort;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        //error("ERROR opening socket");
    }
    server = gethostbyname( "192.168.1.69" );
    //server = gethostbyname( "10.0.5.126" );
    if (server == NULL) {
        //fprintf(stderr,"ERROR, not such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        //error("ERROR connecting");
    }

    //Request file
    frame2Send->header.idMsg          = (char)4;  // Header: Id instruction
    frame2Send->header.consecutive    = 1;        // Header: Consecutive
    frame2Send->header.numTotMsg      = 1;        // Header: Total number of message to send
    frame2Send->header.bodyLen        = 0;        // Header: Usable message lenght
    int tmpFrameLen = sizeof(frameHeader)+frame2Send->header.bodyLen;
    n = ::write(sockfd,frame2Send,tmpFrameLen);
    if (n < 0){
        qDebug() << "writing to socket";
    }

    //Receibing ack with file len
    unsigned int fileLen;
    unsigned char bufferRead[frameBodyLen];
    n = read(sockfd,bufferRead,frameBodyLen);
    memcpy(&fileLen,&bufferRead,sizeof(unsigned int));
    //funcShowMsg("FileLen n("+QString::number(n)+")",QString::number(fileLen));

    if(fileLen>0){
        //Receive File
        unsigned char tmpFile[fileLen];
        if( funcReceiveFile( sockfd, fileLen, bufferRead, tmpFile ) ){

            //Save file
            std::ofstream myFile ("yoRec2.jpg", std::ios::out | std::ios::binary);
            myFile.write((char*)&tmpFile, fileLen);

            //It finishes succesfully
            funcShowMsg("OK","Successfull reception");

        }else{
            funcShowMsg("ERROR","File does not received");
        }

        ::close(sockfd);

    }else{
        funcShowMsg("Alert","File is empty");
    }


}



bool MainWindow::funcReceiveFile(
                                    int sockfd,
                                    unsigned int fileLen,
                                    unsigned char *bufferRead,
                                    unsigned char *tmpFile
){

    qDebug() << "Inside funcReceiveFile";



    //Requesting file
    int i, n;
    //qDebug() << "Writing0";
    n = ::write(sockfd,"sendfile",8);
    if (n < 0){
        qDebug() << "ERROR: writing to socket";
        return false;
    }
    //funcShowMsg("alert","Requesting file");



    //Receive file parts
    unsigned int numMsgs = (fileLen>0)?floor( (float)fileLen / (float)frameBodyLen ):0;
    numMsgs = ((numMsgs*frameBodyLen)<fileLen)?numMsgs+1:numMsgs;
    unsigned int tmpPos = 0;
    memset(tmpFile,'\0',fileLen);
    qDebug() << "Receibing... " <<  QString::number(numMsgs) << " messages";
    qDebug() << "fileLen: " << fileLen;

    //Receive the last
    if(numMsgs==0){
        //Receives the unik message
        bzero(bufferRead,frameBodyLen);
        //qDebug() << "R1";
        //n = read(sockfd,tmpFile,fileLen);
    }else{

        //ui->progBar->setVisible(true);
        ui->progBar->setRange(0,numMsgs);
        ui->progBar->setValue(0);

        funcActivateProgBar();

        for(i=1;i<=(int)numMsgs;i++){
            ui->progBar->setValue(i);
            bzero(bufferRead,frameBodyLen);
            n = read(sockfd,bufferRead,frameBodyLen);
            qDebug() << "n: " << n;
            if(n!=(int)frameBodyLen&&i<(int)numMsgs){
                qDebug() << "ERROR, message " << i << "WRONG";
                return false;
            }
            //Append message to file
            memcpy( &tmpFile[tmpPos], bufferRead, frameBodyLen );
            tmpPos += n;
            //Request other part
            if( i<(int)numMsgs ){
                //qDebug() << "W2";
                QtDelay(2);
                n = ::write(sockfd,"sendpart",8);                
                if(n<0){
                    qDebug() << "ERROR: Sending part-file request";
                    return false;
                }
            }
        }
        ui->progBar->setValue(0);
        ui->progBar->setVisible(false);
        ui->progBar->update();
        QtDelay(30);

    }

    qDebug() << "tmpPos: " << tmpPos;


    return true;
}

void MainWindow::funcActivateProgBar(){
    mouseCursorReset();
    ui->progBar->setVisible(true);
    ui->progBar->setValue(0);
    ui->progBar->update();
    QtDelay(50);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_pbAddIp_clicked()
{
    //Validating IP
    if( !funcIsIP( ui->txtIp->text().toStdString() ) ){
        funcShowMsg("ERROR","Invalid IP address");
        ui->txtIp->setFocus();
    }else{
        //Checks if IP is in the list and remove it if exists
        int i;
        for(i=0;i<ui->tableLstCams->rowCount();i++){
            if( ui->tableLstCams->item(i,1)->text().trimmed() == ui->txtIp->text().trimmed() ){
                ui->tableLstCams->removeRow(i);
            }
        }

        //Add IP
        camSettings * tmpCamSett = (camSettings*)malloc(sizeof(camSettings));
        tmpCamSett->idMsg = (char)0;
        funcValCam(
                        ui->txtIp->text().trimmed().toStdString(),
                        lstSettings->tcpPort,
                        tmpCamSett
                  );
        if( tmpCamSett->idMsg < 1 ){
            funcShowMsg("ERROR","Camera does not respond at "+ui->txtIp->text());
            ui->txtIp->setFocus();
        }else{
            ui->tableLstCams->insertRow( ui->tableLstCams->rowCount() );
            ui->tableLstCams->setItem(
                                            ui->tableLstCams->rowCount()-1,
                                            0,
                                            new QTableWidgetItem(QString(tmpCamSett->Alias))
                                      );
            ui->tableLstCams->setItem(
                                            ui->tableLstCams->rowCount()-1,
                                            1,
                                            new QTableWidgetItem(ui->txtIp->text().trimmed())
                                      );
            ui->tableLstCams->setColumnWidth(0,150);
            ui->tableLstCams->setColumnWidth(1,150);
            //funcShowMsg("Success","Camera detected at "+ui->txtIp->text().trimmed());
        }
    }
}

void MainWindow::on_pbSearchAll_clicked()
{
    //Clear table
    while( ui->tableLstCams->rowCount() > 0 ){
        ui->tableLstCams->removeRow(0);
    }

    //Obtain IP list
    QString result = "";//idMsg to send
    FILE* pipe = popen("arp", "r");
    char bufferComm[frameBodyLen];
    try {
      while (!feof(pipe)) {
        if (fgets(bufferComm, frameBodyLen, pipe) != NULL){
          result.append( bufferComm );
        }
      }
    } catch (...) {
      pclose(pipe);
      throw;
    }
    pclose(pipe);

    //Check IPs candidates
    camSettings *tmpCamSett = (camSettings*)malloc(sizeof(camSettings));
    QStringList ipsCandidates = result.split("\n");
    QStringList candIP;
    int i;


    int sockfd, n, tmpFrameLen;
    tmpFrameLen = sizeof(camSettings);
    struct sockaddr_in serv_addr;
    struct hostent *server;
    unsigned char bufferRead[tmpFrameLen];

    for( i=0;i<ipsCandidates.count();i++ ){
        if( !ipsCandidates.at(i).contains("unreachable") &&
            !ipsCandidates.at(i).contains("incomplete") ){
            candIP = ipsCandidates.at(i).split("ether");

            if( funcIsIP( candIP.at(0).toStdString() ) &&
                candIP.at(0) != ui->txtIp->text().trimmed() + "254"
            ){

                qDebug() << "IP: " << candIP.at(0).trimmed();

                tmpCamSett->idMsg = (char)0;
                memset(tmpCamSett,'\0',sizeof(camSettings));

                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0){
                    qDebug() << "opening socket";
                }else{
                    server = gethostbyname( candIP.at(0).trimmed().toStdString().c_str() );
                    if (server == NULL) {
                        qDebug() << "Not such host";
                    }else{


                        bzero((char *) &serv_addr, sizeof(serv_addr));
                        serv_addr.sin_family = AF_INET;
                        bcopy((char *)server->h_addr,
                            (char *)&serv_addr.sin_addr.s_addr,
                            server->h_length);
                        serv_addr.sin_port = htons(lstSettings->tcpPort);




                        if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
                            qDebug() << "connecting";
                        }else{
                            //Request camera settings
                            char tmpIdMsg = (char)1;


                            n = ::write(sockfd,&tmpIdMsg,1);
                            if (n < 0){
                                qDebug() << "writing to socket";
                            }else{
                                //Receibing ack with file len
                                n = read(sockfd,bufferRead,tmpFrameLen);
                                if (n < 0){
                                    qDebug() << "reading socket";
                                }else{
                                    memcpy(tmpCamSett,&bufferRead,tmpFrameLen);

                                    if( tmpCamSett->idMsg > 0 ){
                                        ui->tableLstCams->insertRow( ui->tableLstCams->rowCount() );
                                        ui->tableLstCams->setItem(
                                                                        ui->tableLstCams->rowCount()-1,
                                                                        0,
                                                                        new QTableWidgetItem(QString(tmpCamSett->Alias))
                                                                  );
                                        ui->tableLstCams->setItem(
                                                                        ui->tableLstCams->rowCount()-1,
                                                                        1,
                                                                        new QTableWidgetItem(candIP.at(0).trimmed())
                                                                  );
                                        ui->tableLstCams->setColumnWidth(0,150);
                                        ui->tableLstCams->setColumnWidth(1,150);
                                    }







                                }
                            }
                        }
                    }

                }



                ::close(sockfd);





















            }




        }
    }
}

void MainWindow::on_pbSendComm_clicked()
{
    if( !camSelected->isConnected){
        funcShowMsg("Alert","Camera not connected");
        return (void)NULL;
    }
    if( ui->txtCommand->text().isEmpty() ){
        funcShowMsg("Alert","Empty command");
        return (void)NULL;
    }
    if( ui->tableLstCams->rowCount() == 0 ){
        funcShowMsg("Alert","Not camera detected");
        return (void)NULL;
    }
    ui->tableLstCams->setFocus();

    //Prepare message to send
    frameStruct *frame2send = (frameStruct*)malloc(sizeof(frameStruct));
    memset(frame2send,'\0',sizeof(frameStruct));
    if( !ui->checkBlind->isChecked() ){
        frame2send->header.idMsg = (unsigned char)2;
    }else{
        frame2send->header.idMsg = (unsigned char)3;
    }
    frame2send->header.numTotMsg = 1;
    frame2send->header.consecutive = 1;
    frame2send->header.bodyLen   = ui->txtCommand->text().length();
    memcpy(
                frame2send->msg,
                ui->txtCommand->text().toStdString().c_str(),
                ui->txtCommand->text().length()
          );

    //Prepare command message
    int sockfd, n, tmpFrameLen;
    //unsigned char bufferRead[sizeof(frameStruct)];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    qDebug() << "Comm IP: " << QString((char*)camSelected->IP);
    if (sockfd < 0){
        qDebug() << "opening socket";
        return (void)NULL;
    }
    //server = gethostbyname( ui->tableLstCams->item(tmpRow,1)->text().toStdString().c_str() );
    server = gethostbyname( (char*)camSelected->IP );
    if (server == NULL) {
        qDebug() << "Not such host";
        return (void)NULL;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(camSelected->tcpPort);
    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        qDebug() << "ERROR: connecting to socket";
        return (void)NULL;
    }


    //Request command result
    qDebug() << "idMsg: " << (int)frame2send->header.idMsg;
    qDebug() << "command: " << frame2send->msg;
    qDebug() << "tmpFrameLen: " << sizeof(frameHeader) + ui->txtCommand->text().length();
    tmpFrameLen = sizeof(frameHeader) + ui->txtCommand->text().length();
    n = ::write(sockfd,frame2send,tmpFrameLen);
    if(n<0){
        qDebug() << "ERROR: Sending command";
        return (void)NULL;
    }

    //Receibing ack with file len
    unsigned int fileLen;
    unsigned char bufferRead[frameBodyLen];
    n = read(sockfd,bufferRead,frameBodyLen);
    memcpy(&fileLen,&bufferRead,sizeof(unsigned int));
    fileLen = (fileLen<frameBodyLen)?frameBodyLen:fileLen;
    qDebug() << "fileLen: " << fileLen;
    //funcShowMsg("FileLen n("+QString::number(n)+")",QString::number(fileLen));

    //Receive File
    unsigned char tmpFile[fileLen];
    funcReceiveFile( sockfd, fileLen, bufferRead, tmpFile );
    qDebug() <<tmpFile;
    ::close(sockfd);

    //Show command result
    std::string tmpTxt((char*)tmpFile);
    qDebug() << "Get: " << (char*)tmpFile;
    ui->txtCommRes->setText( QString(tmpTxt.c_str()) ) ;

}

void MainWindow::on_pbConnect_clicked()
{
    ui->tableLstCams->setFocus();
    int numRow = ui->tableLstCams->rowCount();    
    if( numRow > 0 ){
        int rowSelected = ui->tableLstCams->currentRow();
        qDebug() << "CurrentRow: " << QString::number(rowSelected);
        if(rowSelected >= 0){
            if( camSelected->isConnected ){
                qDebug() << "Disconnect";
                camSelected->isConnected = false;
                camSelected->tcpPort = 0;
                memset(camSelected->IP,'\0',sizeof(camSelected->IP));
                ui->pbConnect->setIcon( QIcon(":/new/icons/imagenInte/right.gif") );
                ui->pbSnapshot->setEnabled(false);
            }else{
                qDebug() << "Connect: ";
                camSelected->isConnected = true;
                camSelected->tcpPort = lstSettings->tcpPort;
                memcpy(
                            camSelected->IP,
                            ui->tableLstCams->item(rowSelected,1)->text().trimmed().toStdString().c_str(),
                            sizeof(camSelected->IP)
                      );                
                ui->pbConnect->setIcon( QIcon(":/new/icons/imagenInte/close.png") );
                ui->pbSnapshot->setEnabled(true);
                qDebug() << "IP->: " << QString((char*)camSelected->IP);
                //Save last IP
                saveFile( "./settings/lastIp.hypcam", QString((char*)camSelected->IP) );
            }
        }else{
            ui->pbSnapshot->setEnabled(false);
            funcShowMsg("ERROR", "Camara not selected");            
        }
    }else{
        funcShowMsg("ERROR", "Empty cam list");
        ui->pbSnapshot->setEnabled(false);
    }

}

void MainWindow::on_pbCamTurnOn_clicked()
{
    unsigned char v[20];
    v[0] = 'H';
    v[1] = 'o';
    v[2] = 'l';
    v[3] = 'a';
    std::string v2;
    v2.assign((char*)v);
    qDebug() << "Size: " << v2.size();

}





void MainWindow::on_pbStartVideo_clicked()
{    
    if( !camSelected->isConnected ){
        funcShowMsg("Lack","Select and connect a camera");
    }else{
        //Save camera settings
        //..
        saveRaspCamSettings( "tmp" );

        //Play
        //..
        if( camSelected->stream ){//Playing video
            camSelected->stream = false;
            ui->pbStartVideo->setIcon( QIcon(":/new/icons/imagenInte/player_play.svg") );
            ui->pbStartVideo->setToolTip("Play");
            qDebug() << "Paused";
        }else{
            if( ui->cbOneShot->isChecked() ){
                funcUpdateVideo( 100, 2.3 );
            }else{
                camSelected->stream = true;
                ui->pbStartVideo->setIcon( QIcon(":/new/icons/imagenInte/pause.png") );
                ui->pbStartVideo->setToolTip("Pause");
                qDebug() << "Play";
                QtDelay(100);
                while( camSelected->stream ){
                    funcUpdateVideo( 100, 2.3 );
                    QtDelay(700);
                }
            }
        }
    }

}


unsigned char *MainWindow::funcGetRemoteImg( strReqImg *reqImg ){

    //Open socket
    int sockfd = connectSocket( camSelected );
    unsigned char bufferRead[frameBodyLen];
    qDebug() << "Socket opened";

    //Require photo size
    //QtDelay(5);
    ::write(sockfd,reqImg,sizeof(strReqImg));
    qDebug() << "Img request sent";

    //Receive ACK with the camera status
    read(sockfd,bufferRead,frameBodyLen);
    if( bufferRead[1] == 1 ){//Begin the image adquisition routine
        qDebug() << "Camera OK";
    }else{//Do nothing becouse camera is not accessible
        qDebug() << "ERROR turning on the camera";
    }

    //Receive photo's size
    unsigned int fileLen;
    read(sockfd,bufferRead,frameBodyLen);
    memcpy(&fileLen,&bufferRead,sizeof(unsigned int));
    qDebug() << "Receiving fileLen: " << QString::number(fileLen);

    //Receive File photogram
    int buffLen = ceil((float)fileLen/(float)frameBodyLen)*frameBodyLen;
    unsigned char *tmpFile = (unsigned char*)malloc(buffLen);
    QtDelay(60);
    if( funcReceiveFile( sockfd, fileLen, bufferRead, tmpFile ) ){
        qDebug() << "Frame received";
    }else{
        qDebug() << "ERROR: Frame does not received";
    }

    //Save a backup of the image received
    //..
    if(!saveBinFile( (unsigned long)fileLen, tmpFile,_PATH_IMAGE_RECEIVED)){
        qDebug()<< "ERROR: saving image-file received";
    }

    //Close socket and return
    ::close(sockfd);
    return tmpFile;
}

structRaspcamSettings MainWindow::funcFillSnapshotSettings( structRaspcamSettings raspSett ){
    //Take settings from gui ;D
    //raspSett.width                 = ui->slideWidth->value();
    //raspSett.height                = ui->slideHeight->value();
    memcpy(
                raspSett.AWB,
                ui->cbAWB->currentText().toStdString().c_str(),
                sizeof(raspSett.AWB)
          );
    //raspSett.Brightness            = ui->slideBrightness->value();
    //raspSett.Contrast              = ui->slideContrast->value();
    memcpy(
                raspSett.Exposure,
                ui->cbExposure->currentText().toStdString().c_str(),
                sizeof(raspSett.Exposure)
          );
    //raspSett.ExposureCompensation  = ui->slideExpComp->value();
    //raspSett.Format                = ( ui->rbFormat1->isChecked() )?1:2;
    //raspSett.Green                 = ui->slideGreen->value();
    raspSett.ISO                   = ui->slideISO->value();
    //raspSett.Red                   = ui->slideRed->value();
    //raspSett.Saturation            = ui->slideSaturation->value();
    //raspSett.Sharpness             = ui->slideSharpness->value();
    raspSett.ShutterSpeed          = ui->slideShuterSpeed->value() + ui->slideShuterSpeedSmall->value();
    raspSett.Denoise = (ui->cbDenoise->isChecked())?1:0;
    raspSett.ColorBalance = (ui->cbColorBalance->isChecked())?1:0;
    raspSett.TriggerTime = ui->slideTriggerTime->value();

    return raspSett;
}

unsigned char * MainWindow::funcObtVideo( unsigned char saveLocally ){

    qDebug() << "Dentro 1";
    //Open socket
    int sockfd = connectSocket( camSelected );
    unsigned int fileLen;
    unsigned char bufferRead[frameBodyLen];

    //Require photo size
    qDebug() << "Dentro 2";
    unsigned char tmpInstRes[2];
    tmpInstRes[0] = (unsigned char)6;//Request photo size
    tmpInstRes[1] = saveLocally;
    ::write(sockfd,&tmpInstRes,2);

    unsigned char *tmpFile;

    if( saveLocally==1 ){

        //Receive photo's size
        qDebug() << "Dentro 3";
        read(sockfd,bufferRead,frameBodyLen);
        memcpy(&fileLen,&bufferRead,sizeof(unsigned int));
        qDebug() << "fileLen: " << QString::number(fileLen);

        //Receive File photogram
        qDebug() << "Dentro 4";
        tmpFile = (unsigned char*)malloc(fileLen);

        if( funcReceiveFile( sockfd, fileLen, bufferRead, tmpFile ) ){
            qDebug() << "Big image received";
        }else{
            qDebug() << "ERROR: Big image does not received";
        }
    }else{
        tmpFile = (unsigned char*)malloc(2);
    }

    //Close socket
    ::close(sockfd);

    return tmpFile;
}

bool MainWindow::funcUpdateVideo( unsigned int msSleep, int sec2Stab ){

    msSleep = msSleep;
    mouseCursorWait();
    this->update();

    //Set required image's settings
    //..
    strReqImg *reqImg   = (strReqImg*)malloc(sizeof(strReqImg));
    reqImg->idMsg       = (unsigned char)7;    
    reqImg->stabSec     = sec2Stab;    
    reqImg->raspSett    = funcFillSnapshotSettings( reqImg->raspSett );

    qDebug() << "reqImg->raspSett.TriggerTime: " << reqImg->raspSett.TriggerTime;

    //Define photo's region
    //..
    if( ui->cbThumbPreview->isChecked() )
    {
        reqImg->needCut     = false;
        reqImg->imgCols     = _FRAME_THUMB_W;
        reqImg->imgRows     = _FRAME_THUMB_H;
    }
    else
    {
        if( ui->cbFullPhoto->isChecked() )
        {
            reqImg->needCut = false;
            reqImg->imgCols = _BIG_WIDTH;
            reqImg->imgRows = _BIG_HEIGHT;
        }
        else
        {
            //QString tmpSquare2Load = (ui->cbPreview->isChecked())?_PATH_REGION_OF_INTERES:_PATH_SQUARE_APERTURE;
            if( !funGetSquareXML( _PATH_SQUARE_APERTURE, &reqImg->sqApSett ) ){
                funcShowMsg("ERROR","Loading squareAperture.xml");
                return false;
            }
            reqImg->needCut     = true;
            //Calculate real number of columns of the required photo
            reqImg->imgCols         = round( ((float)reqImg->sqApSett.rectW/(float)reqImg->sqApSett.canvasW) * (float)_BIG_WIDTH);
            reqImg->imgRows         = round( ((float)reqImg->sqApSett.rectH/(float)reqImg->sqApSett.canvasH) * (float)_BIG_HEIGHT);
            reqImg->sqApSett.rectX  = round( ((float)reqImg->sqApSett.rectX/(float)reqImg->sqApSett.canvasW) * (float)_BIG_WIDTH);
            reqImg->sqApSett.rectY  = round( ((float)reqImg->sqApSett.rectY/(float)reqImg->sqApSett.canvasH) * (float)_BIG_HEIGHT);

        }
    }

    //It save the received image
    funcGetRemoteImg( reqImg );
    QImage tmpImg(_PATH_IMAGE_RECEIVED);

    //tmpImg.save("./Results/tmpCropFromSave.ppm");
    ui->labelVideo->setPixmap( QPixmap::fromImage(tmpImg) );
    ui->labelVideo->setFixedWidth( tmpImg.width() );
    ui->labelVideo->setFixedHeight( tmpImg.height() );
    ui->labelVideo->update();

    //Delay in order to refresh actions applied    
    this->update();
    return true;
}








void MainWindow::on_pbSaveImage_clicked()
{    
    bool tmpSaveLocaly = funcShowMsgYesNo("Question","Bring into this PC?");
    //Stop streaming
    ui->pbStartVideo->click();
    ui->pbStartVideo->setEnabled(false);
    ui->pbStartVideo->update();

    //Open socket
    int tmpFrameLen = 2+sizeof(structRaspcamSettings);;
    int sockfd = connectSocket( camSelected );

    //Turn off camera
    unsigned char tmpInstRes[tmpFrameLen];
    tmpInstRes[0] = (unsigned char)5;//Camera instruction
    tmpInstRes[1] = (unsigned char)2;
    ::write(sockfd,&tmpInstRes,2);
    bzero(tmpInstRes,tmpFrameLen);
    read(sockfd,tmpInstRes,2);
    ::close(sockfd);

    //Turn on camera with new parameters
    sockfd = connectSocket( camSelected );
    //raspcamSettings->width  = _BIG_WIDTH;//2592, 640
    //raspcamSettings->height = _BIG_HEIGHT;//1944, 480
    tmpInstRes[0] = 5;
    tmpInstRes[1] = 1;
    memcpy( &tmpInstRes[2], raspcamSettings, sizeof(structRaspcamSettings) );
    ::write(sockfd,&tmpInstRes,tmpFrameLen);
    bzero(tmpInstRes,tmpFrameLen);
    read(sockfd,tmpInstRes,2);
    //qDebug() << "n: " << QString::number(n);
    qDebug() << "instRes: " << tmpInstRes[1];
    if( tmpInstRes[1] == (unsigned char)0 ){
        funcShowMsg("ERROR","Camera did not completed the instruction");
    }
    ::close(sockfd);


    if( tmpSaveLocaly ){

        //Get big file
        unsigned char *tmpFile = funcObtVideo( (unsigned char)1 );

        //Take file name
        char tmpFileName[16+14];
        snprintf(tmpFileName, 16+14, "tmpImages/%lu.ppm", time(NULL));

        std::ofstream outFile ( tmpFileName, std::ios::binary );
        //outFile<<"P6\n"<<raspcamSettings->width<<" "<<raspcamSettings->height<<" 255\n";
        //outFile.write( (char*)tmpFile, 3*raspcamSettings->width*raspcamSettings->height );
        outFile.close();
        delete tmpFile;
        qDebug() << "Big image saved locally";

    }else{

        //Send instruction: Save image in HypCam
        sockfd = connectSocket( camSelected );
        tmpInstRes[0] = (unsigned char)6;//Camera instruction
        tmpInstRes[1] = (unsigned char)2;
        ::write(sockfd,&tmpInstRes,2);
        bzero(tmpInstRes,tmpFrameLen);
        read(sockfd,tmpInstRes,2);
        qDebug() << "Image saved into HypCam";
        ::close(sockfd);

    }

    qDebug() << "Before turn off";

    //Turn off camera
    sockfd = connectSocket( camSelected );
    tmpInstRes[0] = (unsigned char)5;//Camera instruction
    tmpInstRes[1] = (unsigned char)2;
    ::write(sockfd,&tmpInstRes,2);
    bzero(tmpInstRes,tmpFrameLen);
    read(sockfd,tmpInstRes,2);
    ::close(sockfd);
    qDebug() << "Camera Turned off";

    //Turn on camera with new parameters
    sockfd = connectSocket( camSelected );
    funcSetCam(raspcamSettings);
    tmpInstRes[0] = 5;
    tmpInstRes[1] = 1;
    memcpy( &tmpInstRes[2], raspcamSettings, sizeof(structRaspcamSettings) );
    ::write(sockfd,&tmpInstRes,tmpFrameLen);
    bzero(tmpInstRes,tmpFrameLen);
    read(sockfd,tmpInstRes,2);
    qDebug() << "instRes: " << tmpInstRes[1];
    if( tmpInstRes[1] == (unsigned char)0 ){
        funcShowMsg("ERROR","Camera did not completed the instruction");
    }
    ::close(sockfd);

    //Start streaming
    ui->pbStartVideo->setEnabled(true);
    ui->pbStartVideo->click();
}

void MainWindow::on_slideShuterSpeed_valueChanged(int value)
{
    ui->labelShuterSpeed->setText( "Shuter Speed: " + QString::number(value + ui->slideShuterSpeedSmall->value()) );
}

void MainWindow::on_slideISO_valueChanged(int value)
{
    ui->labelISO->setText( "ISO: " + QString::number(value) );
}

void MainWindow::on_pbSaveRaspParam_clicked()
{
    if( ui->txtCamParamXMLName->text().isEmpty() ){
        funcShowMsg("Lack","Type the scenario's name");
    }else{

        QString tmpName = ui->txtCamParamXMLName->text();
        tmpName.replace(".xml","");
        tmpName.replace(".XML","");
        qDebug() << tmpName;

        bool saveFile = false;
        if( QFileInfo::exists( "./XML/camPerfils/" + tmpName + ".xml" ) ){
            if( funcShowMsgYesNo("Alert","Replace existent file?") ){
                QFile file( "./XML/camPerfils/" + tmpName + ".xml" );
                file.remove();
                saveFile = true;
            }
        }else{
            saveFile = true;
        }

        if( saveFile ){
            if( saveRaspCamSettings( tmpName ) ){
                funcShowMsg("Success","File stored successfully");
            }else{
                funcShowMsg("ERROR","Saving raspcamsettings");
            }
            /*
            //Prepare file
            QString newFileCon = "";
            newFileCon.append("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
            newFileCon.append("<settings>\n");
            newFileCon.append("    <AWB>"+ ui->cbAWB->currentText() +"</AWB>\n");
            newFileCon.append("    <Exposure>"+ ui->cbExposure->currentText() +"</Exposure>\n");
            newFileCon.append("    <Brightness>"+ QString::number(ui->slideBrightness->value() ) +"</Brightness>\n");
            newFileCon.append("    <Sharpness>"+ QString::number(ui->slideSharpness->value() ) +"</Sharpness>\n");
            newFileCon.append("    <Contrast>"+ QString::number(ui->slideContrast->value() ) +"</Contrast>\n");
            newFileCon.append("    <Saturation>"+ QString::number(ui->slideSaturation->value() ) +"</Saturation>\n");
            newFileCon.append("    <ShutterSpeed>"+ QString::number(ui->slideShuterSpeed->value() ) +"</ShutterSpeed>\n");
            newFileCon.append("    <ISO>"+ QString::number(ui->slideISO->value() ) +"</ISO>\n");
            newFileCon.append("    <ExposureCompensation>"+ QString::number(ui->slideExpComp->value() ) +"</ExposureCompensation>\n");
            if( ui->rbFormat1->isChecked() ){
                newFileCon.append("    <Format>1</Format>\n");
            }else{
                newFileCon.append("    <Format>2</Format>\n");
            }
            newFileCon.append("    <Red>"+ QString::number(ui->slideRed->value() ) +"</Red>\n");
            newFileCon.append("    <Green>"+ QString::number(ui->slideGreen->value() ) +"</Green>\n");
            newFileCon.append("</settings>");

            //Save
            ui->txtCamParamXMLName->setText(tmpName);
            QFile newFile( "./XML/camPerfils/" + tmpName + ".xml" );
            if ( newFile.open(QIODevice::ReadWrite) ){
                QTextStream stream( &newFile );
                stream << newFileCon << endl;
                newFile.close();
                funcShowMsg("Success","File stored successfully");
            }
            */
        }
    }
}

bool MainWindow::saveRaspCamSettings( QString tmpName ){
    //Prepare file contain
    //..
    QString newFileCon = "";
    QString denoiseFlag = (ui->cbDenoise->isChecked())?"1":"0";
    QString colbalFlag = (ui->cbColorBalance->isChecked())?"1":"0";
    //QString previewFlag = (ui->cbPreview->isChecked())?"1":"0";
    QString oneShotFlag = (ui->cbOneShot->isChecked())?"1":"0";
    QString fullPhotoFlag = (ui->cbFullPhoto->isChecked())?"1":"0";
    newFileCon.append("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    newFileCon.append("<settings>\n");
    newFileCon.append("    <AWB>"+ ui->cbAWB->currentText() +"</AWB>\n");
    newFileCon.append("    <Exposure>"+ ui->cbExposure->currentText() +"</Exposure>\n");
    newFileCon.append("    <Denoise>"+ denoiseFlag +"</Denoise>\n");
    newFileCon.append("    <ColorBalance>"+ colbalFlag +"</ColorBalance>\n");
    newFileCon.append("    <FullPhoto>"+ fullPhotoFlag +"</FullPhoto>\n");
    //newFileCon.append("    <Preview>"+ previewFlag +"</Preview>\n");
    newFileCon.append("    <OneShot>"+ oneShotFlag +"</OneShot>\n");
    newFileCon.append("    <TriggerTime>"+ QString::number( ui->slideTriggerTime->value() ) +"</TriggerTime>\n");
    newFileCon.append("    <ShutterSpeed>"+ QString::number( ui->slideShuterSpeed->value() ) +"</ShutterSpeed>\n");
    newFileCon.append("    <ShutterSpeedSmall>"+ QString::number( ui->slideShuterSpeedSmall->value() ) +"</ShutterSpeedSmall>\n");
    newFileCon.append("    <ISO>"+ QString::number( ui->slideISO->value() ) +"</ISO>\n");
    newFileCon.append("</settings>");
    //Save
    //..
    QFile newFile( "./XML/camPerfils/" + tmpName + ".xml" );
    if(newFile.exists())newFile.remove();
    if ( newFile.open(QIODevice::ReadWrite) ){
        QTextStream stream( &newFile );
        stream << newFileCon << endl;
        newFile.close();
    }else{
        return false;
    }
    return true;
}

void MainWindow::on_pbObtPar_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select XML..."),
                                                        "./XML/camPerfils/",
                                                        "(*.xml);;"
                                                     );
    if( !filePath.isEmpty() ){
        QStringList tmpList = filePath.split("/");
        tmpList = tmpList.at( tmpList.count()-1 ).split(".");
        ui->txtCamParamXMLName->setText( tmpList.at( 0 ) );

        funcGetRaspParamFromXML( raspcamSettings, filePath );
        funcIniCamParam( raspcamSettings );
    }
}



bool MainWindow::funcSetCam( structRaspcamSettings *raspcamSettings ){


        memcpy(
                    raspcamSettings->AWB,
                    ui->cbAWB->currentText().toStdString().c_str(),
                    sizeof(raspcamSettings->AWB)
              );

        memcpy(
                    raspcamSettings->Exposure,
                    ui->cbExposure->currentText().toStdString().c_str(),
                    sizeof(raspcamSettings->Exposure)
              );


        //raspcamSettings->width = ui->slideWidth->value();

        //raspcamSettings->height = ui->slideHeight->value();

        //raspcamSettings->Brightness = ui->slideBrightness->value();

        //raspcamSettings->Sharpness = ui->slideSharpness->value();

        //raspcamSettings->Contrast = ui->slideContrast->value();

        //raspcamSettings->Saturation = ui->slideSaturation->value();

        raspcamSettings->ShutterSpeed = ui->slideShuterSpeed->value();

        raspcamSettings->ISO = ui->slideISO->value();

        //raspcamSettings->ExposureCompensation = ui->slideExpComp->value();

        //raspcamSettings->Red = ui->slideRed->value();

        //raspcamSettings->Green = ui->slideGreen->value();



    /*
    memcpy(raspcamSettings->AWB,"AUTO\0",5);            // OFF,AUTO,SUNLIGHT,CLOUDY,TUNGSTEN,FLUORESCENT,INCANDESCENT,FLASH,HORIZON
    memcpy(raspcamSettings->Exposure,"FIREWORKS\0",6);      // OFF,AUTO,NIGHT,NIGHTPREVIEW,BACKLIGHT,SPOTLIGHT,SPORTS,SNOW,BEACH,VERYLONG,FIXEDFPS,ANTISHAKE,FIREWORKS

    raspcamSettings->width                  = 384;      // 1280 to 2592
    raspcamSettings->height                 = 288;      // 960 to 1944
    raspcamSettings->Brightness             = 50;       // 0 to 100
    raspcamSettings->Sharpness              = 0;        // -100 to 100
    raspcamSettings->Contrast               = 0;        // -100 to 100
    raspcamSettings->Saturation             = 0;        // -100 to 100
    raspcamSettings->ShutterSpeed           = 1000*75;  // microsecs (max 330000)
    raspcamSettings->ISO                    = 400;      // 100 to 800
    raspcamSettings->ExposureCompensation   = 0;        // -10 to 10
    raspcamSettings->Format                 = 2;        // 1->raspicam::RASPICAM_FORMAT_GRAY | 2->raspicam::RASPICAM_FORMAT_YUV420
    raspcamSettings->Red                    = 0;        // 0 to 8 set the value for the RED component of white balance
    raspcamSettings->Green                  = 0;        // 0 to 8 set the value for the GREEN component of white balance
    */
    return true;
}


void MainWindow::funcGetSnapshot()
{

    mouseCursorWait();

    //Save path
    //..
    saveFile(_PATH_LAST_SNAPPATH,ui->txtSnapPath->text());


    //Set required image's settings
    //..
    strReqImg *reqImg   = (strReqImg*)malloc(sizeof(strReqImg));
    reqImg->idMsg       = (unsigned char)7;
    reqImg->raspSett    = funcFillSnapshotSettings( reqImg->raspSett );    

    //Define photo's region
    //..
    if( ui->cbFullPhoto->isChecked() ){
        reqImg->needCut     = false;
        reqImg->imgCols     = _BIG_WIDTH;//2592 | 640
        reqImg->imgRows     = _BIG_HEIGHT;//1944 | 480
    }else{
        if( !funGetSquareXML( _PATH_REGION_OF_INTERES, &reqImg->sqApSett ) ){
            funcShowMsg("ERROR","Loading squareAperture.xml");
            return (void)false;
        }
        reqImg->needCut     = true;
        //Calculate real number of columns of the required photo
        reqImg->imgCols         = round( ((float)reqImg->sqApSett.rectW/(float)reqImg->sqApSett.canvasW) * (float)_BIG_WIDTH);
        reqImg->imgRows         = round( ((float)reqImg->sqApSett.rectH/(float)reqImg->sqApSett.canvasH) * (float)_BIG_HEIGHT);
        reqImg->sqApSett.rectX  = round( ((float)reqImg->sqApSett.rectX/(float)reqImg->sqApSett.canvasW) * (float)_BIG_WIDTH);
        reqImg->sqApSett.rectY  = round( ((float)reqImg->sqApSett.rectY/(float)reqImg->sqApSett.canvasH) * (float)_BIG_HEIGHT);

        //qDebug() << "canvasW: " << reqImg->sqApSett.canvasW;
        //qDebug() << "canvasH: " << reqImg->sqApSett.canvasH;
        //qDebug() << "rectX: " << reqImg->sqApSett.rectX;
        //qDebug() << "rectY: " << reqImg->sqApSett.rectY;
        //qDebug() << "reqImg->imgCols: " << reqImg->imgCols;
        //qDebug() << "reqImg->imgRows: " << reqImg->imgRows;

        //return (void)true;
    }

    //Require remote image
    //...
    //It saves image into HDD: _PATH_IMAGE_RECEIVED
    funcGetRemoteImg( reqImg );
    QImage tmpImg( _PATH_IMAGE_RECEIVED );

    //Show snapshot
    //..
    QImage tmpThumb = tmpImg.scaledToHeight(_FRAME_THUMB_H);
    tmpThumb = tmpThumb.scaledToWidth(_FRAME_THUMB_W);
    ui->labelVideo->setPixmap( QPixmap::fromImage(tmpThumb) );
    ui->labelVideo->setFixedSize( tmpThumb.width(), tmpThumb.height() );

    //Save snapshot with a timestamp file-name generated automatically
    //..
    QString tmpFileName = ui->txtSnapPath->text().trimmed();
    tmpFileName.append(QString::number(time(NULL)));
    tmpFileName.append(".RGB888");
    QFile::copy(_PATH_IMAGE_RECEIVED, tmpFileName);
    qDebug() << "tmpFileName: " << tmpFileName;


}


void MainWindow::on_pbSnapshot_clicked()
{
    ui->pbStartVideo->setEnabled(false);


    funcGetSnapshot();


    ui->pbStartVideo->setEnabled(true);
}




void MainWindow::on_pbExpIds_clicked()
{
    /*
    int screen_width =  1000;
    int screen_height = 700;

    QGraphicsScene *scene = new QGraphicsScene(0,0,screen_width,screen_height);
    QPixmap pim(imgPath);
    scene->setBackgroundBrush(pim.scaled(screen_width,screen_height,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    */
}

void MainWindow::funcPutImageIntoGV(QGraphicsView *gvContainer, QString imgPath){    
    QPixmap pim(imgPath);
    QGraphicsScene *scene = new QGraphicsScene(0,0,pim.width(),pim.height());
    scene->setBackgroundBrush(pim);
    gvContainer->setScene(scene);
    gvContainer->resize(pim.width(),pim.height());
    gvContainer->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    gvContainer->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    /*
    QHBoxLayout *layout = new QHBoxLayout;
    ui->tab_3->setLayout(layout);
    ui->tab_3->layout()->addWidget(gvContainer);
    layout->setEnabled(false);
    */
}


void MainWindow::on_pbPtsClearAll_clicked()
{
    lstPixSelAux->clear();

    canvasAux->scene()->clear();

    lstBorder->clear();
    lstSelPix->clear();


}


void MainWindow::on_pbSavePixs_clicked()
{
    if( lstSelPix->isEmpty() ){
        funcShowMsg("LACK","Not pixels to export");
    }else{
        int i;
        QString filePath = "./Results/lstPixels.txt";
        QFile fileLstPixels(filePath);
        if (!fileLstPixels.open(QIODevice::WriteOnly | QIODevice::Text)){
            funcShowMsg("ERROR","Creating file fileLstPixels");
        }else{
            QTextStream streamLstPixels(&fileLstPixels);
            for( i=0; i<lstSelPix->count(); i++ ){
                streamLstPixels << QString::number(lstSelPix->at(i).first) << " "<< QString::number(lstSelPix->at(i).second) << "\n";
            }
        }
        fileLstPixels.close();
        funcShowMsg("Success","List of pixels exported into: "+filePath);
    }
}


/*
bool MainWindow::on_pb2XY_clicked()
{
    //Validate that exist pixel selected
    //..
    if( lstSelPix->count()<1){
        funcShowMsg("Lack","Not pixels selected");
        return false;
    }

    //Create xycolor space
    //..
    GraphicsView *xySpace = new GraphicsView(this);
    funcPutImageIntoGV(xySpace, "./imgResources/CIEManual.png");
    xySpace->setWindowTitle( "XY color space" );
    xySpace->show();

    //Transform each pixel from RGB to xy and plot it
    //..
    QImage tmpImg(imgPath);
    QRgb tmpPix;
    colSpaceXYZ *tmpXYZ = (colSpaceXYZ*)malloc(sizeof(colSpaceXYZ));
    int tmpOffsetX = -13;
    int tmpOffsetY = -55;
    int tmpX, tmpY;
    int i;
    for( i=0; i<lstSelPix->count(); i++ ){
        tmpPix = tmpImg.pixel( lstSelPix->at(i).first, lstSelPix->at(i).second );
        funcRGB2XYZ( tmpXYZ, (float)qRed(tmpPix), (float)qGreen(tmpPix), (float)qBlue(tmpPix) );
        //funcRGB2XYZ( tmpXYZ, 255.0, 0, 0  );
        tmpX = floor( (tmpXYZ->x * 441.0) / 0.75 ) + tmpOffsetX;
        tmpY = 522 - floor( (tmpXYZ->y * 481.0) / 0.85 ) + tmpOffsetY;
        funcAddPoit2Graph( xySpace, tmpX, tmpY, 1, 1,
                           QColor(qRed(tmpPix),qGreen(tmpPix),qBlue(tmpPix)),
                           QColor(qRed(tmpPix),qGreen(tmpPix),qBlue(tmpPix))
                         );
    }

    //Save image plotted
    //..
    QPixmap pixMap = QPixmap::grabWidget(xySpace);
    pixMap.save("./Results/Miscelaneas/RGBPloted.png");

    return true;
}
*/


/*
void MainWindow::on_pbLoadImg_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"),"./imgResources/",tr("Image files (*.*)"));
    imgPath = fileNames.at(0);
    funcPutImageIntoGV( myCanvas, imgPath );
    //funcShowMsg("",fileNames.at(0));
}
*/

/*
void MainWindow::on_pbUpdCut_clicked()
{

    //Set Canvas dimensions
    //..
    int maxW = 640;
    int maxH = 480;

    QString imgPath = "./imgResources/tmpBigToCut.ppm";

    if( funcShowMsgYesNo("Alert","Get remote image?") ){
        //Set required image's settings
        //..
        ui->progBar->setVisible(true);
        strReqImg *reqImg       = (strReqImg*)malloc(sizeof(strReqImg));
        reqImg->idMsg           = (unsigned char)7;
        reqImg->needCut         = false;
        reqImg->stabSec         = 3;
        reqImg->imgCols         = _BIG_WIDTH;//2592 | 640
        reqImg->imgRows         = _BIG_HEIGHT;//1944 | 480
        reqImg->raspSett        = funcFillSnapshotSettings( reqImg->raspSett );
        unsigned char *tmpFrame = funcGetRemoteImg( reqImg );
        std::ofstream outFile ( imgPath.toStdString(), std::ios::binary );
        outFile<<"P6\n"<<reqImg->imgCols<<" "<<reqImg->imgRows<<" 255\n";
        outFile.write( (char*)tmpFrame, 3*reqImg->imgCols*reqImg->imgRows );
        outFile.close();
        //delete tmpFrame;
        qDebug() << "Big image saved locally";
        ui->progBar->setVisible(false);
    }

    //Load image
    //..
    QImage imgOrig(imgPath);
    QImage tmpImg = imgOrig.scaled(QSize(640,480),Qt::KeepAspectRatio);
    QGraphicsScene *scene = new QGraphicsScene(0,0,tmpImg.width(),tmpImg.height());
    scene->setBackgroundBrush( QPixmap::fromImage(tmpImg) );
    ui->gvCut->setScene( scene );

    //Set UI
    //..
    ui->gvCut->setFixedSize( maxW, maxH );
    ui->gvCut->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->gvCut->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->slideCutPosX->setMaximum( maxW );
    ui->slideCutPosY->setMaximum(maxH);
    ui->slideCutWX->setMaximum( maxW );
    ui->slideCutWY->setMaximum( maxH );
    //ui->slideCutWX->setMinimum(5);
    //ui->slideCutWY->setMinimum(5);
    ui->slideCutWX->setEnabled(true);
    ui->slideCutWY->setEnabled(true);
    ui->slideCutPosX->setEnabled(true);
    ui->slideCutPosY->setEnabled(true);
    ui->pbSaveSquare->setEnabled(true);
    ui->pbSaveBigSquare->setEnabled(true);


}



void MainWindow::funcSetLines(){
    int xPos    = ui->slideCutPosX->value();
    int xW      = ui->slideCutWX->value();
    int yPos    = ui->slideCutPosY->value();
    int yW      = ui->slideCutWY->value();
    ui->gvCut->scene()->clear();
    ui->gvCut->scene()->addLine(xPos,1,xPos,ui->gvCut->height(),QPen(Qt::red));
    ui->gvCut->scene()->addLine(xPos+xW,1,xPos+xW,ui->gvCut->height(),QPen(Qt::red));
    ui->gvCut->scene()->addLine(1,ui->gvCut->height()-yPos,ui->gvCut->width(),ui->gvCut->height()-yPos,QPen(Qt::red));
    ui->gvCut->scene()->addLine(1,ui->gvCut->height()-yPos-yW,ui->gvCut->width(),ui->gvCut->height()-yPos-yW,QPen(Qt::red));
}
*/

/*
void MainWindow::on_slideCutPosX_valueChanged(int xpos)
{
    xpos = xpos;
    funcSetLines();
}

void MainWindow::on_slideCutWX_valueChanged(int value)
{
    value = value;
    funcSetLines();
}

void MainWindow::on_slideCutWY_valueChanged(int value)
{
    value = value;
    funcSetLines();
}

void MainWindow::on_slideCutPosY_valueChanged(int value)
{
    value = value;
    funcSetLines();
}
*/
/*
void MainWindow::on_pbSaveSquare_clicked()
{
    if( funcShowMsgYesNo("Alert","Do you want to replace the setting?") == 1 ){
        if( funcSaveRect ( "./XML/squareAperture.xml" ) ){
            funcShowMsg("Success","Settings saved");
        }else{
            funcShowMsg("ERROR","It can NOT save settings");
        }
    }
}
*/

/*
bool MainWindow::funcSaveRect( QString fileName ){
    QString tmpContain;
    tmpContain.append( "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" );
    tmpContain.append("<Variables>\n");
    tmpContain.append("\t<width>"+ QString::number(ui->gvCut->width()) +"</width>\n");
    tmpContain.append("\t<height>"+ QString::number(ui->gvCut->height()) +"</height>\n");
    tmpContain.append("\t<x1>"+ QString::number( ui->slideCutPosX->value() ) +"</x1>\n");
    tmpContain.append("\t<y1>"+ QString::number( ui->gvCut->height() - ui->slideCutPosY->value() - ui->slideCutWY->value() ) +"</y1>\n");
    tmpContain.append("\t<x2>"+ QString::number( ui->slideCutPosX->value() + ui->slideCutWX->value() ) +"</x2>\n");
    tmpContain.append("\t<y2>"+ QString::number( ui->gvCut->height() - ui->slideCutPosY->value() ) +"</y2>\n");
    tmpContain.append("</Variables>");
    if( !saveFile( fileName, tmpContain ) ){
        return false;
    }
    return true;
}
*/

/*
void MainWindow::on_pbSaveBigSquare_clicked()
{
    if( funcShowMsgYesNo("Alert","Do you want to replace the setting?") == 1 ){
        if( funcSaveRect ( "./XML/bigSquare.xml" ) ){
            funcShowMsg("Success","Settings saved");
        }else{
            funcShowMsg("ERROR","It can NOT save settings");
        }
    }
}
*/

void MainWindow::on_pbSpecSnapshot_clicked()
{
    //Turn on camera
    //..
    CvCapture* usbCam  = cvCreateCameraCapture( ui->sbSpecUsb->value() );
    cvSetCaptureProperty( usbCam,  CV_CAP_PROP_FRAME_WIDTH,  320*_FACT_MULT );
    cvSetCaptureProperty( usbCam,  CV_CAP_PROP_FRAME_HEIGHT, 240*_FACT_MULT );
    assert( usbCam );

    //Create image
    //..    
    IplImage *tmpCam = cvQueryFrame( usbCam );
    if( ( tmpCam = cvQueryFrame( usbCam ) ) ){
        //Get the image
        QString tmpName = "./snapshots/tmpUSB.png";
        if( _USE_CAM ){
            IplImage *imgRot = cvCreateImage(
                                                cvSize(tmpCam->height,tmpCam->width),
                                                tmpCam->depth,
                                                tmpCam->nChannels
                                            );
            cvTranspose(tmpCam,imgRot);
            cvTranspose(tmpCam,imgRot);
            cvTranspose(tmpCam,imgRot);
            cv::imwrite( tmpName.toStdString(), cv::Mat(imgRot, true) );
        }
        //Create canvas
        //Display accum
        //..        

        //Show image
        QPixmap pix(tmpName);        
        pix = pix.scaledToHeight( _GRAPH_HEIGHT );
        QGraphicsScene *scene = new QGraphicsScene(0,0,pix.width(),pix.height());
        canvasSpec->setBackgroundBrush(pix);
        canvasSpec->setScene( scene );
        canvasSpec->resize(pix.width(),pix.height());
        canvasSpec->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        canvasSpec->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        ui->tab_5->layout()->addWidget(canvasSpec);
        ui->tab_5->layout()->setEnabled(false);
        ui->tab_5->layout()->setAlignment(Qt::AlignLeft);
        ui->gridColors->setAlignment(Qt::AlignLeft);
        ui->gridColors->setAlignment(Qt::AlignLeft);


        ui->slideRedLen->setMaximumWidth(pix.width());
        ui->slideGreenLen->setMaximumWidth(pix.width());
        ui->slideBlueLen->setMaximumWidth(pix.width());

        pix.save("./snapshots/tmpThumb.png");




    }else{
        funcShowMsg("ERROR", "cvQueryFrame empty");
    }
    cvReleaseCapture(&usbCam);

    //Reset slides
    ui->slideRedLen->setValue(0);
    ui->slideGreenLen->setValue(0);
    ui->slideBlueLen->setValue(0);

    //Reset lines
    funcUpdateColorSensibilities();
    funcDrawLines(0,0,0,0);

}

void MainWindow::funcBeginRect(QMouseEvent* e){
    calStruct.x1 = e->x();
    calStruct.y1 = e->y();
    qDebug() << "funcBeginRect";
}

void MainWindow::funcEndRect(QMouseEvent* e, GraphicsView *tmpCanvas){
    calStruct.x2 = e->x();
    calStruct.y2 = e->y();
    ui->pbSpecCut->setEnabled(true);
    qDeleteAll(tmpCanvas->scene()->items());
    int x1, y1, x2, y2, w, h;
    x1 = (calStruct.x1<=e->x())?calStruct.x1:e->x();
    x2 = (calStruct.x1>=e->x())?calStruct.x1:e->x();
    y1 = (calStruct.y1<=e->y())?calStruct.y1:e->y();
    y2 = (calStruct.y1>=e->y())?calStruct.y1:e->y();
    w  = abs(x2-x1);
    h  = abs(y2-y1);
    QPoint p1, p2;
    p1.setX(x1);   p1.setY(y1);
    p2.setX(w);    p2.setY(h);
    customRect* tmpRect = new customRect(p1,p2);
    tmpRect->mapToScene(p1.x(),p1.y(),p2.x(),p2.y());
    //customRect* tmpRect = new customRect(this);
    tmpRect->parameters.W = canvasCalib->scene()->width();
    tmpRect->parameters.H = canvasCalib->scene()->height();
    tmpRect->setPen( QPen(Qt::red) );
    tmpCanvas->scene()->addItem(tmpRect);    
    tmpRect->setFocus();
    tmpRect->parameters.movible = true;
    tmpRect->parameters.canvas = tmpCanvas;
    tmpRect->parameters.backgroundPath = auxQstring;
}

void MainWindow::funcAnalizeAreaSelected(QPoint p1, QPoint p2){
    p1 = p1;
    p2 = p2;
}

void MainWindow::on_chbRed_clicked()
{
    funcUpdateColorSensibilities();
    funcDrawLines(0,0,0,0);
}

void MainWindow::on_chbGreen_clicked()
{
    funcUpdateColorSensibilities();
    funcDrawLines(0,0,0,0);
}

void MainWindow::on_chbBlue_clicked()
{
    funcUpdateColorSensibilities();
    funcDrawLines(0,0,0,0);
}

void MainWindow::funcUpdateColorSensibilities(){
    //Accumulate values in each color
    //..
    canvasSpec->scene()->clear();
    QImage tmpImg( "./snapshots/tmpThumb.png" );
    int Red[tmpImg.width()];memset(Red,'\0',tmpImg.width());
    int Green[tmpImg.width()];memset(Green,'\0',tmpImg.width());
    int Blue[tmpImg.width()];memset(Blue,'\0',tmpImg.width());
    int r, c, maxR, maxG, maxB, xR, xG, xB;
    maxR = 0;
    maxG = 0;
    maxB = 0;
    QRgb pixel;
    for(c=0;c<tmpImg.width();c++){
        Red[c]   = 0;
        Green[c] = 0;
        Blue[c]  = 0;
        for(r=0;r<tmpImg.height();r++){
            if(!tmpImg.valid(QPoint(c,r))){
                qDebug() << "Invalid r: " << r << "c: "<<c;
                qDebug() << "tmpImg.width(): " << tmpImg.width();
                qDebug() << "tmpImg.height(): " << tmpImg.height();
                return (void)NULL;
                close();
            }
            pixel     = tmpImg.pixel(QPoint(c,r));
            Red[c]   += qRed(pixel);
            Green[c] += qGreen(pixel);
            Blue[c]  += qBlue(pixel);
        }
        Red[c]   = round((float)Red[c]/tmpImg.height());
        Green[c] = round((float)Green[c]/tmpImg.height());
        Blue[c]  = round((float)Blue[c]/tmpImg.height());
        if( Red[c] > maxR ){
            maxR = Red[c];
            xR = c;
        }
        if( Green[c] > maxG ){
            maxG = Green[c];
            xG = c;
        }
        if( Blue[c] > maxB ){
            maxB = Blue[c];
            xB = c;
        }
        //qDebug() << "xR: " << xR << "xG: " << xG << "xB: " << xB;
    }

    //Move slides
    //..
    ui->slideRedLen->setMaximum(tmpImg.width());
    ui->slideGreenLen->setMaximum(tmpImg.width());
    ui->slideBlueLen->setMaximum(tmpImg.width());

    ui->slideRedLen->setValue(xR);
    ui->slideGreenLen->setValue(xG);
    ui->slideBlueLen->setValue(xB);

    //qDebug() << "c" << c << "maxR:"<<maxR<<" maxG:"<<maxG<<" maxB:"<<maxB;
    //qDebug() << "c" << c << "xR:"<<xR<<" xG:"<<xG<<" xB:"<<xB;
    //qDebug() << "tmpImg.width(): " << tmpImg.width();
    //qDebug() << "tmpImg.height(): " << tmpImg.height();

    //Draw lines and locate lines
    //..
    int tmpPoint1, tmpPoint2, tmpHeight;
    //tmpHeight = _GRAPH_HEIGHT;
    tmpHeight = tmpImg.height();
    for(c=1;c<tmpImg.width();c++){
        if( ui->chbRed->isChecked() ){
            tmpPoint1 = tmpHeight-Red[c-1];
            tmpPoint2 = tmpHeight-Red[c];
            //tmpPoint1 = tmpImg.height()-Red[c-1];
            //tmpPoint2 = tmpImg.height()-Red[c];
            canvasSpec->scene()->addLine( c, tmpPoint1, c+1, tmpPoint2, QPen(QColor("#FF0000")) );
        }
        if( ui->chbGreen->isChecked() ){
            tmpPoint1 = tmpHeight-Green[c-1];
            tmpPoint2 = tmpHeight-Green[c];
            canvasSpec->scene()->addLine( c, tmpPoint1, c+1, tmpPoint2, QPen(QColor("#00FF00")) );
        }
        if( ui->chbBlue->isChecked() ){
            tmpPoint1 = tmpHeight-Blue[c-1];
            tmpPoint2 = tmpHeight-Blue[c];
            canvasSpec->scene()->addLine( c, tmpPoint1, c+1, tmpPoint2, QPen(QColor("#0000FF")) );
        }        
    }
}

void MainWindow::funcDrawLines(int flag, int xR, int xG, int xB){
    //Draw lines
    //..

    ui->slideRedLen->setEnabled(true);
    ui->slideGreenLen->setEnabled(true);
    ui->slideBlueLen->setEnabled(true);

    if( flag == 0 ){//Get x from slides
        xR = ui->slideRedLen->value();
        xG = ui->slideGreenLen->value();
        xB = ui->slideBlueLen->value();
    }

    //qDebug() << "flag:" << flag << "xR:" << xR << " xG:" << xG << "xB: " << xB;

    canvasSpec->scene()->addLine( xR,1,xR,canvasSpec->height(),QPen(QColor("#FF0000")) );
    canvasSpec->scene()->addLine( xG,1,xG,canvasSpec->height(),QPen(QColor("#00FF00")) );
    canvasSpec->scene()->addLine( xB,1,xB,canvasSpec->height(),QPen(QColor("#0000FF")) );

    ui->slideRedLen->setValue(xR);
    ui->slideGreenLen->setValue(xG);
    ui->slideBlueLen->setValue(xB);

}

void MainWindow::on_pbSpecCut_clicked()
{
    //Prepare variables
    //..
    int w, h, W, H;
    QPixmap tmpPix("./snapshots/tmpUSB.png");
    W = tmpPix.width();
    H = tmpPix.height();
    w = canvasSpec->width();
    h = canvasSpec->height();
    //Extrapolate dimensions
    qDebug() << calStruct.x1 << ", " << calStruct.y1;
    qDebug() << calStruct.x2 << ", " << calStruct.y2;
    calStruct.X1 = round( ((float)W/(float)w)*(float)calStruct.x1 );
    calStruct.Y1 = round( ((float)H/(float)h)*(float)calStruct.y1 );
    calStruct.X2 = round( ((float)W/(float)w)*(float)calStruct.x2 );
    calStruct.Y2 = round( ((float)H/(float)h)*(float)calStruct.y2 );
    calStruct.lenW = abs(calStruct.X2-calStruct.X1);
    calStruct.lenH = abs(calStruct.Y2-calStruct.Y1);
    qDebug() << calStruct.X1 << ", " << calStruct.Y1;
    qDebug() << calStruct.X2 << ", " << calStruct.Y2;
    qDebug() << "lenW=" << calStruct.lenW;
    qDebug() << "lenH=" << calStruct.lenH;

    //Crop image
    //..
    //qDeleteAll(canvasSpec->scene()->items());
    QPixmap cropped = tmpPix.copy( QRect( calStruct.X1, calStruct.Y1, calStruct.lenW, calStruct.lenH ) );
    cropped.save("./snapshots/tmpThumb.png");
    cropped.save("./snapshots/tmpUSB.png");
    QGraphicsScene *scene = new QGraphicsScene(0,0,cropped.width(),cropped.height());
    canvasSpec->setBackgroundBrush(cropped);
    canvasSpec->setCacheMode(QGraphicsView::CacheNone);
    canvasSpec->setScene( scene );
    canvasSpec->resize(cropped.width(),cropped.height());
    canvasSpec->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    canvasSpec->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->tab_5->layout()->addWidget(canvasSpec);
    ui->tab_5->layout()->setEnabled(false);
    ui->tab_5->layout()->setAlignment(Qt::AlignLeft);
    //Set slide max
    ui->slideRedLen->setMaximumWidth(cropped.width());
    ui->slideGreenLen->setMaximumWidth(cropped.width());
    ui->slideBlueLen->setMaximumWidth(cropped.width());



    //Update lines
    //..
    funcUpdateColorSensibilities();

    //Update lines
    //..
    funcDrawLines(0,0,0,0);

    //int tmpCanvW = _GRAPH_HEIGHT;
    int tmpCanvW = cropped.height();
    canvasSpec->resize(QSize(cropped.width(),tmpCanvW));


    ui->pbViewBack->setEnabled(true);
    ui->pbSnapCal->setEnabled(true);



}

void MainWindow::on_slideRedLen_sliderReleased()
{
    int x1, x2, x3;
    x1 = ui->slideRedLen->value();
    x2 = ui->slideGreenLen->value();
    x3 = ui->slideBlueLen->value();
    funcUpdateColorSensibilities();
    funcDrawLines(1,x1,x2,x3);
    qDebug() << "Slide released";
}


void MainWindow::on_slideBlueLen_sliderReleased()
{
    int x1, x2, x3;
    x1 = ui->slideRedLen->value();
    x2 = ui->slideGreenLen->value();
    x3 = ui->slideBlueLen->value();
    funcUpdateColorSensibilities();
    funcDrawLines(1,x1,x2,x3);
    qDebug() << "Slide released";
}

void MainWindow::on_slideGreenLen_sliderReleased()
{
    int x1, x2, x3;
    x1 = ui->slideRedLen->value();
    x2 = ui->slideGreenLen->value();
    x3 = ui->slideBlueLen->value();
    funcUpdateColorSensibilities();
    funcDrawLines(1,x1,x2,x3);
    qDebug() << "Slide released";
}


void MainWindow::on_pbViewBack_clicked()
{
    //canvasSpec->scene()->clear();
    //QGraphicsScene *scene = new QGraphicsScene(0,0,canvasSpec->scene()->width(),canvasSpec->scene()->height());
    //QPixmap backBlack("./imgResources/backBlack.png");
    //scene->setBackgroundBrush(backBlack);
    //canvasSpec->setScene(scene);
    canvasSpec->scene()->invalidate(1,1,200,100);

    canvasSpec->setStyleSheet("background-color: black;");


}

void MainWindow::on_pbSnapCal_clicked()
{
    QString imgCropPath = "./snapshots/tmp/calibCrop.png";

    //Get image from camera
    //..
    QString imgPath = "./snapshots/tmp/calib.png";
    if( _USE_CAM ){
        IplImage *imgCal = funcGetImgFromCam( ui->sbSpecUsb->value(), 500 );
        cv::imwrite( imgPath.toStdString(), cv::Mat(imgCal, true) );
    }

    //Cut image
    //..
    QPixmap tmpOrigPix(imgPath);
    QPixmap tmpImgCrop = tmpOrigPix.copy( QRect( calStruct.X1, calStruct.Y1, calStruct.lenW, calStruct.lenH ) );
    tmpImgCrop.save(imgCropPath);

    //Analyze the image
    //..
    QImage *tmpImg = new QImage(imgCropPath);
    colorAnalyseResult *imgSumary = funcAnalizeImage(tmpImg);

    //Calculate wavelen
    //..
    //int tmpX;
    float tmpWave;
    float pixX[] = {
                        (float)ui->slideBlueLen->value(),
                        (float)ui->slideGreenLen->value(),
                        (float)ui->slideRedLen->value()
                   };
    linearRegresion *linReg = funcCalcLinReg( pixX );
    tmpWave = linReg->a + ( linReg->b * (float)imgSumary->maxMaxx );

    //Display scene with value calculated
    //..
    GraphicsView *wavelen = new GraphicsView(this);
    funcPutImageIntoGV(wavelen, imgCropPath);
    wavelen->setWindowTitle( "x("+ QString::number(imgSumary->maxMaxx) +
                             ") | Wavelen: " +
                             QString::number(tmpWave) );
    QString tmpColor = "#FF0000";
    if(imgSumary->maxMaxColor==2)tmpColor="#00FF00";
    if(imgSumary->maxMaxColor==3)tmpColor="#0000FF";
    wavelen->scene()->addLine(
                                    imgSumary->maxMaxx,
                                    1,
                                    imgSumary->maxMaxx,
                                    tmpImg->height(),
                                    QPen( QColor( tmpColor ) )
                             );
    wavelen->show();
}








void MainWindow::on_pbObtPar_2_clicked()
{
    //Select image
    //..
    auxQstring = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select image..."),
                                                        "./snapshots/Calib/",
                                                        "(*.ppm *.RGB888);;"
                                                     );
    if( auxQstring.isEmpty() ){
        return (void)NULL;
    }    

    //Create a copy of the image selected
    //..
    QImage origImg(auxQstring);
    qDebug() << "auxQstring: " << auxQstring;
    if( !origImg.save(_PATH_DISPLAY_IMAGE) ){
        funcShowMsg("ERROR","Creating image to display");
        return (void)NULL;
    }

    //Rotate if requires
    //..
    if( funcShowMsgYesNo("Alert","Rotate using saved rotation?") == 1 ){
        float rotAngle = getLastAngle();
        doImgRotation( rotAngle );
        globaIsRotated = true;
    }else{
        globaIsRotated = false;
    }

    //Refresh image in scene
    //..
    //Show image
    reloadImage2Display();

    //Load layout
    QLayout *layout = new QVBoxLayout();
    layout->addWidget(canvasCalib);
    layout->setEnabled(false);
    ui->tab_6->setLayout(layout);

    /*
    qDebug() << "tres";
    ui->tab_6->layout()->addWidget(canvasCalib);    
    qDebug() << "cuatro";
    ui->tab_6->layout()->setEnabled(false);
    ui->tab_6->layout()->setAlignment(Qt::AlignLeft);
    */

    //refreshGvCalib( auxQstring );

    //It enables slides
    //..
    //ui->containerCalSave->setEnabled(true);
    ui->toolBarDraw->setEnabled(true);
    ui->toolBarDraw->setVisible(true);
    //ui->slide2AxCalThre->setEnabled(true);



}



void MainWindow::refreshGvCalib( QString fileName )
{
    //Add image to graphic view
    //
    QImage imgOrig(fileName);
    QImage tmpImg = imgOrig.scaled(QSize(640,480),Qt::KeepAspectRatio);
    QGraphicsScene *scene = new QGraphicsScene(0,0,tmpImg.width(),tmpImg.height());
    canvasCalib->scene()->setBackgroundBrush( QPixmap::fromImage(tmpImg) );
    canvasCalib->resize(tmpImg.width(),tmpImg.height());
    scene->setBackgroundBrush( QPixmap::fromImage(tmpImg) );
    canvasCalib->setScene( scene );
    canvasCalib->resize(tmpImg.width(),tmpImg.height());
    //canvasCalib->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    //canvasCalib->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
}

void MainWindow::on_slide2AxCalPos_valueChanged(int value)
{
    value = value;
    updateCalibLine();
}

void MainWindow::updateCalibLine(){
    /*
    canvasCalib->scene()->clear();

    //Creates white line
    //..
    int x1,x2,y;
    y = canvasCalib->height() - ui->slide2AxCalPos->value();
    x1 = 0;
    x2 = canvasCalib->width();
    float rotAngle = -1.0*((float)ui->slide2AxCalRot->value()/5.0);
    canvasCalib->scene()->addLine(x1,y,x2,y,QPen(QColor("#FAE330")));
    ui->slide2AxCalPos->setToolTip( QString::number(ui->slide2AxCalPos->value()) );
    ui->slide2AxCalRot->setToolTip( QString::number(rotAngle) );
    ui->pbClearCalScene->setText("Clear line");
    */

}

void MainWindow::on_pbCalSaveRot_clicked()
{
    /*
    //Points from scene
    float rotAngle = -1.0*((float)ui->slide2AxCalRot->value()/5.0);
    qDebug() << "rotAngle: " << rotAngle;
    if( saveFile("./settings/calib/rotation.hypcam",QString::number(rotAngle)) ){
        funcShowMsg("Success","Rotation saved successfully: "+QString::number(rotAngle));
    }else{
        funcShowMsg("ERROR","Saving rotation");
    }
    */
}

void MainWindow::on_pbClearCalScene_clicked()
{
    /*
    if( ui->pbClearCalScene->text() == "Clear line" ){
        ui->pbClearCalScene->setText("Show line");
        canvasCalib->scene()->clear();
    }else{
        ui->pbClearCalScene->setText("Clear line");
        updateCalibLine();
    }
    */
}

/*
void MainWindow::on_slide2AxCalThre_valueChanged(int value)
{

    //Rotate image if requested
    //..
    if(globaIsRotated){
        float rotAngle = readAllFile( "./settings/calib/rotation.hypcam" ).trimmed().toFloat(0);
        QImage imgRot = funcRotateImage(auxQstring, rotAngle);
        imgRot.save(_PATH_DISPLAY_IMAGE);
    }
    //Apply threshold to the image
    //..
    QImage *imgThre = new QImage(auxQstring);
    funcImgThreshold( value, imgThre );
    QString tmpFilePaht = _PATH_DISPLAY_IMAGE;
    if( imgThre->save(tmpFilePaht) ){
        QtDelay(100);
        QPixmap pix(tmpFilePaht);
        pix = pix.scaledToHeight(_GRAPH_CALIB_HEIGHT);
        canvasCalib->setBackgroundBrush(pix);
        ui->slide2AxCalThre->setValue(value);
        ui->slide2AxCalThre->setToolTip(QString::number(value));
        ui->slide2AxCalThre->update();
        QtDelay(20);
    }

}
*/

void MainWindow::funcImgThreshold( int threshold, QImage *tmpImage ){
    int r, c;
    QRgb pix;
    for(r=0;r<tmpImage->height();r++){
        for(c=0;c<tmpImage->width();c++){
            pix = tmpImage->pixel(c,r);
            if( qRed(pix)<=threshold && qGreen(pix)<=threshold && qBlue(pix)<=threshold ){
                tmpImage->setPixel(QPoint(c,r),0);
            }
        }
    }
}


/*
void MainWindow::on_pbCalSaveTop_clicked()
{

    //Update image view
    //..
    QImage tmpImg( auxQstring );
    funcUpdateImgView( &tmpImg );

    //Crop image
    //..
    funcTransPix(
                    &calStruct,
                    canvasCalib->width(),
                    canvasCalib->height(),
                    tmpImg.width(),
                    tmpImg.height()
                );

    //Analize square selected
    //..
    QImage imgCrop = tmpImg.copy(
                                    calStruct.X1,
                                    calStruct.Y1,
                                    calStruct.lenW,
                                    calStruct.lenH
                                );
    colorAnalyseResult *tmpRes = funcAnalizeImage( &imgCrop );


    //Draw fluorescent RGB pixels
    //..
    int tmpX;
    //Clear scene
    canvasCalib->scene()->clear();
    //Regresh rec
    canvasCalib->scene()->addRect(calStruct.x1,calStruct.y1,calStruct.x2-calStruct.x1,calStruct.y2-calStruct.y1,QPen(QColor("#FF0000")));
    //Red
    tmpX = round(
                    (float)((calStruct.X1 + tmpRes->maxRx)*canvasCalib->width()) /
                    (float)tmpImg.width()
                );
    customLine *redLine = new customLine(QPoint(tmpX,0),QPoint(tmpX,canvasCalib->height()),QPen(Qt::red));
    canvasCalib->scene()->addItem(redLine);
    redLine->setToolTip("Red");
    redLine->parameters.name = "Vertical-Red-Right-Line";
    redLine->parameters.orientation = 2;
    redLine->parameters.lenght = canvasCalib->height();
    redLine->parameters.movible = false;
    //Green
    tmpX = round(
                    (float)((calStruct.X1 + tmpRes->maxGx)*canvasCalib->width()) /
                    (float)tmpImg.width()
                );
    customLine *GreenLine = new customLine(QPoint(tmpX,0),QPoint(tmpX,canvasCalib->height()),QPen(Qt::green));
    canvasCalib->scene()->addItem(GreenLine);
    GreenLine->setToolTip("Green");
    GreenLine->parameters.name = "Vertical-Green-Right-Line";
    GreenLine->parameters.orientation = 2;
    GreenLine->parameters.lenght = canvasCalib->height();
    GreenLine->parameters.movible = false;
    //Blue
    tmpX = round(
                    (float)((calStruct.X1 + tmpRes->maxBx)*canvasCalib->width()) /
                    (float)tmpImg.width()
                );
    customLine *BlueLine = new customLine(QPoint(tmpX,0),QPoint(tmpX,canvasCalib->height()),QPen(Qt::blue));
    canvasCalib->scene()->addItem(BlueLine);
    BlueLine->setToolTip("Blue");
    BlueLine->parameters.name = "Vertical-Blue-Right-Line";
    BlueLine->parameters.orientation = 2;
    BlueLine->parameters.lenght = canvasCalib->height();
    BlueLine->parameters.movible = false;



}
*/

/*
void MainWindow::funcUpdateImgView(QImage *tmpImg){
    //Applies rotation
    //..

    //Applies threshold
    //..
    if( ui->slide2AxCalThre->value()>0 ){
        funcImgThreshold( ui->slide2AxCalThre->value(), tmpImg );
    }
}
*/

void MainWindow::on_pbSpecLoadSnap_clicked()
{
    //Select image
    //..
    auxQstring = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select image..."),
                                                        "./snapshots/Calib/",
                                                        "(*.ppm);;"
                                                     );
    if( auxQstring.isEmpty() ){
        return (void)NULL;
    }

    //Rotate if requires
    //..
    if( funcShowMsgYesNo("Alert","Rotate using saved rotation?") == 1 ){
        float rotAngle = readAllFile( "./settings/calib/rotation.hypcam" ).trimmed().toFloat(0);
        QImage imgRot = funcRotateImage(auxQstring, rotAngle);
        imgRot.save(auxQstring);
    }


    //Create canvas
    //Display accum
    //..

    //Show image
    QPixmap pix(auxQstring);
    pix.save("./snapshots/tmpUSB.png");
    pix = pix.scaledToHeight( _GRAPH_HEIGHT );
    QGraphicsScene *scene = new QGraphicsScene(0,0,pix.width(),pix.height());
    canvasSpec->setBackgroundBrush(pix);
    canvasSpec->setScene( scene );
    canvasSpec->resize(pix.width(),pix.height());
    canvasSpec->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    canvasSpec->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->tab_5->layout()->addWidget(canvasSpec);
    ui->tab_5->layout()->setEnabled(false);
    ui->tab_5->layout()->setAlignment(Qt::AlignLeft);
    ui->gridColors->setAlignment(Qt::AlignLeft);
    ui->gridColors->setAlignment(Qt::AlignLeft);


    ui->slideRedLen->setMaximumWidth(pix.width());
    ui->slideGreenLen->setMaximumWidth(pix.width());
    ui->slideBlueLen->setMaximumWidth(pix.width());

    pix.save("./snapshots/tmpThumb.png");


}

void MainWindow::on_actionRect_triggered()
{
    /*
    selColor *selCol = new selColor(this);
    connect(selCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addRect2Calib(QString)));
    selCol->setModal(true);
    selCol->exec();
    disconnect(selCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addRect2Calib(QString)));
    */

    clearFreeHandPoligon();

    //Change mouse's cursor
    addRect2Calib("#F00");
    QApplication::setOverrideCursor(Qt::CrossCursor);
    ResetGraphToolBar("Rectangle");
    //Connect to calib double axis
    connect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcBeginRect(QMouseEvent*) )
           );
    connect(
                canvasCalib,
                SIGNAL( signalMouseReleased(QMouseEvent*) ),
                this,
                SLOT( funcCalibMouseRelease(QMouseEvent*) )
           );


}

void MainWindow::ResetGraphToolBar( QString toEnable ){
    //Disable all
    //..



    if( toEnable=="Rectangle" ){

    }
}

void MainWindow::on_actionCircle_triggered()
{
    selColor *selCol = new selColor(this);
    connect(selCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addCircle2Calib(QString)));
    selCol->setModal(true);
    selCol->exec();
    disconnect(selCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addCircle2Calib(QString)));
}

void MainWindow::on_actionHorizontalLine_triggered()
{
    selColor *selHCol = new selColor(this);
    connect(selHCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addHorLine2Calib(QString)));
    selHCol->setModal(true);
    selHCol->exec();
    disconnect(selHCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addHorLine2Calib(QString)));
}

void MainWindow::on_actionVerticalLine_triggered()
{
    selColor *selVCol = new selColor(this);
    connect(selVCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addVertLine2Calib(QString)));
    selVCol->setModal(true);
    selVCol->exec();
    disconnect(selVCol, SIGNAL(signalColorSelected(QString)), this, SLOT(addVertLine2Calib(QString)));
}

void MainWindow::addRect2Calib(QString colSeld){
    qDebug() << "Rect: " << colSeld;

}

void MainWindow::addCircle2Calib(QString colSeld){
    qDebug() << "Circle: " << colSeld;

}

void MainWindow::addVertLine2Calib(QString colSeld){
    int x = round( canvasCalib->width() / 2 );
    QPoint p1(x,0);
    QPoint p2(x, canvasCalib->height());
    customLine *tmpVLine = new customLine(p1, p2, QPen(QColor(colSeld)));
    globalCanvVLine = tmpVLine;
    //globalCanvVLine->setRotation(-90.0);
    //globalCanvVLine->moveBy(60,this->height());
    canvasCalib->scene()->addItem( globalCanvVLine );
    canvasCalib->update();
}

void MainWindow::addHorLine2Calib(QString colSeld){
    int y = round( canvasCalib->height() / 2 );
    QPoint p1(0,y);
    QPoint p2( canvasCalib->width(), y);
    customLine *tmpHLine = new customLine(p1, p2, QPen(QColor(colSeld)));
    globalCanvHLine = tmpHLine;
    canvasCalib->scene()->addItem( globalCanvHLine );
    canvasCalib->update();
}







void MainWindow::on_actionClear_triggered()
{
    //Clear scene
    canvasCalib->scene()->clear();

    clearFreeHandPoligon();

    clearRectangle();

    //Mouse
    mouseCursorReset();

}

void MainWindow::clearFreeHandPoligon(){
    //Clear Free-hand points
    lstBorder->clear();
    lstSelPix->clear();
    lstPixSelAux->clear();

    //Disconnect signals
    disconnect(
               canvasAux,
               SIGNAL( signalMousePressed(QMouseEvent*) ),
               this,
               SLOT( funcAddPoint(QMouseEvent*) )
          );
    canvasAux->update();
}

void MainWindow::clearRectangle(){
    disconnect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcBeginRect(QMouseEvent*) )
           );
    disconnect(
                canvasCalib,
                SIGNAL( signalMouseReleased(QMouseEvent*) ),
                this,
                SLOT( funcCalibMouseRelease(QMouseEvent*) )
           );
    canvasCalib->update();
}

void MainWindow::on_actionSelection_triggered()
{
    //Change cursor
    QApplication::restoreOverrideCursor();

    //Disconnect
    //..
    //Rectangle
    disconnect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcBeginRect(QMouseEvent*) )
           );
    disconnect(
                canvasCalib,
                SIGNAL( signalMouseReleased(QMouseEvent*) ),
                this,
                SLOT( funcCalibMouseRelease(QMouseEvent*) )
           );

}

void MainWindow::on_actionDrawToolbar_triggered()
{
    disableAllToolBars();
    ui->toolBarDraw->setVisible(true);
}

void MainWindow::on_pbExpPixs_tabBarClicked(int index)
{
    disableAllToolBars();
    switch(index){
        case 3:
            ui->toolBarDraw->setVisible(true);
            break;
    }
}


/*
void MainWindow::on_slide2AxCalThre_sliderReleased()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //int value = ui->slide2AxCalThre->value();
    //Rotate image if requested
    //..
    if(globaIsRotated){
        float rotAngle = readAllFile( "./settings/calib/rotation.hypcam" ).trimmed().toFloat(0);
        QImage imgRot = funcRotateImage(auxQstring, rotAngle);
        imgRot.save(_PATH_DISPLAY_IMAGE);
    }
    //Apply threshold to the image
    //..
    QImage *imgThre = new QImage(auxQstring);
    funcImgThreshold( value, imgThre );
    QString tmpFilePaht = _PATH_DISPLAY_IMAGE;
    if( imgThre->save(tmpFilePaht) ){
        QtDelay(100);
        QPixmap pix(tmpFilePaht);
        pix = pix.scaledToHeight(_GRAPH_CALIB_HEIGHT);
        canvasCalib->setBackgroundBrush(pix);
       // ui->slide2AxCalThre->setValue(value);
        //ui->slide2AxCalThre->setToolTip(QString::number(value));
        //ui->slide2AxCalThre->update();
        //QtDelay(20);
    }
    QApplication::restoreOverrideCursor();
}
*/

void MainWindow::on_actionDoubAxisDiff_triggered()
{
    genCalibXML *genCalib = new genCalibXML(this);
    genCalib->setModal(true);
    genCalib->show();

}


void MainWindow::on_slideTriggerTime_valueChanged(int value)
{
    ui->labelTriggerTime->setText( "Trigger at: " + QString::number(value) + "s" );
}

void MainWindow::on_actionRotateImg_triggered()
{
    DrawVerAndHorLines( canvasCalib, Qt::magenta );
    rotationFrm *doRot = new rotationFrm(this);
    doRot->setModal(false);
    connect(doRot,SIGNAL(angleChanged(float)),this,SLOT(doImgRotation(float)));
    doRot->show();
    doRot->move(QPoint(this->width(),0));
    doRot->raise();
    doRot->update();
}

void MainWindow::doImgRotation( float angle ){
    QTransform transformation;
    transformation.rotate(angle);
    QImage tmpImg(auxQstring);
    tmpImg = tmpImg.transformed(transformation);
    tmpImg.save(_PATH_DISPLAY_IMAGE);
    reloadImage2Display();
    DrawVerAndHorLines( canvasCalib, Qt::magenta );
}

void MainWindow::DrawVerAndHorLines(GraphicsView *tmpCanvas, Qt::GlobalColor color){
    QPoint p1(0,(tmpCanvas->height()/2));
    QPoint p2(tmpCanvas->width(),(tmpCanvas->height()/2));
    customLine *hLine = new customLine(p1,p2,QPen(color));
    tmpCanvas->scene()->addItem(hLine);
    p1.setX((tmpCanvas->width()/2));
    p1.setY(0);
    p2.setX((tmpCanvas->width()/2));
    p2.setY(tmpCanvas->height());
    customLine *vLine = new customLine(p1,p2,QPen(color));
    tmpCanvas->scene()->addItem(vLine);
    tmpCanvas->update();
}

void MainWindow::reloadImage2Display(){
    //Load image to display
    QPixmap pix(_PATH_DISPLAY_IMAGE);
    pix = pix.scaledToHeight(_GRAPH_CALIB_HEIGHT);    
    //It creates the scene to be loaded into Layout
    QGraphicsScene *sceneCalib = new QGraphicsScene(0,0,pix.width(),pix.height());    
    canvasCalib->setBackgroundBrush(QBrush(Qt::black));
    canvasCalib->setBackgroundBrush(pix);    
    canvasCalib->setScene( sceneCalib );
    canvasCalib->resize(pix.width(),pix.height());
    canvasCalib->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    canvasCalib->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    canvasCalib->update();
    //qDebug() << "CanvasSceneW: "<< canvasCalib->scene()->width();
    //qDebug() << "CanvasSceneH: "<< canvasCalib->scene()->height();
    //qDebug() << "CanvasW: "<< canvasCalib->width();
    //qDebug() << "CanvasH: "<< canvasCalib->height();
}

void MainWindow::on_actionLoadCanvas_triggered()
{
    //Select image
    //..
    auxQstring = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select image..."),
                                                        "./snapshots/Calib/",
                                                        "(*.ppm *.RGB888 *.tif *.png *.jpg, *.jpeg);;"
                                                     );
    if( auxQstring.isEmpty() ){
        return (void)NULL;
    }

    loadImageIntoCanvasEdit(auxQstring, true);


}

void MainWindow::loadImageIntoCanvasEdit(QString fileName, bool ask){
    //Create a copy of the image selected
    //..
    QImage origImg(fileName);
    origImg.save(_PATH_DISPLAY_IMAGE);

    //Rotate if requires
    //..
    if(ask)
    {
        if( funcShowMsgYesNo("Alert","Rotate using saved rotation?") == 1 )
        {
            float rotAngle = getLastAngle();
            doImgRotation( rotAngle );
            globaIsRotated = true;
        }
        else
        {
            globaIsRotated = false;
        }
    }

    //Refresh image in scene
    //..
    //Show image
    reloadImage2Display();
    //Load layout
    QLayout *layout = new QVBoxLayout();
    layout->addWidget(canvasCalib);
    layout->setEnabled(false);
    ui->tab_6->setLayout(layout);

    //It enables slides
    //..
    ui->toolBarDraw->setEnabled(true);
    ui->toolBarDraw->setVisible(true);
    //ui->slide2AxCalThre->setEnabled(true);


    reloadImage2Display();
}

void MainWindow::on_actionApplyThreshold_triggered()
{
    recParamFrm *recParam = new recParamFrm(this);
    recParam->setModal(false);
    connect(recParam,SIGNAL(paramGenerated(QString)),this,SLOT(applyThreshol2Scene(QString)));
    recParam->setWindowTitle("Type the threshold [0-255]");
    recParam->show();
    recParam->raise();
    recParam->update();
}

void MainWindow::applyThreshol2Scene(QString threshold){
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int value = threshold.toInt(0);
    //Apply threshold to the image
    //..
    QImage *imgThre = new QImage(auxQstring);
    funcImgThreshold( value, imgThre );
    imgThre->save(_PATH_DISPLAY_IMAGE);
    //Rotate image if requested
    //..
    if(globaIsRotated){
        float rotAngle = getLastAngle();
        QImage imgRot = funcRotateImage(_PATH_DISPLAY_IMAGE, rotAngle);
        imgRot.save(_PATH_DISPLAY_IMAGE);
        qDebug() << "Rotate: " << rotAngle;
    }
    //Update canvas
    //..
    reloadImage2Display();

    /*
    if( imgThre->save(_PATH_DISPLAY_IMAGE) ){
        QtDelay(100);
        QPixmap pix(_PATH_DISPLAY_IMAGE);
        pix = pix.scaledToHeight(_GRAPH_CALIB_HEIGHT);
        canvasCalib->setBackgroundBrush(pix);
        ui->slide2AxCalThre->setValue(value);
        ui->slide2AxCalThre->setToolTip(QString::number(value));
        ui->slide2AxCalThre->update();
        //QtDelay(20);
    }
    */
    QApplication::restoreOverrideCursor();
}

float MainWindow::getLastAngle(){
    return readAllFile( _PATH_LAST_ROTATION ).trimmed().toFloat(0);
}


void MainWindow::mouseCursorWait(){
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void MainWindow::mouseCursorReset(){
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionLoadSquareRectangle_triggered()
{
    //Obtain squiare aperture params
    squareAperture *tmpSqAperture = (squareAperture*)malloc(sizeof(squareAperture));
    if( !funGetSquareXML( _PATH_SQUARE_APERTURE, tmpSqAperture ) ){
        funcShowMsg("ERROR","Loading _PATH_SQUARE_APERTURE");
        return (void)false;
    }

    //Draw a rectangle of the square aperture
    QPoint p1(tmpSqAperture->rectX,tmpSqAperture->rectY);
    QPoint p2(tmpSqAperture->rectW,tmpSqAperture->rectH);
    customRect *tmpRect = new customRect(p1,p2);
    tmpRect->setPen(QPen(Qt::red));
    tmpRect->parameters.W = canvasCalib->width();
    tmpRect->parameters.H = canvasCalib->height();
    canvasCalib->scene()->clear();
    canvasCalib->scene()->addItem(tmpRect);
    canvasCalib->update();

}

void MainWindow::on_actionLoadRegOfInteres_triggered()
{
    //Obtain squiare aperture params
    squareAperture *tmpSqAperture = (squareAperture*)malloc(sizeof(squareAperture));
    if( !funGetSquareXML( _PATH_REGION_OF_INTERES, tmpSqAperture ) ){
        funcShowMsg("ERROR","Loading _PATH_REGION_OF_INTERES");
        return (void)false;
    }

    //Draw a rectangle of the square aperture
    QPoint p1(tmpSqAperture->rectX,tmpSqAperture->rectY);
    QPoint p2(tmpSqAperture->rectW,tmpSqAperture->rectH);
    customRect *tmpRect = new customRect(p1,p2);
    tmpRect->setPen(QPen(Qt::red));
    tmpRect->parameters.W = canvasCalib->width();
    tmpRect->parameters.H = canvasCalib->height();
    canvasCalib->scene()->clear();
    canvasCalib->scene()->addItem(tmpRect);
    canvasCalib->update();
}

void MainWindow::on_slideShuterSpeedSmall_valueChanged(int value)
{
    ui->labelShuterSpeed->setText( "Shuter Speed: " + QString::number(value + ui->slideShuterSpeed->value()) );
}

void MainWindow::on_actionToolPenArea_triggered()
{
    clearRectangle();
    mouseCursorCross();
    canvasAux = canvasCalib;
    connect(
                canvasCalib,
                SIGNAL( signalMousePressed(QMouseEvent*) ),
                this,
                SLOT( funcAddPoint(QMouseEvent*) )
           );
}

void MainWindow::mouseCursorHand(){
    QApplication::setOverrideCursor(Qt::PointingHandCursor);
}

void MainWindow::mouseCursorCross(){
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void MainWindow::on_actionSend_to_XYZ_triggered()
{
    //Validate that exist pixel selected
    //..
    if( lstSelPix->count()<1){
        funcShowMsg("Lack","Not pixels selected");
        return (void)false;
    }

    //Create xycolor space
    //..
    GraphicsView *xySpace = new GraphicsView(this);
    funcPutImageIntoGV(xySpace, "./imgResources/CIEManual.png");
    xySpace->setWindowTitle( "XY color space" );
    xySpace->show();

    //Transform each pixel from RGB to xy and plot it
    //..
    QImage tmpImg(_PATH_DISPLAY_IMAGE);
    int W = tmpImg.width();
    int H = tmpImg.height();
    int pixX, pixY;
    QRgb tmpPix;
    colSpaceXYZ *tmpXYZ = (colSpaceXYZ*)malloc(sizeof(colSpaceXYZ));
    int tmpOffsetX = -13;
    int tmpOffsetY = -55;
    int tmpX, tmpY;
    int i;
    qDebug() << "<lstSelPix->count(): " << lstSelPix->count();
    for( i=0; i<lstSelPix->count(); i++ ){
        //Get pixel position in real image
        pixX = (float)(lstSelPix->at(i).first * W) / (float)canvasAux->width();
        pixY = (float)(lstSelPix->at(i).second * H) / (float)canvasAux->height();
        tmpPix = tmpImg.pixel( pixX, pixY );
        funcRGB2XYZ( tmpXYZ, (float)qRed(tmpPix), (float)qGreen(tmpPix), (float)qBlue(tmpPix) );
        //funcRGB2XYZ( tmpXYZ, 255.0, 0, 0  );
        tmpX = floor( (tmpXYZ->x * 441.0) / 0.75 ) + tmpOffsetX;
        tmpY = 522 - floor( (tmpXYZ->y * 481.0) / 0.85 ) + tmpOffsetY;
        funcAddPoit2Graph( xySpace, tmpX, tmpY, 1, 1,
                           QColor(qRed(tmpPix),qGreen(tmpPix),qBlue(tmpPix)),
                           QColor(qRed(tmpPix),qGreen(tmpPix),qBlue(tmpPix))
                         );
    }

    //Save image plotted
    //..
    QPixmap pixMap = QPixmap::grabWidget(xySpace);
    pixMap.save("./Results/Miscelaneas/RGBPloted.png");

}

void MainWindow::on_actionSaveCanvas_triggered()
{
    recParamFrm *recParam = new recParamFrm(this);
    recParam->setModal(false);
    connect(recParam,SIGNAL(paramGenerated(QString)),this,SLOT(saveCalib(QString)));
    recParam->setWindowTitle("Type the filename...");
    recParam->show();
    recParam->raise();
    recParam->update();
}

void MainWindow::saveCalib(QString fileName){
    bool result = saveCanvas(canvasCalib,fileName);
    if( result ){
        funcShowMsg("Success","Canvas saved");
    }else{
        funcShowMsg("ERROR","Saving Canvas");
    }
}

bool MainWindow::saveCanvas(GraphicsView *tmpCanvas, QString fileName){
    //Save
    //..
    //Remove if exists
    QFile prevImg(fileName);
    if(prevImg.exists()){
        prevImg.remove();
    }
    prevImg.close();
    QPixmap pixMap = QPixmap::grabWidget(tmpCanvas);
    if(pixMap.save(fileName)){
        return true;
    }else{
        return false;
    }
    return true;
}

void MainWindow::on_actionExportPixelsSelected_triggered()
{
    if( lstSelPix->isEmpty() ){
        funcShowMsg("LACK","Not pixels to export");
    }else{
        int i;
        QString filePath = "./Results/lstPixels.txt";
        QFile fileLstPixels(filePath);
        if (!fileLstPixels.open(QIODevice::WriteOnly | QIODevice::Text)){
            funcShowMsg("ERROR","Creating file fileLstPixels");
        }else{
            QTextStream streamLstPixels(&fileLstPixels);
            for( i=0; i<lstSelPix->count(); i++ ){
                streamLstPixels << QString::number(lstSelPix->at(i).first) << " "<< QString::number(lstSelPix->at(i).second) << "\n";
            }
        }
        fileLstPixels.close();
        funcShowMsg("Success","List of pixels exported into: "+filePath);
    }
}

void MainWindow::on_pbLANConnect_clicked()
{
    ui->txtCommand->setText("sudo iwconfig wlan0 essid ESSID-NAME key s:PASS");
    ui->checkBlind->setChecked(true);
    funcShowMsg("Alert","Execute IWCONFIG setting BLIND mode");
}

void MainWindow::on_pbLANTestConn_clicked()
{
    ui->txtCommand->setText("iwconfig");
    ui->checkBlind->setChecked(false);
    ui->pbSendComm->click();
}

void MainWindow::on_actionGenHypercube_triggered()
{

    QString fileName;
    fileName = QFileDialog::getSaveFileName(
                                                this,
                                                tr("Save Hypercube as..."),
                                                "./Hypercubes/",
                                                tr("Documents (*.hypercube)")
                                            );
    if( fileName.isEmpty() )
    {
        return (void)NULL;
    }


    QTime timeStamp;
    timeStamp.start();


    if( generatesHypcube(15, fileName) )
    {
        //Extracts hypercube
        extractsHyperCube(fileName);
        //Show time to extract files
        QString time;
        time = timeToQString( timeStamp.elapsed() );
        qDebug() << time;
        //Inform to the user
        funcShowMsg(" ", "Hypercube exported successfully");
    }
    //exit(2);

}

bool MainWindow::generatesHypcube(int numIterations, QString fileName){



    mouseCursorWait();

    int i, l, aux, N;
    double *F, *fRed, *fGreen, *fBlue;
    QList<double> lstChoises;
    int hypW, hypH, hypL;
    lstDoubleAxisCalibration daCalib;    
    lstChoises  = getWavesChoised();
    funcGetCalibration(&daCalib);

    hypW        = daCalib.squareUsableW;
    hypH        = daCalib.squareUsableH;
    hypL        = lstChoises.count();
    N           = hypW * hypH * hypL;//Voxels

    F           = (double*)malloc(N*sizeof(double));
    fRed        = calculatesF(numIterations,_RED,&daCalib);
    fGreen      = calculatesF(numIterations,_GREEN,&daCalib);
    fBlue       = calculatesF(numIterations,_BLUE,&daCalib);

    //Demosaicing hypercube
    //..
    //Get hash to the corresponding sensitivity
    QList<double> Sr;
    QList<double> Sg;
    QList<double> Sb;
    for(l=0; l<hypL; l++)
    {        
        aux = ((floor(lstChoises.at(l)) - floor(daCalib.minWavelength) )==0)?0:floor( (floor(lstChoises.at(l)) - floor(daCalib.minWavelength) ) / (double)daCalib.minSpecRes );
        Sr.append( daCalib.Sr.at(aux)  );
        Sg.append( daCalib.Sg.at(aux) );
        Sb.append( daCalib.Sb.at(aux) );
    }
    int j, pixByImage;
    double min, max;
    int minPos, maxPos;
    min = 9999;
    max = -1;
    pixByImage = daCalib.squareUsableW * daCalib.squareUsableH;    
    i=0;
    for(l=0; l<hypL;l++)
    {
        for(j=0; j<pixByImage; j++)
        {
            F[i]    = (fRed[i]+fGreen[i]+fBlue[i]) / (Sr.at(l)+Sg.at(l)+Sb.at(l));
            if(min>F[i])
            {
                min     = F[i];
                minPos  = i;
            }
            if(max<F[i])
            {
                max     = F[i];
                maxPos  = i;
            }
            i++;
        }
    }
    printf("min(%lf,%d) max(%lf,%d)\n",min,minPos,max,maxPos);
    fflush(stdout);

    //Save hypercube
    //..
    //Format: Date,W,H,L,l1,...,lL,pix_1_l1,pix_2_l1,...pix_n_l1,pix_1_l2,pix_2_l2,...pix_n_l2,...,pix_1_L,pix_2_L,...pix_n_L
    QString hypercube;
    QDateTime dateTime = QDateTime::currentDateTime();
    hypercube.append(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
    hypercube.append(","+QString::number(daCalib.squareUsableW));
    hypercube.append(","+QString::number(daCalib.squareUsableH));
    hypercube.append(","+QString::number(lstChoises.count()));
    for(l=0; l<lstChoises.count(); l++)
    {
        hypercube.append(","+QString::number(lstChoises.at(l)));
    }
    N = hypW * hypH * hypL;//Voxels
    for(i=0; i<N; i++)
    {
        hypercube.append(","+QString::number(F[i]));
    }
    fileName.replace(".hypercube","");
    saveFile(fileName+".hypercube",hypercube);






    mouseCursorReset();





    /*

    if(false)
    {
        double min, max;
        int minPos, maxPos;
        min = 9999;
        max = -1;

        for(int n=0; n<N; n++)
        {
            if(min > F[n])
            {
                min = F[n];
                minPos = n;
            }
            if(max < F[n])
            {
                max = F[n];
                maxPos = n;
            }
           //    printf("F[%d] | %lf\n",n,F[n]);
        }
        printf("min(%lf,%d) max(%lf,%d)\n",min,minPos,max,maxPos);
        fflush(stdout);
    }
    */







    return true;

}

double *MainWindow::calculatesF(int numIterations, int sensor, lstDoubleAxisCalibration *daCalib)
{
    //Get original image
    //..
    int i, N, M;
    QImage img( _PATH_DISPLAY_IMAGE );
    M = img.width() * img.height();

    //Creates and fills H
    // ..
    //Creates containers
    int hypW, hypH, hypL;
    QList<double> lstChoises;
    pixel **Hcol;
    int **Hrow;

    lstChoises  = getWavesChoised();
    hypW        = daCalib->squareUsableW;
    hypH        = daCalib->squareUsableH;
    hypL        = lstChoises.count();
    N           = hypW * hypH * hypL;//Voxels

    //Reserves Memory for H
    //..
    Hcol        = (pixel**)malloc(N*sizeof(pixel*));
    for(int n=0; n<N; n++)
    {
        Hcol[n] = (pixel*)malloc(5*sizeof(pixel));
    }

    Hrow        = (int**)malloc(M*sizeof(int*));
    for(int m=0; m<M; m++)
    {
        Hrow[m]     = (int*)malloc(sizeof(int));
        Hrow[m][0]  = 0;
    }

    //It creates H
    //..
    createsHColAndHrow( Hcol, Hrow, &img, daCalib );

    //It creates image to proccess
    //..
    double *g, *gTmp, *f, *fKPlusOne;
    gTmp        = (double*)malloc(M*sizeof(double));
    fKPlusOne   = (double*)malloc(N*sizeof(double));
    g           = serializeImageToProccess( img, sensor );//g
    f           = createsF0(Hcol, g, N);//f0
    for( i=0; i<numIterations; i++ )
    {
        createsGTmp( gTmp, g, Hrow, f, M );//(Hf)m
        improveF( fKPlusOne, Hcol, f, gTmp, N );
        memcpy(f,fKPlusOne,(N*sizeof(double)));
    }

    //It finishes
    return f;
}




void MainWindow::improveF( double *fKPlusOne, pixel **Hcol, double *f, double *gTmp, int N )
{
    int n;
    double avgMeasure;//average measure
    double relevance;//How relevant it is respect to all voxels overlaped
    double numProj;//It is integer but is used double to evit many cast operations
    numProj = 5.0;
    for( n=0; n<N; n++ )
    {
        //fKPlusOne[n]    = 0.0;
        avgMeasure      = f[n] / numProj;
        //qDebug() << "n: " << n << " | avgMeasure: " << avgMeasure;

        relevance       = gTmp[Hcol[n][0].index] +
                          gTmp[Hcol[n][1].index] +
                          gTmp[Hcol[n][2].index] +
                          gTmp[Hcol[n][3].index] +
                          gTmp[Hcol[n][4].index];
        //qDebug() << "relevance: " << relevance;

        fKPlusOne[n]    = avgMeasure * relevance;
        //qDebug() << "fKPlusOne[" << n << "]: " << fKPlusOne[n];

    }

}

void MainWindow::createsGTmp(double *gTmp, double *g, int **Hrow, double *f, int M)
{
    int m, n;
    for( m=0; m<M; m++ )
    {
        gTmp[m] = 0.0;
        if( Hrow[m][0] > 0 )
        {
            for( n=1; n<=Hrow[m][0]; n++ )
            {
                gTmp[m] += f[Hrow[m][n]];
            }
            gTmp[m] = ( g[m] > 0 && gTmp[m] > 0 )?(g[m]/gTmp[m]):0;
        }
    }
}


double *MainWindow::createsF0(pixel **Hcol, double *g, int N)
{
    double *f;
    f = (double*)malloc(N*sizeof(double));
    for( int n=0; n<N; n++ )
    {
        f[n] = g[Hcol[n][0].index] +//Zero
               g[Hcol[n][1].index] +//Right
               g[Hcol[n][2].index] +//Up
               g[Hcol[n][3].index] +//Left
               g[Hcol[n][4].index]; //Down
        //qDebug() << "f[" << n << "]: " << f[n];
    }
    return f;
}

double *MainWindow::serializeImageToProccess(QImage img, int sensor)
{
    int M, m;
    double *g;
    M = img.width() * img.height();
    g = (double*)malloc( M * sizeof(double) );

    QRgb rgb;
    m=0;
    for( int r=0; r<img.height(); r++ )
    {
        for( int c=0; c<img.width(); c++ )
        {
            if( sensor == _RED )
            {
                rgb     = img.pixel(QPoint(c,r));
                g[m]    = (double)qRed(rgb);
            }
            else
            {
                if( sensor == _RGB )
                {
                    rgb     = img.pixel(QPoint(c,r));
                    g[m]    = (double)(qRed(rgb) + qGreen(rgb) + qBlue(rgb));
                }
                else
                {
                    if( sensor == _GREEN )
                    {
                        rgb     = img.pixel(QPoint(c,r));
                        g[m]    = (double)qGreen(rgb);
                    }
                    else
                    {   //_BLUE
                        rgb     = img.pixel(QPoint(c,r));
                        g[m]    = (double)qBlue(rgb);
                    }
                }
            }
            m++;
        }
    }
    return g;
}

void MainWindow::createsHColAndHrow(pixel **Hcol, int **Hrow, QImage *img, lstDoubleAxisCalibration *daCalib )
{
    //Prepares variables and constants
    //..
    int hypW, hypH, hypL, idVoxel;
    QList<double> lstChoises;
    strDiffProj Pj;
    lstChoises  = getWavesChoised();
    hypW        = daCalib->squareUsableW;
    hypH        = daCalib->squareUsableH;
    hypL        = lstChoises.count();

    //Fill Hcol
    //..
    idVoxel = 0;
    for(int len=1; len<=hypL; len++)
    {
        Pj.wavelength = lstChoises.at(len-1);
        for(int row=1; row<=hypH; row++)
        {
            for(int col=1; col<=hypW; col++)
            {
                //Obtain diffraction projection for the acutual wavelength
                Pj.x = col;
                Pj.y = row;
                calcDiffProj(&Pj,daCalib);
                //Creates a new item in the c-th Hcol
                Hcol[idVoxel][0].x      = Pj.x;//Zero
                Hcol[idVoxel][0].y      = Pj.y;
                Hcol[idVoxel][0].index  = xyToIndex( Hcol[idVoxel][0].x, Hcol[idVoxel][0].y, img->width() );

                Hcol[idVoxel][1].x      = Pj.rx;//Right
                Hcol[idVoxel][1].y      = Pj.ry;
                Hcol[idVoxel][1].index  = xyToIndex( Hcol[idVoxel][1].x, Hcol[idVoxel][1].y, img->width() );

                Hcol[idVoxel][2].x      = Pj.ux;//Up
                Hcol[idVoxel][2].y      = Pj.uy;
                Hcol[idVoxel][2].index  = xyToIndex( Hcol[idVoxel][2].x, Hcol[idVoxel][2].y, img->width() );

                Hcol[idVoxel][3].x      = Pj.lx;//Left
                Hcol[idVoxel][3].y      = Pj.ly;
                Hcol[idVoxel][3].index  = xyToIndex( Hcol[idVoxel][3].x, Hcol[idVoxel][3].y, img->width() );

                Hcol[idVoxel][4].x      = Pj.dx;//Down
                Hcol[idVoxel][4].y      = Pj.dy;
                Hcol[idVoxel][4].index  = xyToIndex( Hcol[idVoxel][4].x, Hcol[idVoxel][4].y, img->width() );

                //Creates new item in Hrow
                insertItemIntoRow(Hrow,Hcol[idVoxel][0].index,idVoxel);
                insertItemIntoRow(Hrow,Hcol[idVoxel][1].index,idVoxel);
                insertItemIntoRow(Hrow,Hcol[idVoxel][2].index,idVoxel);
                insertItemIntoRow(Hrow,Hcol[idVoxel][3].index,idVoxel);
                insertItemIntoRow(Hrow,Hcol[idVoxel][4].index,idVoxel);

                idVoxel++;
            }
        }
    }
}

void MainWindow::insertItemIntoRow(int **Hrow, int row, int col)
{
    int actualPos;
    actualPos = Hrow[row][0]+1;
    Hrow[row] = (int*)realloc(Hrow[row],((actualPos+1)*sizeof(int)));
    Hrow[row][actualPos] = col;
    Hrow[row][0]++;
}

QList<double> MainWindow::getWavesChoised()
{
    QList<double> wavelengths;
    QString waves;
    waves = readFileParam(_PATH_WAVE_CHOISES);
    QList<QString> choises;
    choises = waves.split(",");
    Q_FOREACH(const QString choise, choises)
    {
        if( !choise.isEmpty() && choise != " " && choise!="\n" )
        {
            wavelengths.append(choise.toDouble(0));
        }
    }
    return wavelengths;
}

void MainWindow::on_actionValidCal_triggered()
{
    //validateCalibration *frmValCal = new validateCalibration(this);
    //frmValCal->setModal(false);
    //frmValCal->show();
}

void MainWindow::on_actionValCal_triggered()
{
    selWathToCheck *tmpFrm = new selWathToCheck(this);
    tmpFrm->setModal(false);
    tmpFrm->show();
}

void MainWindow::on_actionSquareUsable_triggered()
{
    //Read Calibration
    lstDoubleAxisCalibration daCalib;
    funcGetCalibration(&daCalib);

    selWathToCheck *checkCalib = new selWathToCheck(this);
    checkCalib->showSqUsable(
                                daCalib.squareUsableX,
                                daCalib.squareUsableY,
                                daCalib.squareUsableW,
                                daCalib.squareUsableH,
                                Qt::magenta
                            );

}

void MainWindow::on_actionChoseWavelength_triggered()
{
    choseWaveToExtract *form = new choseWaveToExtract(this);
    form->show();
}

void MainWindow::on_actionFittFunction_triggered()
{
    //Select images
    //..
    QString originFileName;
    originFileName = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select image origin..."),
                                                        "./tmpImages/",
                                                        "(*.ppm *.RGB888 *.tif *.png *.jpg, *.jpeg *.gif);;"
                                                     );
    if( originFileName.isEmpty() )
    {
        return (void)NULL;
    }

    //Obtains dots
    //..
    int row, col, min, minPos, val, i;
    QRgb pixel;
    QImage img(originFileName);
    QImage tmpImg(originFileName);
    QString function;
    function = QString::number(img.height());
    i = 0;
    for( col=0; col<img.width(); col++ )
    {
        min     = 900;
        minPos  = 0;
        for( row=0; row<img.height(); row++ )
        {
            pixel   = img.pixel(col,row);
            val     = qRed(pixel)+qGreen(pixel)+qBlue(pixel);
            if( val < min )
            {
                min     = val;
                minPos  = row;
            }
            tmpImg.setPixelColor(QPoint(col,minPos),Qt::white);
        }
        function.append(","+QString::number(img.height()-minPos));
        tmpImg.setPixelColor(QPoint(col,minPos),Qt::magenta);
        i++;
    }
    //Save
    saveFile(_PATH_HALOGEN_FUNCTION,function);
    tmpImg.save(_PATH_AUX_IMG);

    funcShowMsg("Function saved successfully",_PATH_HALOGEN_FUNCTION);

}

void MainWindow::on_actionShow_hypercube_triggered()
{
    //Select images
    //..
    QString originFileName;
    originFileName = QFileDialog::getOpenFileName(
                                                        this,
                                                        tr("Select hypercube..."),
                                                        "./Hypercubes/",
                                                        "(*.hypercube);;"
                                                     );
    qDebug() << originFileName;
    if( originFileName.isEmpty() )
    {
        return (void)NULL;
    }

    //Generates hypercube
    //..
    extractsHyperCube(originFileName);

    funcShowMsg("Hypercube extracted successfully",_PATH_TMP_HYPCUBES);


}

void MainWindow::extractsHyperCube(QString originFileName)
{
    //Extracts information about the hypercube
    //..
    QString qstringHypercube;
    QList<QString> hypItems;
    QList<double> waves;
    qstringHypercube = readFileParam( originFileName );
    hypItems = qstringHypercube.split(",");
    QString dateTime;
    int W, H, L, l;
    dateTime = hypItems.at(0);  hypItems.removeAt(0);
    W = hypItems.at(0).toInt(0);         hypItems.removeAt(0);
    H = hypItems.at(0).toInt(0);         hypItems.removeAt(0);
    L = hypItems.at(0).toInt(0);         hypItems.removeAt(0);
    for(l=0;l<L;l++)
    {
        waves.append( hypItems.at(0).toDouble(0) );
        hypItems.removeAt(0);
    }

    //Generates the images into a tmpImage
    //..
    funcClearDirFolder(_PATH_TMP_HYPCUBES);
    QString tmpFileName;
    QList<QImage> hypercube;
    QImage tmpImg(W,H,QImage::Format_Grayscale8);
    int tmpVal;
    int col, row;
    for( l=0; l<L; l++ )
    {
        for( row=0; row<H; row++ )
        {
            for( col=0; col<W; col++ )
            {
                tmpVal = (hypItems.at(0).toDouble(0)<255.0)?(char)ceil(hypItems.at(0).toDouble(0)):(char)255;
                tmpImg.setPixelColor(QPoint(col,row),qGray(tmpVal,tmpVal,tmpVal));
                hypItems.removeAt(0);
            }
        }
        tmpFileName = _PATH_TMP_HYPCUBES + QString::number(waves.at(l)) + ".png";
        tmpImg.save(tmpFileName);
        hypercube.append(tmpImg);
    }
}
