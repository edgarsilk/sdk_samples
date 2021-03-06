#ifndef SERVERSIMULATIONDIALOG_H
#define SERVERSIMULATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ServerSimulationDialog;
}

class ServerSimulationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerSimulationDialog(QWidget *parent = 0);
    ~ServerSimulationDialog();

signals:
    void roomSubmitted(const QString& caller, const QString& invitationUrl);

protected slots:
    void onIncomingCallPressed();

private:
    Ui::ServerSimulationDialog *ui;
};

#endif // SERVERSIMULATIONDIALOG_H
