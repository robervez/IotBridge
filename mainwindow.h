#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtNetwork>
#include <QTimer>

namespace Ui {
class MainWindow;
}


template<class T> bool
my_connect(const QSharedPointer<T> &sender,
           const char *signal,
           const QObject *receiver,
           const char *method,
           Qt::ConnectionType type = Qt::AutoConnection)
{
    return QObject::connect(sender.data(), signal, receiver, method, type);
}




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void DetectPorts();
    void StartRead();
    void onSerialRead();
    void SendToHttpURL();
    void Log(QString str);
    void replyFinished(QNetworkReply *reply);
    void changeTimerVal(int val);
    void onTimer();
    void SaveSettingsAndDisableUI();
    void LoadSettingsAndEnableUI();

signals:
    void PacketReceived();

private:
    Ui::MainWindow *ui;

    QList<QSerialPortInfo> _portList;
    QSharedPointer<QSerialPort> _ptrPort;
    bool _bEnabled;
    int _iFSMState; //FSM state
    float _vals[100];
    int _iNval;
    int _iCurVal;
    QByteArray _buffer;

    QTimer _timerC2A;

    QNetworkAccessManager _qnam;
    QSettings *_psettings;

};

#endif // MAINWINDOW_H
