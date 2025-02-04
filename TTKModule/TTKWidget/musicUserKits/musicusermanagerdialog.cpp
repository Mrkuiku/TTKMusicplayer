#include "musicusermanagerdialog.h"
#include "ui_musicusermanagerdialog.h"
#include "musicuiobject.h"
#include "musicusermodel.h"
#include "musicuserconfigmanager.h"
#include "musicuserrecordwidget.h"

MusicUserManagerDialog::MusicUserManagerDialog(QWidget *parent)
     : QDialog(parent),
       m_ui(new Ui::MusicUserManagerDialog)
{
    m_ui->setupUi(this);
    setFixedSize(size());

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    createButtonPopMenu();

#ifdef Q_OS_UNIX
    m_ui->muiscPurchasedButton->setFocusPolicy(Qt::NoFocus);
    m_ui->muiscBalanceButton->setFocusPolicy(Qt::NoFocus);
    m_ui->muiscOVipButton->setFocusPolicy(Qt::NoFocus);
    m_ui->logoffButton->setFocusPolicy(Qt::NoFocus);
    m_ui->musicSettingButton->setFocusPolicy(Qt::NoFocus);
    m_ui->vipButton->setFocusPolicy(Qt::NoFocus);
    m_ui->muiscPackButton->setFocusPolicy(Qt::NoFocus);
#endif

    connect(m_ui->logoffButton, SIGNAL(clicked()), SLOT(musicUserLogoff()));
    connect(m_ui->userIcon, SIGNAL(clicked()), SLOT(popupUserRecordWidget()));
    connect(m_ui->userName, SIGNAL(clicked()), SLOT(popupUserRecordWidget()));
}

MusicUserManagerDialog::~MusicUserManagerDialog()
{
    if(!m_userUID.m_uid.isEmpty())
    {
        m_userModel->updateUser(m_userUID, QString(), QString(), m_ui->userName->text(),
                                QString::number(m_userModel->getUserLogTime(m_userUID).toLongLong() + m_timer.elapsed() / (MT_S2MS * 30)));
    }
    delete m_ui;
}

void MusicUserManagerDialog::setUserUID(const MusicUserUIDItem &uid)
{
    m_userUID = uid;
    m_ui->userName->setText(m_userModel->getUserName(uid));
    m_ui->userIcon->setPixmap(QPixmap(m_userModel->getUserIcon(uid)).scaled(m_ui->userIcon->size()));

    createUserTime();
    m_timer.start();
}

void MusicUserManagerDialog::setUserModel(MusicUserModel *model)
{
    m_userModel = model;
}

void MusicUserManagerDialog::createUserTime() const
{
    const qint64 time = m_userModel->getUserLogTime(m_userUID).toLongLong();
    m_ui->totalTimeLabel->setText(QString::number(time));
}

void MusicUserManagerDialog::createButtonPopMenu()
{
    m_popMenu.addAction(tr("Modifies"), this, SLOT(popupUserRecordWidget()));
    m_popMenu.addAction(tr("Switches"), this, SLOT(musicUserLogoff()));
    m_popMenu.addAction(tr("Spacing"));
    m_popMenu.setStyleSheet(MusicUIObject::MQSSMenuStyle02);
    m_ui->musicSettingButton->setMenu(&m_popMenu);
    MusicUtils::Widget::adjustMenuPosition(&m_popMenu);
}

void MusicUserManagerDialog::musicUserLogoff()
{
    m_userModel->updateUser(m_userUID, QString(), QString(), m_ui->userName->text(),
                            QString::number(m_userModel->getUserLogTime(m_userUID).toLongLong() + m_timer.elapsed() / (MT_S2MS * 30)));

    MusicUserConfigManager xml;
    if(!xml.readConfig())
    {
        return;
    }
    MusicUserRecords records;
    xml.readUserData(records);

    int index = -1;
    for(int i=0; i<records.count(); ++i)
    {
        if(records[i].m_uid == m_userUID.m_uid)
        {
            index = i;
        }
    }
    if(index != -1)
    {
        records[index].m_auto = false;  //auto login flag
    }
    xml.writeUserData(records);

    m_userUID.m_uid.clear();
    Q_EMIT userStateChanged(MusicUserUIDItem(), QString());
    close();
}

int MusicUserManagerDialog::exec()
{
    QWidget *pa = TTKStatic_cast(QWidget*, parent());
    const QPoint &point = pa->mapToGlobal(QPoint(0, 0));
    move(point.x(), point.y() + 27);
    return QDialog::exec();
}

void MusicUserManagerDialog::leaveEvent(QEvent *event)
{
    QDialog::leaveEvent(event);
    close();
}

void MusicUserManagerDialog::popupUserRecordWidget()
{
#if !TTK_QT_VERSION_CHECK(5,0,0)
    close();
#endif
    if(m_userUID.m_server <= 0)
    {
        MusicUserRecordWidget record;
        connect(&record, SIGNAL(resetUserName(QString)), SLOT(resetUserName(QString)));
        connect(&record, SIGNAL(userIconChanged(MusicUserUIDItem,QString)), SIGNAL(userStateChanged(MusicUserUIDItem,QString)));
        record.setUserModel(m_userModel, m_userUID);
        record.exec();
    }
}

void MusicUserManagerDialog::resetUserName(const QString &name)
{
    m_ui->userName->setText(name);
}
