#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include <QTimer>
#include <QList>
#include <QSettings>

#define BAND_160M 1
#define BAND_80M  2
#define BAND_40M  3
#define BAND_20M  4
#define BAND_15M  5
#define BAND_10M  6

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void timerPollRadioTimeout();
    void timerPollRadioRXTimeout();
    void closeEvent(QCloseEvent *event);
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSerialPort *serial;
    QTimer *timerPollRadio;
    QTimer *timerPollRadioRX;
    QByteArray rxBuffer;
    QList<QByteArray> rxCmdList;
    int currentBand = 0;
    int prevBand = 0;
    int currentFreq[3] = {0};
    int currentFreqPos = 0;
    int prevFreq = 0;
    int currPower = 0;
    QSettings *settings;
    void parseFreq(int freq);
    void loadSettings();
    void saveSettings();
private slots:
    void on_spinBoxPwr160m_valueChanged(int arg1);
    void quitApplication();
    void on_pushButtonConnect_clicked();

private:
    Ui::MainWindow *ui;
    void parseRXBuffer(QByteArray rxData);
    int getPowerOnBand(int band);
    const char* getBandPowerStr(int power);
};
#endif // MAINWINDOW_H
