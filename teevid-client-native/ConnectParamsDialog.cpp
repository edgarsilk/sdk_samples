#include "ConnectParamsDialog.h"
#include "ui_ConnectParamsDialog.h"

#include <QMessageBox>
#include <QSettings>

ConnectParamsDialog::ConnectParamsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectParamsDialog)
{
    ui->setupUi(this);

    setModal(true);
    setWindowTitle("Connection parameters");

    QSettings settings("teevid-client-native", "connect_params");
    settings.beginGroup("teevid-client-native");
    ui->lineEditHost->setText(settings.value("host", "").toString());
    ui->lineEditToken->setText(settings.value("token", "").toString());
    ui->lineEditRoom->setText(settings.value("room", "").toString());
    ui->lineEditUser->setText(settings.value("user", "").toString());
    ui->lineEditPassword->setText(settings.value("password", "").toString());
    ui->lineEditAccessPin->setText(settings.value("access_pin", "").toString());
    settings.endGroup();

    connect(ui->btnApply, SIGNAL(pressed()), this, SLOT(OnBtnApply()));
    connect(ui->btnCancel, SIGNAL(pressed()), this, SLOT(OnBtnCancel()));
}

ConnectParamsDialog::~ConnectParamsDialog()
{
    delete ui;
}

QString ConnectParamsDialog::GetHost() const
{
    return ui->lineEditHost->text();
}

QString ConnectParamsDialog::GetToken() const
{
    return ui->lineEditToken->text();
}

QString ConnectParamsDialog::GetRoom() const
{
    return ui->lineEditRoom->text();
}

QString ConnectParamsDialog::GetUser() const
{
    return ui->lineEditUser->text();
}

QString ConnectParamsDialog::GetPassword() const
{
    return ui->lineEditPassword->text();
}

int ConnectParamsDialog::GetAccessPin() const
{
    return ui->lineEditAccessPin->text().toInt();
}

void ConnectParamsDialog::OnBtnApply()
{
    QString host = ui->lineEditHost->text();
    if (host.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter server host");
        mb.exec();
        return;
    }

    QString token = ui->lineEditToken->text();
    if (token.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter validation token for the server");
        mb.exec();
        return;
    }

    QString room = ui->lineEditRoom->text();
    if (room.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter room name");
        mb.exec();
        return;
    }

    QString user = ui->lineEditUser->text();
    if (user.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter your name");
        mb.exec();
        return;
    }

    QString password = ui->lineEditPassword->text();
    if (password.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter your password");
        mb.exec();
        return;
    }

    QString accessPin = ui->lineEditAccessPin->text();
    if (accessPin.isEmpty())
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please enter your access PIN");
        mb.exec();
        return;
    }

    QSettings settings("teevid-client-native", "connect_params");
    settings.beginGroup("teevid-client-native");
    settings.setValue("host", host);
    settings.setValue("token", token);
    settings.setValue("room", room);
    settings.setValue("user", user);
    settings.setValue("password", password);
    settings.setValue("access_pin", accessPin);
    settings.endGroup();

    _paramsApplied = true;
    close();
}

void ConnectParamsDialog::OnBtnCancel()
{
    exit(0);
}

void ConnectParamsDialog::closeEvent(QCloseEvent *event)
{
    emit (_paramsApplied ? paramsApplied() : paramsCancelled());
    QDialog::closeEvent(event);
}
