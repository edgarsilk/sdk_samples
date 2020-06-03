#include "InitialScreen.h"
#include "ui_InitialScreen.h"

#include "FriendItem.h"
#include "CallItemVideoView.h"
#include "ServerSimulationDialog.h"
#include "ConnectParamsDialog.h"

#include <QButtonGroup>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <memory>

static const std::string kToken = "token_provided_by_teevid_support";

using namespace teevid_sdk;
InitialScreen::InitialScreen(QWidget *parent) : QWidget(parent), ui(new Ui::InitialScreen)
{
    InitUI();
    //InitSDK();
}

InitialScreen::~InitialScreen()
{
    UnsubscribeFromVideo();
    delete ui;
}

void InitialScreen::InitSDK()
{
    std::string teevidServer = _connectParamsDialog->GetHost().toStdString();
    teeVidClient_ = TeeVidFactory::CreateTeeVidClient();
    teeVidClient_->Initialize(kToken, teevidServer, (ITeeVidClientObserver*)this);
}

void InitialScreen::InitUI()
{
    ui->setupUi(this);

    QString styleSheet = "QFrame#frameUser, #frameUserContainer, #frameCallContainer { border: none; } "
                         "QFrame#frameSettings, #frameInvite { background-color: #009999; } "
                         "QPushButton#btnModeFriends, #btnModeCallHistory, #btnModeSettings { background-color: transparent; color: #ffffff; border: 1px solid #dddddd; } "
                         "QPushButton#btnModeFriends:checked, #btnModeCallHistory:pressed, #btnModeSettings:pressed { background-color: #99cc99; } "
                         "QPushButton#btnEndCall { background-color: transparent; background-image: url(:/end_call.png); } "
                         "QPushButton#btnMicrophone { background-color: transparent; background-image: url(:/microphone_off.png); } "
                         "QPushButton#btnMicrophone[turn_on=\"true\"] { background-color: transparent; background-image: url(:/microphone_on.png); } "
                         "QPushButton#btnCamera { background-color: transparent; background-image: url(:/camera_off.png); } "
                         "QPushButton#btnCamera[turn_on=\"true\"] { background-color: transparent; background-image: url(:/camera_on.png); } "
                         "QLabel#labelCamera, #labelTranslate, #labelLanguage { color: #000000; }"
                         "QLabel#labelCamera[checked=\"true\"], #labelTranslate[checked=\"true\"], #labelLanguage[checked=\"true\"] { color: #ffffff; }"
                         "QLabel#labelUserName { color: #000000; } ";
    setStyleSheet(styleSheet);

    _modeGroup = new QButtonGroup(this);
    _modeGroup->setExclusive(true);

    _modeGroup->addButton(ui->btnModeFriends, eModeButton::Friends);
    _modeGroup->addButton(ui->btnModeCallHistory, eModeButton::CallHistory);
    _modeGroup->addButton(ui->btnModeSettings, eModeButton::Settings);

    ui->btnModeFriends->setChecked(true);

    ui->comboBoxLanguage->addItem("English");
    ui->comboBoxLanguage->addItem("Chinese");

    connect(_modeGroup, SIGNAL(buttonClicked(int)), this, SLOT(onModeSelected(int)));
    connect(ui->checkBoxCamera, SIGNAL(stateChanged(int)), this, SLOT(onCameraChecked(int)));
    connect(ui->checkBoxTranslate, SIGNAL(stateChanged(int)), this, SLOT(onTanslationChecked(int)));
    connect(ui->btnInvite, SIGNAL(pressed()), this, SLOT(onInvitePressed()));
    connect(ui->btnServerSimulation, SIGNAL(pressed()), this, SLOT(onServerSimulationPressed()));

    ui->btnMicrophone->setProperty("turn_on", true);
    ui->btnCamera->setProperty("turn_on", true);

    connect(ui->btnEndCall, SIGNAL(pressed()), this, SLOT(onBtnEndCallPressed()));
    connect(ui->btnMicrophone, SIGNAL(pressed()), this, SLOT(onBtnMicrophonePressed()));
    connect(ui->btnCamera, SIGNAL(pressed()), this, SLOT(onBtnCameraPressed()));

    ui->listViewFriends->setSelectionMode(QAbstractItemView::ExtendedSelection);

    callItems_.push_back(ui->frameCallPart_1);
    callItems_.push_back(ui->frameCallPart_2);
    callItems_.push_back(ui->frameCallPart_3);
    callItems_.push_back(ui->frameCallPart_4);
    callItems_.push_back(ui->frameCallPart_5);

    connect(ui->frameCallPart_1, SIGNAL(lowQualitySelected(long)), this, SLOT(onLowQualitySelected(long)));
    connect(ui->frameCallPart_1, SIGNAL(highQualitySelected(long)), this, SLOT(onHighQualitySelected(long)));
    connect(ui->frameCallPart_2, SIGNAL(lowQualitySelected(long)), this, SLOT(onLowQualitySelected(long)));
    connect(ui->frameCallPart_2, SIGNAL(highQualitySelected(long)), this, SLOT(onHighQualitySelected(long)));
    connect(ui->frameCallPart_3, SIGNAL(lowQualitySelected(long)), this, SLOT(onLowQualitySelected(long)));
    connect(ui->frameCallPart_3, SIGNAL(highQualitySelected(long)), this, SLOT(onHighQualitySelected(long)));
    connect(ui->frameCallPart_4, SIGNAL(lowQualitySelected(long)), this, SLOT(onLowQualitySelected(long)));
    connect(ui->frameCallPart_4, SIGNAL(highQualitySelected(long)), this, SLOT(onHighQualitySelected(long)));
    connect(ui->frameCallPart_5, SIGNAL(lowQualitySelected(long)), this, SLOT(onLowQualitySelected(long)));
    connect(ui->frameCallPart_5, SIGNAL(highQualitySelected(long)), this, SLOT(onHighQualitySelected(long)));

    _connectParamsDialog = new ConnectParamsDialog(this);
    _connectParamsDialog->show();

    connect(_connectParamsDialog, SIGNAL(paramsApplied()), this, SLOT(onConnectParamsApplied()));
}

void InitialScreen::setFriendsData(std::vector<Contact> friends)
{
    _friends = friends;

    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(1);
    for (size_t i = 0; i < _friends.size(); ++i)
    {
        model->appendRow(new QStandardItem());
        model->setData(model->index(i, 0), _friends[i].name, Qt::UserRole);
        model->setData(model->index(i, 0), QSize(ui->listViewFriends->width(), 50), Qt::SizeHintRole);
    }

    ui->listViewFriends->setModel(model);

    for (size_t i = 0; i < _friends.size(); ++i)
    {
        FriendItem *item = new FriendItem();
        item->setName(_friends[i].name);
        item->setPhone(_friends[i].phone);
        ui->listViewFriends->setIndexWidget(ui->listViewFriends->model()->index(i, 0), item);
    }
}

bool InitialScreen::isMicrophoneOn() const
{
    return ui->btnMicrophone->property("enabled").toBool();
}

bool InitialScreen::isCameraOn() const
{
    return ui->btnCamera->property("enabled").toBool();
}

void InitialScreen::OnConnected (long , const std::string& )
{
}
void InitialScreen::OnConnectionError (const std::string& )
{
}

void InitialScreen::OnStreamAdded (long streamId, const std::string& participantId, const std::string& name, int type)
{
    CallItemVideoView* callItem = GetVacantVideoView();
    if (callItem)
    {
        callItem->setStreamId(streamId);
        teeVidClient_->Subscribe(streamId, callItem);
    }
}

void InitialScreen::OnStreamRemoved(long streamId)
{
    CallItemVideoView* callItem = GetVideoViewById(streamId);
    if (callItem)
    {
        teeVidClient_->Unsubscribe(streamId);
        callItem->setStreamId(0);
    }
}

void InitialScreen::OnDisconnected (){
}
void InitialScreen::OnRoomModeChanged (RoomMode ){
}
void InitialScreen::OnError (const std::string& ){
}
void InitialScreen::OnParticipantRemoved (const std::string& ){
}
void InitialScreen::OnParticipantVideoViewRemoved (const std::string& ){
}
int InitialScreen::OnAccessPinRequested (){
    return 12345;
}
void InitialScreen::OnActiveSpeakerChanged (const std::string& , const std::string& ){
}
void InitialScreen::OnMuteAttributesUpdated (const MuteAttributes& ){
}
void InitialScreen::OnParticipantUpdated (const std::string& , const MuteAttributes& ){
}
void InitialScreen::OnRaiseHandStatusUpdated (bool ){
}

void InitialScreen::onConnectParamsApplied()
{
    InitSDK();
}

void InitialScreen::onModeSelected(int modeId)
{
    ui->stackedWidget->setCurrentIndex(modeId);
}


void InitialScreen::onCameraChecked(int state)
{
    ui->labelCamera->setProperty("checked", state == Qt::Checked);
    style()->polish(ui->labelCamera);
}


void InitialScreen::onTanslationChecked(int state)
{
    ui->labelTranslate->setProperty("checked", state == Qt::Checked);
    style()->polish(ui->labelTranslate);
    ui->comboBoxLanguage->setEnabled(state == Qt::Checked);
}


void InitialScreen::onInvitePressed()
{
    QItemSelectionModel *model = dynamic_cast<QItemSelectionModel*>(ui->listViewFriends->selectionModel());
    if (!model)
        return;

    QModelIndexList selectedIndices = ui->listViewFriends->selectionModel()->selectedIndexes();
    if (selectedIndices.count() == 0)
    {
        QMessageBox mb(QMessageBox::Critical, "Error", "Please select at least one contact");
        mb.exec();
        return;
    }

    QString msgText = "Invite contacts:\n";
    for (QModelIndex index : selectedIndices)
    {
        msgText.append(_friends[index.row()].name + " (" + _friends[index.row()].phone + ")\n");
    }
    QMessageBox mb(QMessageBox::Information, "Invitation", msgText);
    mb.exec();

    std::string room = _connectParamsDialog->GetRoom().toStdString();
    std::string user = _connectParamsDialog->GetUser().toStdString();
    teeVidClient_->ConnectTo(room, user, 0, 0);
}

void InitialScreen::onServerSimulationPressed()
{
    if (!_serverSimulationDialog)
    {
        _serverSimulationDialog = new ServerSimulationDialog(this);
        connect(_serverSimulationDialog, SIGNAL(roomNameSubmitted(std::string)), this, SLOT(onRoomNameSubmitted(std::string)));
    }
    _serverSimulationDialog->show();
}

void InitialScreen::onBtnEndCallPressed()
{
    QMessageBox mb(QMessageBox::Information, "Invitation", "Call ended");
    mb.exec();
}

void InitialScreen::onBtnMicrophonePressed()
{
    bool enabled = ui->btnMicrophone->property("turn_on").toBool();
    ui->btnMicrophone->setProperty("turn_on", !enabled);
    style()->polish(ui->btnMicrophone);
}

void InitialScreen::onBtnCameraPressed()
{
    bool enabled = ui->btnCamera->property("turn_on").toBool();
    ui->btnCamera->setProperty("turn_on", !enabled);
    style()->polish(ui->btnCamera);
}

void InitialScreen::onRoomNameSubmitted(const std::string& roomId)
{
    teeVidClient_->ConnectTo(roomId, "guest", 0, 0);
}

void InitialScreen::onLowQualitySelected(long streamId)
{
}

void InitialScreen::onHighQualitySelected(long streamId)
{
}

void InitialScreen::UnsubscribeFromVideo()
{
    for (auto iter = callItems_.begin(); iter != callItems_.end(); ++iter)
    {
        CallItemVideoView* callItem = *iter;
        if (callItem && callItem->getStreamId() > 0)
        {
            teeVidClient_->Unsubscribe(callItem->getStreamId());
            callItem->setStreamId(0);
        }
    }
}

CallItemVideoView* InitialScreen::GetVacantVideoView() const
{
    CallItemVideoView* view = nullptr;
    for (auto iter = callItems_.begin(); iter != callItems_.end(); ++iter)
    {
        CallItemVideoView* callItem = *iter;
        if (callItem && callItem->getStreamId() == 0)
        {
            view = callItem;
            break;
        }
    }
    return view;
}

CallItemVideoView* InitialScreen::GetVideoViewById(long streamId) const
{
    CallItemVideoView* view = nullptr;
    for (auto iter = callItems_.begin(); iter != callItems_.end(); ++iter)
    {
        CallItemVideoView* callItem = *iter;
        if (callItem && callItem->getStreamId() == streamId)
        {
            view = callItem;
            break;
        }
    }
    return view;
}