#include "musicabstractmovedialog.h"
#include "musicbackgroundmanager.h"
#include "musicbackgroundconfigmanager.h"
#include "musicextractwrapper.h"

#include <QPainter>
#include <QMouseEvent>

MusicAbstractMoveDialog::MusicAbstractMoveDialog(QWidget *parent)
    : MusicAbstractMoveDialog(true, parent)
{

}

MusicAbstractMoveDialog::MusicAbstractMoveDialog(bool transparent, QWidget *parent)
    : QDialog(parent)
{
    ///Remove the title bar
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, transparent);

    m_moveOption = false;
    m_leftButtonPress = false;
    m_showShadow = true;
    m_background = nullptr;

    G_BACKGROUND_PTR->addObserver(this);
}

MusicAbstractMoveDialog::~MusicAbstractMoveDialog()
{
    G_BACKGROUND_PTR->removeObserver(this);
}

void MusicAbstractMoveDialog::backgroundChanged()
{
    if(m_background)
    {
        setBackgroundPixmap(size());
    }
}

void MusicAbstractMoveDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    if(m_showShadow)
    {
        QPainter painter(this);
        MusicUtils::Widget::setBorderShadow(this, &painter);
    }
}

void MusicAbstractMoveDialog::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if(event->button() == Qt::LeftButton && !m_moveOption)
    {
        m_leftButtonPress = true;
    }
#if TTK_QT_VERSION_CHECK(6,0,0)
    m_pressAt = event->globalPosition().toPoint();
#else
    m_pressAt = event->globalPos();
#endif
}

void MusicAbstractMoveDialog::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if(!m_leftButtonPress)
    {
        event->ignore();
        return;
    }
#if TTK_QT_VERSION_CHECK(6,0,0)
    const int xpos = event->globalPosition().x() - m_pressAt.x();
    const int ypos = event->globalPosition().y() - m_pressAt.y();
    m_pressAt = event->globalPosition().toPoint();
#else
    const int xpos = event->globalX() - m_pressAt.x();
    const int ypos = event->globalY() - m_pressAt.y();
    m_pressAt = event->globalPos();
#endif
    move(x() + xpos, y() + ypos);
}

void MusicAbstractMoveDialog::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
#if TTK_QT_VERSION_CHECK(6,0,0)
    m_pressAt = event->globalPosition().toPoint();
#else
    m_pressAt = event->globalPos();
#endif
    m_leftButtonPress = false;
}

void MusicAbstractMoveDialog::setBackgroundPixmap(QLabel *label, const QSize &size)
{
    m_background = label;
    setBackgroundPixmap(size);
}

void MusicAbstractMoveDialog::setBackgroundPixmap(const QSize &size)
{
    QLabel *label = TTKStatic_cast(QLabel*, m_background);
    MusicBackgroundImage image;
    if(MusicExtractWrapper::outputSkin(&image, G_BACKGROUND_PTR->getBackgroundUrl()))
    {
        label->setPixmap(image.m_pix.scaled(size));
    }
    else
    {
        label->setPixmap(QPixmap(G_BACKGROUND_PTR->getBackgroundUrl()).scaled(size));
    }
}
