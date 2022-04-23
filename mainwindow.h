#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonSetZero_clicked();
    void on_sliderWeight_sliderMoved(int position);
    void on_sliderWeight_sliderPressed();
    void on_sliderWeight_sliderReleased();

    void on_buttonSetZero_2_clicked();
    void on_sliderWeight_2_sliderMoved(int position);
    void on_sliderWeight_2_sliderPressed();
    void on_sliderWeight_2_sliderReleased();

    void on_buttonSetZero_3_clicked();
    void on_sliderWeight_3_sliderMoved(int position);
    void on_sliderWeight_3_sliderPressed();
    void on_sliderWeight_3_sliderReleased();

    void on_buttonSetLightGreen_clicked();
    void on_buttonSetLightGreen_2_clicked();
    void on_buttonSetLightGreen_3_clicked();

    void on_buttonTCPServer_clicked();
    void on_buttonTCPServer_2_clicked();
    void on_buttonTCPServer_3_clicked();

public slots:
    void onNewConnection_1();
    void onSocketStateChanged_1(QAbstractSocket::SocketState socketState);
    void onReadyRead_1();

    void onNewConnection_2();
    void onSocketStateChanged_2(QAbstractSocket::SocketState socketState);
    void onReadyRead_2();

    void onNewConnection_3();
    void onSocketStateChanged_3(QAbstractSocket::SocketState socketState);
    void onReadyRead_3();

private:
    Ui::MainWindow *ui;
    QTcpServer server_1;
    QList<QTcpSocket *> sockets_1;
    QTcpServer server_2;
    QList<QTcpSocket *> sockets_2;
    QTcpServer server_3;
    QList<QTcpSocket *> sockets_3;
};
#endif // MAINWINDOW_H
