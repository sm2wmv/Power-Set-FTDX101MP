#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString configFile("settings.ini");

    settings = new QSettings(configFile,QSettings::IniFormat,0);

    loadSettings();

    timerPollRadio = new QTimer(this);
    connect(timerPollRadio, SIGNAL(timeout()), this, SLOT(timerPollRadioTimeout()));

    timerPollRadioRX = new QTimer(this);
    connect(timerPollRadioRX, SIGNAL(timeout()), this, SLOT(timerPollRadioRXTimeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerPollRadioTimeout() {
    serial->write("FA;");
    serial->write("PC;");
}

int MainWindow::getPowerOnBand(int band) {
    if (band == BAND_160M)
        return(ui->spinBoxPwr160m->value());
    if (band == BAND_80M)
        return(ui->spinBoxPwr80m->value());
    if (band == BAND_40M)
        return(ui->spinBoxPwr40m->value());
    if (band == BAND_20M)
        return(ui->spinBoxPwr20m->value());
    if (band == BAND_15M)
        return(ui->spinBoxPwr15m->value());
    if (band == BAND_10M)
        return(ui->spinBoxPwr10m->value());

    return(0);
}

const char* MainWindow::getBandPowerStr(int power) {
    QByteArray temp;
    temp.append("PC");
    if (power < 10) {
        temp.append("00");
    }
    else if (power < 100) {
        temp.append("0");
    }

    temp.append(QString::number(power));

    temp.append(';');
    qDebug() << temp;
    return(temp.constData());
}


void MainWindow::parseFreq(int freq) {
    //qDebug() << "Parse freq: " << freq;
    if (prevFreq != freq) {
        if ((freq > 100000) && freq < 2500000) {
            if (currentBand != BAND_160M) {
                prevBand = currentBand;
                currentBand = BAND_160M;
            }
        }
        else if ((freq > 2500000) && freq < 4000000) {
            if (currentBand != BAND_80M) {
                prevBand = currentBand;
                currentBand = BAND_80M;
            }
        }
        else if ((freq > 6500000) && freq < 8000000) {
            if (currentBand != BAND_40M) {
                prevBand = currentBand;
                currentBand = BAND_40M;
            }
        }
        else if ((freq > 13000000) && freq < 15000000) {
            if (currentBand != BAND_20M) {
                prevBand = currentBand;
                currentBand = BAND_20M;
            }
        }
        else if ((freq > 20000000) && freq < 22000000) {
            if (currentBand != BAND_15M) {
                prevBand = currentBand;
                currentBand = BAND_15M;
            }
        }
        else if ((freq > 27000000) && freq < 30000000) {
                if (currentBand != BAND_10M) {
                    prevBand = currentBand;
                    currentBand = BAND_10M;
                }
        }
        else
            currentBand = 0;
    }

    prevFreq = freq;

    if (currentBand != prevBand) {
        qDebug() << "BAND CHANGED: " << currentBand;

        int bandPower = getPowerOnBand(currentBand);
        qDebug() << "Sending PWR: " << bandPower;

        serial->write(getBandPowerStr(bandPower));

        prevBand = currentBand;
    }
}

void MainWindow::parseRXBuffer(QByteArray rxData) {
    int termPos = 0;

    while(rxData.contains(';')) {
        termPos = rxData.indexOf(';');
        //qDebug() << "Term pos: " << termPos;

        rxCmdList.append(rxData.mid(0, termPos));
        rxData.remove(0,termPos+1);
    }

    for(int i=0;i<rxCmdList.count();i++) {
        QByteArray currCmd;
        currCmd = rxCmdList.at(i);

        if (currCmd.contains("PC")) {
            ui->labelPWR->setText(QString::number(currCmd.mid(2, currCmd.length()-2).toInt()) + "W");
        }
        else if (currCmd.contains("FA")) {
            //qDebug() << "Freq command";
            if (currentFreqPos > 2)
                currentFreqPos = 0;

            //We make sure that we have the same frequency in the whole array, to make sure we don't
            //act upon something that is corrupt
            currentFreq[currentFreqPos++] = currCmd.mid(2, currCmd.length()-2).toInt();

            ui->labelFreq->setText(QString::number(currentFreq[currentFreqPos-1]) + " Hz");

            if ((currentFreq[0] == currentFreq[1]) && (currentFreq[1] == currentFreq[2]))
                parseFreq(currentFreq[0]);
        }

    }

    rxCmdList.clear();
}

void MainWindow::timerPollRadioRXTimeout() {
    rxBuffer.append(serial->readAll());

    parseRXBuffer(rxBuffer);
    rxBuffer.clear();
}

void MainWindow::on_spinBoxPwr160m_valueChanged(int arg1) {

}

void MainWindow::on_pushButtonConnect_clicked()
{
  serial = new QSerialPort();
  serial->setPortName(comPortName);

  serial->setBaudRate(QSerialPort::Baud38400);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);

  if (!serial->open(QIODevice::ReadWrite)) {
      qDebug() << "Could not open serial port";
      return;
  }
  else {
      timerPollRadio->start(1000);
      timerPollRadioRX->start(250);
  }
}

void MainWindow::loadSettings() {
  settings->beginGroup("Settings");

  ui->spinBoxPwr160m->setValue(settings->value("Power160m").toInt());
  ui->spinBoxPwr80m->setValue(settings->value("Power80m").toInt());
  ui->spinBoxPwr40m->setValue(settings->value("Power40m").toInt());
  ui->spinBoxPwr20m->setValue(settings->value("Power20m").toInt());
  ui->spinBoxPwr15m->setValue(settings->value("Power15m").toInt());
  ui->spinBoxPwr10m->setValue(settings->value("Power10m").toInt());

  settings->endGroup();
}

void MainWindow::saveSettings() {
  settings->beginGroup("Settings");

  settings->setValue("Power160m", ui->spinBoxPwr160m->value());
  settings->setValue("Power80m", ui->spinBoxPwr80m->value());
  settings->setValue("Power40m", ui->spinBoxPwr40m->value());
  settings->setValue("Power20m", ui->spinBoxPwr20m->value());
  settings->setValue("Power15m", ui->spinBoxPwr15m->value());
  settings->setValue("Power10m", ui->spinBoxPwr10m->value());

  settings->endGroup();
}

void MainWindow::quitApplication() {
    qDebug("Bye Bye");
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();

    saveSettings();

    QTimer::singleShot(2000, this, SLOT(quitApplication()));
}
