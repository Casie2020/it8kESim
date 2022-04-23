#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDate>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowTitle("IT8000E Simulator");
    ui->setupUi(this);

    ui->buttonTCPServer->setText("Offline");
    ui->buttonTCPServer->setStyleSheet("background-color : rgb(255, 123, 123);");

    on_sliderWeight_sliderMoved(0);
    ui->labelTrafficLight->setText("Traffic Light Red");
    ui->labelTrafficLight->setStyleSheet("background-color : rgb(255, 123, 123);");
    ui->buttonSetLightGreen->setEnabled(false);

    ui->buttonTCPServer_2->setText("Offline");
    ui->buttonTCPServer_2->setStyleSheet("background-color : rgb(255, 123, 123);");

    on_sliderWeight_2_sliderMoved(0);
    ui->labelTrafficLight_2->setText("Traffic Light Red");
    ui->labelTrafficLight_2->setStyleSheet("background-color : rgb(255, 123, 123);");
    ui->buttonSetLightGreen_2->setEnabled(false);

    ui->buttonTCPServer_3->setText("Offline");
    ui->buttonTCPServer_3->setStyleSheet("background-color : rgb(255, 123, 123);");

    on_sliderWeight_3_sliderMoved(0);
    ui->labelTrafficLight_3->setText("Traffic Light Red");
    ui->labelTrafficLight_3->setStyleSheet("background-color : rgb(255, 123, 123);");
    ui->buttonSetLightGreen_3->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


// --- Weegbrug 1 ---

void MainWindow::on_buttonTCPServer_clicked()
{
    if (!ui->buttonTCPServer->text().compare("Offline")) {
        server_1.listen(QHostAddress::Any, 1234);
        connect(&server_1, SIGNAL(newConnection()), this, SLOT(onNewConnection_1()));
        ui->buttonTCPServer->setText("Online ");
        ui->buttonTCPServer->setStyleSheet("background-color : rgb(110, 243, 141);");
    } else {
        for (QTcpSocket* socket : sockets_1) {
            socket->disconnectFromHost();
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead_1()));
            disconnect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_1(QAbstractSocket::SocketState)));
        }
        server_1.close();
        disconnect(&server_1, SIGNAL(newConnection()), this, SLOT(onNewConnection_1()));
        ui->buttonTCPServer->setText("Offline");
        ui->buttonTCPServer->setStyleSheet("background-color : rgb(255, 123, 123);");
    }
}

void MainWindow::onNewConnection_1()
{
    QTcpSocket *clientSocket = server_1.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead_1()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_1(QAbstractSocket::SocketState)));

    sockets_1.push_back(clientSocket);
}

void MainWindow::onSocketStateChanged_1(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        sockets_1.removeOne(sender);
        disconnect(sender, SIGNAL(readyRead()), this, SLOT(onReadyRead_1()));
        disconnect(sender, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_1(QAbstractSocket::SocketState)));
    }
}

void MainWindow::onReadyRead_1()
{
    QString response = "";
    int grossweight = 0;

    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    if (QString(data) == "<RM1>\r\n") {
        // startchar
        response += "<";
        // error code
        if (ui->checkBoxOverloadErr->isChecked()) {
            response += "12";
        } else if (ui->checkBoxGeneralErr->isChecked()) {
            response += "11";
        } else {
            response += "00";
        }
        // scale status
        if (ui->checkBoxInMotion->isChecked()) {
            response += "1";  // scale in motion
        } else {
            response += "0";  // scale settled
        }
        if (ui->sliderWeight->value() >= 0) {
            response += "0";  // gross positive
            grossweight = ui->sliderWeight->value();
        } else {
            response += "1";  // gross negative
            grossweight = -ui->sliderWeight->value();
        }
        // Date
        response += QDate::currentDate().toString("yy-MM-dd");
        // Time
        response += QTime::currentTime().toString("HH:mm");
        // Ident-No
        response += "   0";
        // Scale No
        response += "1";
        // Gross weight
        QString gw = QString::number(grossweight);
        response += gw.rightJustified(8, ' ');
        // Tare weight
        response += QString("0").rightJustified(8, ' ');
        // Net weight
        response += gw.rightJustified(8, ' ');
        // Unit
        response += "kg";
        // Tare code
        response += "  ";
        // Weighing range
        response += " ";
        // Terminal no
        response += "001";
        // CRC16 checksum
        response += QString("TODO").rightJustified(8, ' ');
        // end char
        response += ">";
        // CR+LF
        response += "\r\n";
    } else if (QString(data) == "<SZ1>\r\n") {
        on_buttonSetZero_clicked();
        response = "<00>\r\n";
    } else if (QString(data) == "<OS01>\r\n") {
        on_buttonSetLightGreen_clicked();
        response = "<00>\r\n";
    }
    sender->write(response.toUtf8());
    qDebug() << "WB1:" << response;
}

void MainWindow::on_buttonSetZero_clicked()
{
    ui->sliderWeight->setValue(0);
    on_sliderWeight_sliderMoved(0);
}

void MainWindow::on_buttonSetLightGreen_clicked()
{
    if (ui->sliderWeight->value() > 100) {
        ui->labelTrafficLight->setText("Traffic Light Green");
        ui->labelTrafficLight->setStyleSheet("background-color : rgb(110, 243, 141);");
    }
}

void MainWindow::on_sliderWeight_sliderMoved(int position)
{
    static int previouspostion = 0;

    if (previouspostion > 100 && position <= 100) {
        // change traffic light to red
        ui->labelTrafficLight->setText("Traffic Light Red");
        ui->labelTrafficLight->setStyleSheet("background-color : rgb(255, 123, 123);");
        ui->buttonSetLightGreen->setEnabled(false);
    }
    if (previouspostion < 100 && position >= 100) {
        ui->buttonSetLightGreen->setEnabled(true);
    }
    ui->labelWeight->setText(QString::number(position) + " kg");
    previouspostion = position;
}

void MainWindow::on_sliderWeight_sliderPressed()
{
    ui->checkBoxInMotion->setChecked(true);
}

void MainWindow::on_sliderWeight_sliderReleased()
{
    ui->checkBoxInMotion->setChecked(false);
}

// --- Weegbrug 2 ---

void MainWindow::on_buttonTCPServer_2_clicked()
{
    if (!ui->buttonTCPServer_2->text().compare("Offline")) {
        server_2.listen(QHostAddress::Any, 1235);
        connect(&server_2, SIGNAL(newConnection()), this, SLOT(onNewConnection_2()));
        ui->buttonTCPServer_2->setText("Online ");
        ui->buttonTCPServer_2->setStyleSheet("background-color : rgb(110, 243, 141);");
    } else {
        for (QTcpSocket* socket : sockets_2) {
            socket->disconnectFromHost();
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead_2()));
            disconnect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_2(QAbstractSocket::SocketState)));
        }
        server_2.close();
        disconnect(&server_2, SIGNAL(newConnection()), this, SLOT(onNewConnection_2()));
        ui->buttonTCPServer_2->setText("Offline");
        ui->buttonTCPServer_2->setStyleSheet("background-color : rgb(255, 123, 123);");
    }
}

void MainWindow::onNewConnection_2()
{
    QTcpSocket *clientSocket = server_2.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead_2()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_2(QAbstractSocket::SocketState)));

    sockets_2.push_back(clientSocket);
}

void MainWindow::onSocketStateChanged_2(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        sockets_2.removeOne(sender);
        disconnect(sender, SIGNAL(readyRead()), this, SLOT(onReadyRead_2()));
        disconnect(sender, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_2(QAbstractSocket::SocketState)));
    }
}

void MainWindow::onReadyRead_2()
{
    QString response = "";
    int grossweight = 0;
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    if (QString(data) == "<RM1>\r\n") {
        // startchar
        response += "<";
        // error code
        if (ui->checkBoxOverloadErr_2->isChecked()) {
            response += "12";
        } else if (ui->checkBoxGeneralErr_2->isChecked()) {
            response += "11";
        } else {
            response += "00";
        }
        // scale status
        if (ui->checkBoxInMotion_2->isChecked()) {
            response += "1";  // scale in motion
        } else {
            response += "0";  // scale settled
        }
        if (ui->sliderWeight_2->value() >= 0) {
            response += "0";  // gross positive
            grossweight = ui->sliderWeight_2->value();
        } else {
            response += "1";  // gross negative
            grossweight = -ui->sliderWeight_2->value();
        }
        // Date
        response += QDate::currentDate().toString("yy-MM-dd");
        // Time
        response += QTime::currentTime().toString("HH:mm");
        // Ident-No
        response += "   0";
        // Scale No
        response += "1";
        // Gross weight
        QString gw = QString::number(grossweight);
        response += gw.rightJustified(8, ' ');
        // Tare weight
        response += QString("0").rightJustified(8, ' ');
        // Net weight
        response += gw.rightJustified(8, ' ');
        // Unit
        response += "kg";
        // Tare code
        response += "  ";
        // Weighing range
        response += " ";
        // Terminal no
        response += "001";
        // CRC16 checksum
        response += QString("TODO").rightJustified(8, ' ');
        // end char
        response += ">";
        // CR+LF
        response += "\r\n";
    } else if (QString(data) == "<SZ1>\r\n") {
        on_buttonSetZero_clicked();
        response = "<00>\r\n";
    } else if (QString(data) == "<OS01>\r\n") {
        on_buttonSetLightGreen_clicked();
        response = "<00>\r\n";
    }
    sender->write(response.toUtf8());
    qDebug() << "WB2:" << response;
}


void MainWindow::on_buttonSetZero_2_clicked()
{
    ui->sliderWeight_2->setValue(0);
    on_sliderWeight_2_sliderMoved(0);
}

void MainWindow::on_buttonSetLightGreen_2_clicked()
{
    if (ui->sliderWeight_2->value() > 100) {
        ui->labelTrafficLight_2->setText("Traffic Light Green");
        ui->labelTrafficLight_2->setStyleSheet("background-color : rgb(110, 243, 141);");
    }
}

void MainWindow::on_sliderWeight_2_sliderMoved(int position)
{
    static int previouspostion = 0;

    if (previouspostion > 100 && position <= 100) {
        // change traffic light to red
        ui->labelTrafficLight_2->setText("Traffic Light Red");
        ui->labelTrafficLight_2->setStyleSheet("background-color : rgb(255, 123, 123);");
        ui->buttonSetLightGreen_2->setEnabled(false);
    }
    if (previouspostion < 100 && position >= 100) {
        ui->buttonSetLightGreen_2->setEnabled(true);
    }
    ui->labelWeight_2->setText(QString::number(position) + " kg");
    previouspostion = position;
}

void MainWindow::on_sliderWeight_2_sliderPressed()
{
    ui->checkBoxInMotion_2->setChecked(true);
}

void MainWindow::on_sliderWeight_2_sliderReleased()
{
    ui->checkBoxInMotion_2->setChecked(false);
}

// --- Weegbrug 3 ---

void MainWindow::on_buttonTCPServer_3_clicked()
{
    if (!ui->buttonTCPServer_3->text().compare("Offline")) {
        server_3.listen(QHostAddress::Any, 1236);
        connect(&server_3, SIGNAL(newConnection()), this, SLOT(onNewConnection_3()));
        ui->buttonTCPServer_3->setText("Online ");
        ui->buttonTCPServer_3->setStyleSheet("background-color : rgb(110, 243, 141);");
    } else {
        for (QTcpSocket* socket : sockets_3) {
            socket->disconnectFromHost();
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead_3()));
            disconnect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_3(QAbstractSocket::SocketState)));
        }
        server_3.close();
        disconnect(&server_3, SIGNAL(newConnection()), this, SLOT(onNewConnection_3()));
        ui->buttonTCPServer_3->setText("Offline");
        ui->buttonTCPServer_3->setStyleSheet("background-color : rgb(255, 123, 123);");
    }
}

void MainWindow::onNewConnection_3()
{
    QTcpSocket *clientSocket = server_3.nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead_3()));
    connect(clientSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_3(QAbstractSocket::SocketState)));

    sockets_3.push_back(clientSocket);
}

void MainWindow::onSocketStateChanged_3(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        sockets_3.removeOne(sender);
        disconnect(sender, SIGNAL(readyRead()), this, SLOT(onReadyRead_3()));
        disconnect(sender, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChanged_3(QAbstractSocket::SocketState)));
    }
}

void MainWindow::onReadyRead_3()
{
    QString response = "";
    int grossweight = 0;

    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    if (QString(data) == "<RM1>\r\n") {
        // startchar
        response += "<";
        // error code
        if (ui->checkBoxOverloadErr_3->isChecked()) {
            response += "12";
        } else if (ui->checkBoxGeneralErr_3->isChecked()) {
            response += "11";
        } else {
            response += "00";
        }
        // scale status
        if (ui->checkBoxInMotion_3->isChecked()) {
            response += "1";  // scale in motion
        } else {
            response += "0";  // scale settled
        }
        if (ui->sliderWeight_3->value() >= 0) {
            response += "0";  // gross positive
            grossweight = ui->sliderWeight_3->value();
        } else {
            response += "1";  // gross negative
            grossweight = -ui->sliderWeight_3->value();
        }
        // Date
        response += QDate::currentDate().toString("yy-MM-dd");
        // Time
        response += QTime::currentTime().toString("HH:mm");
        // Ident-No
        response += "   0";
        // Scale No
        response += "1";
        // Gross weight
        QString gw = QString::number(grossweight);
        response += gw.rightJustified(8, ' ');
        // Tare weight
        response += QString("0").rightJustified(8, ' ');
        // Net weight
        response += gw.rightJustified(8, ' ');
        // Unit
        response += "kg";
        // Tare code
        response += "  ";
        // Weighing range
        response += " ";
        // Terminal no
        response += "001";
        // CRC16 checksum
        response += QString("TODO").rightJustified(8, ' ');
        // end char
        response += ">";
        // CR+LF
        response += "\r\n";
    } else if (QString(data) == "<SZ1>\r\n") {
        on_buttonSetZero_clicked();
        response = "<00>\r\n";
    } else if (QString(data) == "<OS01>\r\n") {
        on_buttonSetLightGreen_clicked();
        response = "<00>\r\n";
    }
    sender->write(response.toUtf8());
    qDebug() << "WB3:" << response;
}

void MainWindow::on_buttonSetZero_3_clicked()
{
    ui->sliderWeight_3->setValue(0);
    on_sliderWeight_3_sliderMoved(0);
}

void MainWindow::on_buttonSetLightGreen_3_clicked()
{
    if (ui->sliderWeight_3->value() > 100) {
        ui->labelTrafficLight_3->setText("Traffic Light Green");
        ui->labelTrafficLight_3->setStyleSheet("background-color : rgb(110, 243, 141);");
    }
}

void MainWindow::on_sliderWeight_3_sliderMoved(int position)
{
    static int previouspostion = 0;

    if (previouspostion > 100 && position <= 100) {
        // change traffic light to red
        ui->labelTrafficLight_3->setText("Traffic Light Red");
        ui->labelTrafficLight_3->setStyleSheet("background-color : rgb(255, 123, 123);");
        ui->buttonSetLightGreen_3->setEnabled(false);
    }
    if (previouspostion < 100 && position >= 100) {
        ui->buttonSetLightGreen_3->setEnabled(true);
    }
    ui->labelWeight_3->setText(QString::number(position) + " kg");

    previouspostion = position;
}

void MainWindow::on_sliderWeight_3_sliderPressed()
{
    ui->checkBoxInMotion_3->setChecked(true);
}

void MainWindow::on_sliderWeight_3_sliderReleased()
{
    ui->checkBoxInMotion_3->setChecked(false);
}


