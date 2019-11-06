#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <qdatetime.h>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName("AImageLab");
    QCoreApplication::setOrganizationDomain("www.airi.unimore.it");
    QCoreApplication::setApplicationName("QtIotBridge");
    _psettings = new QSettings(this);

    connect(ui->btnStart,SIGNAL(pressed()),this,SLOT(StartRead()));

    connect(this,SIGNAL(PacketReceived()),this,SLOT(SendToHttpURL()));
    connect(this,SIGNAL(PacketReceived()),this,SLOT(LogToFile()));

    connect(&_qnam, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);
    connect(ui->sliderFrequ ,&QAbstractSlider::valueChanged,this,&MainWindow::changeTimerVal);
    connect(&_timerC2A,&QTimer::timeout,this,&MainWindow::onTimer);


    LoadSettingsAndEnableUI();
    DetectPorts();


    // default:
    _vals[0]=14;
    _vals[1]=10;
    _vals[2]=0;

    _timerC2A.setInterval(ui->sliderFrequ->value()*1000);
    _timerC2A.setSingleShot(false);

    // inizio disattivato
    _bEnabled=false;
    _iFSMState=0;



}

void MainWindow::changeTimerVal(int val){
    if (val >0 && val < 100)
        _timerC2A.setInterval(val*1000);
    ui->sliderFrequ->setToolTip(QString::number(_timerC2A.interval()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString PortDescriptor(const QSerialPortInfo & info)
{ return info.portName()+QString(" | ") +info.description();}

void MainWindow::DetectPorts()
{
    _portList = QSerialPortInfo::availablePorts();
    ui->selPort->clear();
    for (int i=0; i<_portList.length();i++){
        ui->selPort->addItem(PortDescriptor(_portList[i]),i);
        if (PortDescriptor(_portList[i]).toLower().contains("arduino"))
            ui->selPort->setCurrentIndex(i);
    }


}


void MainWindow::Log(QString str){
    ui->txtLog->append(str);

}

void MainWindow::LogToFile(){
    if(!ui->chkEnLog->isChecked())
        return;
    QString foldername = ui->edtFolderLog->text();
    if (!QDir(foldername).exists()){
        QDir().mkdir(foldername);
    }
    QString filename=foldername;
    filename.append("/log_");
    QDateTime now = QDateTime::currentDateTime();
    QString format = "yyyyMMMdd";
    filename.append(now.toString(format));
    filename.append(".txt");
    QFile f(filename);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        f.write(now.toString("hh:mm:ss\t").toLatin1());
        f.write(QString::number(_iNval).toLatin1());
         for(int i=0; i<_iNval; i++)
        {
           f.write("\t");
           f.write(QString::number(_vals[i]).toLatin1());
        }
        f.write("\n");
    }
}

void MainWindow::StartRead(){

    if (!_bEnabled){
        //DEBUG SendToHttpURL();

        SaveSettingsAndDisableUI();

        // open the port
        _ptrPort.clear();
        int i;
        i = ui->selPort->currentData().toInt();
        //Log(QString::number(i));
        if (i<0)
            return;

        _ptrPort=QSharedPointer<QSerialPort>(new QSerialPort(_portList[i]),&QObject::deleteLater);
        _ptrPort->setBaudRate(QSerialPort::Baud9600);
        my_connect(_ptrPort,SIGNAL(readyRead()),this,SLOT(onSerialRead()));
        // connect on read

        // open and enable port
        _ptrPort->open(QSerialPort::ReadWrite);
        _bEnabled=true;
        Log("serial port opened");
        ui->btnStart->setText("Stop");
        _timerC2A.start();
    }
    else{

        LoadSettingsAndEnableUI();

        // chiudo seriale
        if (!_ptrPort.isNull())
            _ptrPort->close();
        _ptrPort.clear();
        ui->btnStart->setText("Start");
         _bEnabled=false;
        // start timers
         _timerC2A.stop();

    }
}



void MainWindow::onSerialRead(){
    //Log("Data Received");
    int i;
    // state machine
    while (!_ptrPort->atEnd()) {
            QByteArray data = _ptrPort->readAll();

            // uso un byte alla volta
            for (i=0; i<data.length(); i++){
                char ch=data[i];
                char str[20];

                sprintf(str,"%c:%d\n",ch,ch);
                //Log(str);

                if (ch==13) // terminata la riga: uso i dati ricevuti
                {   int iFutureState=_iFSMState;
                    if (_iFSMState==0){
                       if (_buffer[0]=='#')
                           iFutureState=1;
                       else
                           _buffer.clear();
                    }
                    if (_iFSMState==1){
                       int ival = _buffer.toInt();
                       // TODO check
                       _iNval=ival;
                       _iCurVal=0;
                       if (_iNval)
                          iFutureState=2;
                       else
                           iFutureState=0; // pacchetto inutile

                    }
                    if (_iFSMState==2){
                       float fval = _buffer.toFloat();
                       // TODO check
                       _vals[_iCurVal]=fval;
                       _iCurVal++;
                       if (_iCurVal>=_iNval)
                           iFutureState=3;
                    }
                    if (_iFSMState==3){
                       if (_buffer[0]=='$')
                          {
                            iFutureState=0;
                            QString strTolog;
                            strTolog=QString("Data block received from Serial: ")+QString::number(_iNval)+QString("|");
                            for(int j=0; j<_iNval; j++) strTolog+=QString::number(_vals[j])+QString("|");

                            Log(strTolog);
                            emit(PacketReceived());
                          }
                    }

                    _iFSMState=iFutureState;

                }
                if (ch==10) // butto via il buffer precedente
                    _buffer.clear();

                if (!(ch==10 || ch==13))
                { // accodo
                    _buffer.append(ch);

                }

            }

        }
}


void MainWindow::SendToHttpURL(){

    if (ui->chkEnA2C->isChecked()){
        QString str;
        str=ui->edtURLA2C->text() +QString("?")+ui->edtParamA2C->text();

        str.replace("$$V1$$",QString::number(_vals[0]));
        str.replace("$$V2$$",QString::number(_vals[1]));
        str.replace("$$V3$$",QString::number(_vals[2]));

        QUrl url;
        url.setUrl(str);
        Log(QString("URL: ")+str);

        QNetworkReply *reply;
        reply = _qnam.get(QNetworkRequest(url));
        //QByteArray ris = reply->readAll();
        //Log(reply->errorString());
    }
}


void MainWindow::onTimer(){
    if (ui->chkEnC2A->isChecked()){
        QString str;
        str=ui->edtURLC2A->text() +QString("?")+ui->edtParamC2A->text();

        QUrl url;
        url.setUrl(str);
        Log(str);

        QNetworkReply *reply;
        reply = _qnam.get(QNetworkRequest(url));
        //QByteArray ris = reply->readAll();
    }
}


void MainWindow::replyFinished(QNetworkReply *reply){
    // risposta da chiamata A2C
    if (reply->url().toString().contains(ui->edtURLA2C->text())){
        QByteArray ris = reply->readAll();
        Log(QString("answer from Arduino to Cloud: ")+ris);
    }

    // risposta da chiamata C2A
    if (reply->url().toString().contains(ui->edtURLC2A->text())){
        Log("answer from Cloud to Arduino");
        QByteArray ris = reply->readAll();
        Log(ris);
    }
}


void MainWindow::SaveSettingsAndDisableUI(){
    bool en;
    en=false;

    // save settings
    _psettings->setValue("UrlA2C",ui->edtURLA2C->text()); ui->edtURLA2C->setEnabled(en);
    _psettings->setValue("ParamA2C",ui->edtParamA2C->text()); ui->edtParamA2C->setEnabled(en);
    _psettings->setValue("UrlC2A",ui->edtURLC2A->text());ui->edtURLC2A->setEnabled(en);
    _psettings->setValue("ParamC2A",ui->edtParamC2A->text()); ui->edtParamC2A->setEnabled(en);
    _psettings->setValue("freq",ui->sliderFrequ->value()); ui->sliderFrequ->setEnabled(en);
    _psettings->setValue("EnableA2C",ui->chkEnA2C->isChecked()); ui->chkEnA2C->setEnabled(en);
    _psettings->setValue("EnableC2A",ui->chkEnC2A->isChecked()); ui->chkEnC2A->setEnabled(en);
    _psettings->setValue("EnableLog",ui->chkEnLog->isChecked()); ui->chkEnLog->setEnabled(en);
    _psettings->setValue("LogFolder",ui->edtFolderLog->text()); ui->edtFolderLog->setEnabled(en);
    ui->selPort->setEnabled(en);

}
void MainWindow::LoadSettingsAndEnableUI(){
    bool en;
    en=true;

    // save settings
    ui->edtURLA2C->setText(_psettings->value("UrlA2C","http://imagelab.ing.unimore.it/iotdemo/addval.asp").toString());         ui->edtURLA2C->setEnabled(en);
    ui->edtParamA2C->setText(_psettings->value("ParamA2C","idsensor=$$V1$$&v1=$$V2$$&v2=$$V3$$").toString());     ui->edtParamA2C->setEnabled(en);
    ui->edtURLC2A->setText(_psettings->value("UrlC2A","http://imagelab.ing.unimore.it/iotdemo/GetLastV1Val.asp").toString());         ui->edtURLC2A->setEnabled(en);
    ui->edtParamC2A->setText(_psettings->value("ParamC2A","idsensor=14").toString());     ui->edtParamC2A->setEnabled(en);
    ui->sliderFrequ->setValue(_psettings->value("freq",5).toInt());          ui->sliderFrequ->setEnabled(en);
    ui->chkEnA2C->setChecked(_psettings->value("EnableA2C",true).toBool());     ui->chkEnA2C->setEnabled(en);
    ui->chkEnC2A->setChecked(_psettings->value("EnableC2A",true).toBool());     ui->chkEnC2A->setEnabled(en);
    ui->chkEnLog->setChecked(_psettings->value("EnableLog",true).toBool());     ui->chkEnLog->setEnabled(en);
    ui->edtFolderLog->setText(_psettings->value("LogFolder","Log").toString());     ui->edtFolderLog->setEnabled(en);

    ui->selPort->setEnabled(en);

}

