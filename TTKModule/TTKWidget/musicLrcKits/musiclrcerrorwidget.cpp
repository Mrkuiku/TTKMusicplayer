#include "musiclrcerrorwidget.h"
#include "ui_musiclrcerrorwidget.h"
#include "musicuiobject.h"
#include "musictoastlabel.h"

#include <QButtonGroup>

#define MAX_LEGNTH 30

MusicLrcErrorWidget::MusicLrcErrorWidget(QWidget *parent)
    : MusicAbstractMoveDialog(parent),
      m_ui(new Ui::MusicLrcErrorWidget)
{
    m_ui->setupUi(this);
    setFixedSize(size());

    m_ui->topTitleCloseButton->setIcon(QIcon(":/functions/btn_close_hover"));
    m_ui->topTitleCloseButton->setStyleSheet(MusicUIObject::MQSSToolButtonStyle04);
    m_ui->topTitleCloseButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->topTitleCloseButton->setToolTip(tr("Close"));
    connect(m_ui->topTitleCloseButton, SIGNAL(clicked()), SLOT(close()));

    m_ui->radioButton1->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->radioButton2->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->radioButton3->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->radioButton4->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);

    m_ui->textEdit->setStyleSheet(MusicUIObject::MQSSTextEditStyle01 + MusicUIObject::MQSSScrollBarStyle01);
    m_ui->textEdit->setEnabled(false);
    m_ui->radioButton1->setChecked(true);
    connect(m_ui->textEdit, SIGNAL(textChanged()), SLOT(textAreaChanged()));

    m_ui->pushButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);
    m_ui->pushButton->setCursor(QCursor(Qt::PointingHandCursor));
    connect(m_ui->pushButton, SIGNAL(clicked()), SLOT(confirmButtonClicked()));

#ifdef Q_OS_UNIX
    m_ui->radioButton1->setFocusPolicy(Qt::NoFocus);
    m_ui->radioButton2->setFocusPolicy(Qt::NoFocus);
    m_ui->radioButton3->setFocusPolicy(Qt::NoFocus);
    m_ui->radioButton4->setFocusPolicy(Qt::NoFocus);
    m_ui->pushButton->setFocusPolicy(Qt::NoFocus);
#endif

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_ui->radioButton1, 0);
    buttonGroup->addButton(m_ui->radioButton2, 1);
    buttonGroup->addButton(m_ui->radioButton3, 2);
    buttonGroup->addButton(m_ui->radioButton4, 3);
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(buttonGroup, SIGNAL(idClicked(int)), SLOT(buttonClicked(int)));
#else
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SLOT(buttonClicked(int)));
#endif
}

MusicLrcErrorWidget::~MusicLrcErrorWidget()
{
    delete m_ui;
}

void MusicLrcErrorWidget::buttonClicked(int index)
{
    if(index == 3)
    {
        m_ui->textEdit->setEnabled(true);
        m_ui->textEdit->setText(QString());
    }
}

void MusicLrcErrorWidget::textAreaChanged()
{
    QString text = m_ui->textEdit->toPlainText();
    const int length = text.count();
    if(length > MAX_LEGNTH)
    {
        QTextCursor textCursor = m_ui->textEdit->textCursor();
        const int position = textCursor.position();
        text.remove(position - (length - MAX_LEGNTH), length - MAX_LEGNTH);
        m_ui->textEdit->setText(text);
        textCursor.setPosition(position - (length - MAX_LEGNTH));
        m_ui->textEdit->setTextCursor(textCursor);
    }
    else
    {
        const QString &style = QString("<html><head/><body><p><span style=' font-size:10pt;'>%1"
                                 "</span><span style=' font-size:10pt; color:#ff0000;'> %2 "
                                 "</span><span style=' font-size:10pt;'>%3</span></p></body></html>")
                                 .arg(tr("You can now input")).arg(MAX_LEGNTH - length).arg(tr("Words"));
        m_ui->label->setText(style);
    }
}

void MusicLrcErrorWidget::confirmButtonClicked()
{
    MusicToastLabel::popup(tr("Report error finished!"));

    close();
}

int MusicLrcErrorWidget::exec()
{
    setBackgroundPixmap(m_ui->background, size());
    return MusicAbstractMoveDialog::exec();
}
