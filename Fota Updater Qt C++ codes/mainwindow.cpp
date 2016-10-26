//include headers for our applications
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
//Our mainwindow ui class for visual
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //set ui
    ui->setupUi(this);
    //set icon for app
    setWindowIcon(QIcon(":/icone.jpg"));


    m_socketPort = ui->lineEdit_3->text().toLong();

    //Create socket object for TCP/IP communication
    socket = new QTcpSocket();
    connect(socket,SIGNAL(readyRead()),SLOT(fotaClientReceived()));
    QRegExpValidator *v = new QRegExpValidator(this);
    QRegExp rx("((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})");
    v->setRegExp(rx);
    ui->lineEdit_2->setValidator(v);

    msgbox = new QMessageBox(this);

    ui->lineEdit_3->setValidator( new QIntValidator(0, 65532, this) );

    ui->comboBox->addItem("512");
    ui->comboBox->addItem("1024");
    ui->comboBox->addItem("2048");
    ui->comboBox->addItem("4096");
    ui->comboBox->addItem("8192");
    ui->comboBox->addItem("ALL");

    ui->comboBox_2->addItem("10");
    ui->comboBox_2->addItem("50");
    ui->comboBox_2->addItem("100");
    ui->comboBox_2->addItem("250");
    ui->comboBox_2->addItem("500");

    m_socketIP = ui->lineEdit_2->text();
    QString port = ui->lineEdit_3->text();
    m_socketPort = port.toInt();

    ui->textBrowser->append("------------------------README---------------------------------");
    ui->textBrowser->append("APP: PLEASE SET YOUR ESP IP ADRESS AND PORT NUMBER");
    ui->textBrowser->append("APP: AFTER YOU SET CONFIGS, JUST CONNECT AND SELECT BINARY FILE");
    ui->textBrowser->append("APP: YOU CAN CHANGE YOUR FIRMWARE OVER FOTA WITH EASILY!");
    ui->textBrowser->append("---------------------------------------------------------------");
}

//Connect to destination ESP8266 Server
void MainWindow::connectTcpServer(QString IP, int PORT) {
    socket->connectToHost(IP,PORT);
    socket->waitForConnected(30000);
    m_connected_state = (socket->state() == QTcpSocket::ConnectedState);

    if(m_connected_state) {
        ui->label_3->setStyleSheet("QLabel { background-color : green; color : black; }");
        ui->label_3->setText("CONNECTED");
        ui->textBrowser->append("CONNECTED TO ESP:" + IP + ":" + QString::number((PORT)));
    }
    else {
        ui->label_3->setStyleSheet("QLabel { background-color : red; color : blue; }");
        ui->label_3->setText("CONN FAILED ");
        ui->textBrowser->append("CONNECTION FAILED TO:" + IP + ":" + QString::number((PORT)));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    m_fileName = QFileDialog::getOpenFileName(this,
          tr("Open Firmware Binary File"), QDir::homePath(), tr("BIN Files (*.bin)"));

    ui->lineEdit->setText(m_fileName);
    ui->textBrowser->append("APP: Selected BIN:" + m_fileName);

}


void MainWindow::on_pushButton_2_clicked()
{
    ui->textBrowser->append("APP: Opening selected File for reading.");

    if(!m_connected_state) {
        msgbox->setWindowTitle("FATAL ERROR!");
        msgbox->setText("CONNECTION NOT FOUND. CHECK CONFIGS!");
        msgbox->exec();
        return;
    }

    else {
        ui->textBrowser->append("APP: Socket connection is alive");
    }

    QFile BinFile(m_fileName);

       if (BinFile.open(QIODevice::ReadOnly))
       {
           ui->textBrowser->append("APP: Firmware Binary file opened succesfuly.");
           QByteArray rawFile; int size = 0;
           QString package_size;
           QString delay;
           package_size = ui->comboBox->currentText();
           delay = ui->comboBox_2->currentText();

            ui->textBrowser->append("APP: Selected package size :"+ package_size);
            ui->textBrowser->append("APP: Selected package sending delay :"+ delay);
           if(QString::compare("ALL",package_size, Qt::CaseInsensitive)== 0 ) {
               rawFile = BinFile.readAll();
               ui->textBrowser->append("ATTENTION: WRITING FIRMWARE PACKAGE");
               socket->write(rawFile);
               size = rawFile.size();
               ui->textBrowser->append("FOTA: total size of sending firmware data:" + rawFile.size());
               socket->flush();
               socket->disconnect();
               rawFile.clear();
               size=0;
               return;
           }

           else {
               while (!BinFile.atEnd())
               {
                  rawFile = BinFile.read(package_size.toInt());
                  socket->write(rawFile);
                //  QThread::sleep(ui->comboBox_2->currentText().toInt());
                  QThread::sleep(delay.toInt()/1000);
                  ui->textBrowser->append("FOTA: SENDING DATA:" + QString::number(rawFile.size()));
                  size+=rawFile.size();
               }

               ui->textBrowser->append("FOTA: total size of sending data:" + rawFile.size());
               socket->flush();
               socket->disconnect();
               rawFile.clear();
               size=0;
               return;

           }

}
       else {
           ui->textBrowser->append("FATAL: BINARY FILE DIDN'T OPEN FOR READING");
       }

}


//Close program and destroy socket.
void MainWindow::on_pushButton_4_clicked()
{
    socket->deleteLater();
    exit(0);
}

void MainWindow::fotaClientReceived() {
    QByteArray data = socket->readAll();
    qDebug () <<data;
    ui->textBrowser->append("RECEIVED" + QString(data));
}


//Get ESP8266 IP Adresses from UI
void MainWindow::on_lineEdit_2_editingFinished()
{
    m_socketIP = ui->lineEdit_2->text();
    ui->textBrowser->append("Destination IP Adress Configured: "+ m_socketIP);
}

//Get ESP8266 port number from UI
void MainWindow::on_lineEdit_3_editingFinished()
{
    QString port = ui->lineEdit_3->text();
    m_socketPort = port.toInt();
    ui->textBrowser->append("Destination Port Number Configured: "+ port);
}

//Connect to ESP TCP server
void MainWindow::on_pushButton_3_clicked()
{
    //Check if ip adress or line edit no empty.
    if(ui->lineEdit_2->text().isEmpty() || ui->lineEdit_3->text().isEmpty())
    {
        //Show error message

        msgbox->setText("CHECK YOUR IP ADRESS AND PORT CONFIGS");
        msgbox->setWindowTitle("FATAL ERROR");
        msgbox->exec();
    }

    //if we are here, then connect to TCP server.
    else {
        connectTcpServer(m_socketIP,m_socketPort);
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->textBrowser->clearHistory();
    ui->textBrowser->clear();

}


void MainWindow::on_pushButton_6_clicked()
{
    QString link = "https://tr.linkedin.com/in/demirtasm";
    QDesktopServices::openUrl(QUrl(link));
}
