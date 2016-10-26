#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

#include <QFileDialog>

#include <QMessageBox>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void connectTcpServer(QString IP, int PORT);
private slots:
    void on_pushButton_clicked();
    void fotaClientReceived();
    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_3_editingFinished();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString m_socketIP;
    int m_socketPort ;

    QFileDialog *dialog;
    bool m_connected_state = false;
    QMessageBox *msgbox;
     QString m_fileName= "";
};

#endif // MAINWINDOW_H
