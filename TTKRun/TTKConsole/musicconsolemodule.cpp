#include "musicconsolemodule.h"
#include "musicplayer.h"
#include "musicplaylist.h"
#include "musicformats.h"
#include "musictkplconfigmanager.h"
#include "musichotkeymanager.h"
#include "musicfileutils.h"

#if TTK_QT_VERSION_CHECK(5,2,0)
#include <QCommandLineOption>
#include <QCommandLineParser>
#endif

MusicConsoleModule::MusicConsoleModule(QObject *parent)
    : QObject(parent)
{
    m_musicPlayer = new MusicPlayer(this);
    m_musicPlaylist = new MusicPlaylist(this);

    m_volume = 100;
    m_playbackMode = "Order";
    m_enhanced = "Off";

    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlayOrder);
    m_musicPlayer->setPlaylist(m_musicPlaylist);

    connect(m_musicPlayer, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(m_musicPlayer, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(m_musicPlaylist, SIGNAL(currentIndexChanged(int)), SLOT(currentIndexChanged(int)));

    G_HOTKEY_PTR->addHotKey("Ctrl+B");
    G_HOTKEY_PTR->addHotKey("Ctrl+Left");
    G_HOTKEY_PTR->addHotKey("Ctrl+Right");
    G_HOTKEY_PTR->addHotKey("Ctrl+Up");
    G_HOTKEY_PTR->addHotKey("Ctrl+Down");
    G_HOTKEY_PTR->addHotKey("Ctrl+1");
    G_HOTKEY_PTR->addHotKey("Ctrl+2");
    G_HOTKEY_PTR->addHotKey("Ctrl+3");
    G_HOTKEY_PTR->addHotKey("Ctrl+4");
    G_HOTKEY_PTR->addHotKey("Ctrl+5");
    G_HOTKEY_PTR->addHotKey("Alt+1");
    G_HOTKEY_PTR->addHotKey("Alt+2");
    G_HOTKEY_PTR->addHotKey("Alt+3");
    G_HOTKEY_PTR->addHotKey("Alt+4");
    G_HOTKEY_PTR->addHotKey("Alt+5");
    G_HOTKEY_PTR->addHotKey("Ctrl+Q");

    connect(G_HOTKEY_PTR->getHotKey(0), SIGNAL(activated()), SLOT(musicStatePlay()));
    connect(G_HOTKEY_PTR->getHotKey(1), SIGNAL(activated()), SLOT(musicPlayPrevious()));
    connect(G_HOTKEY_PTR->getHotKey(2), SIGNAL(activated()), SLOT(musicPlayNext()));
    connect(G_HOTKEY_PTR->getHotKey(3), SIGNAL(activated()), SLOT(musicActionVolumePlus()));
    connect(G_HOTKEY_PTR->getHotKey(4), SIGNAL(activated()), SLOT(musicActionVolumeSub()));

    connect(G_HOTKEY_PTR->getHotKey(5), SIGNAL(activated()), SLOT(musicPlayOrder()));
    connect(G_HOTKEY_PTR->getHotKey(6), SIGNAL(activated()), SLOT(musicPlayRandom()));
    connect(G_HOTKEY_PTR->getHotKey(7), SIGNAL(activated()), SLOT(musicPlaylistLoop()));
    connect(G_HOTKEY_PTR->getHotKey(8), SIGNAL(activated()), SLOT(musicPlayOneLoop()));
    connect(G_HOTKEY_PTR->getHotKey(9), SIGNAL(activated()), SLOT(musicPlayItemOnce()));

    connect(G_HOTKEY_PTR->getHotKey(10), SIGNAL(activated()), SLOT(musicEnhancedOff()));
    connect(G_HOTKEY_PTR->getHotKey(11), SIGNAL(activated()), SLOT(musicEnhanced3D()));
    connect(G_HOTKEY_PTR->getHotKey(12), SIGNAL(activated()), SLOT(musicEnhancedNICAM()));
    connect(G_HOTKEY_PTR->getHotKey(13), SIGNAL(activated()), SLOT(musicEnhancedSubwoofer()));
    connect(G_HOTKEY_PTR->getHotKey(14), SIGNAL(activated()), SLOT(musicEnhancedVocal()));

    connect(G_HOTKEY_PTR->getHotKey(15), SIGNAL(activated()), qApp, SLOT(quit()));

}

MusicConsoleModule::~MusicConsoleModule()
{
    TTK_LOGGER_INFO("\nRelease all");
    delete m_musicPlayer;
    delete m_musicPlaylist;
}

bool MusicConsoleModule::init(const QCoreApplication &app)
{
#if TTK_QT_VERSION_CHECK(5,2,0)
    QCommandLineOption op1("u", "", ".");
    QCommandLineOption op2("d", "", ".");
    QCommandLineOption op3("l", "", ".");

    QCommandLineParser parser;
    parser.addOption(op1);
    parser.addOption(op2);
    parser.addOption(op3);
    parser.process(app);

    if(app.arguments().count() == 1)
    {
        TTK_LOGGER_INFO("\nOptions:");
        TTK_LOGGER_INFO("-u //Music play url path");
        TTK_LOGGER_INFO("-d //Music play dir path");
        TTK_LOGGER_INFO("-l //Music playlist url path\n");
        return false;
    }

    if(parser.isSet(op1))
    {
        const QString &url = parser.value(op1);
        if(url.isEmpty())
        {
            TTK_LOGGER_ERROR("Music play url path is empty");
            return false;
        }
        else
        {
            TTK_LOGGER_INFO("Add play url path: " << url);
            m_musicPlaylist->addMedia(0, url);
            m_musicPlaylist->setCurrentIndex(0);
        }
    }
    else if(parser.isSet(op2))
    {
        const QString &url = parser.value(op2);
        if(url.isEmpty())
        {
            TTK_LOGGER_ERROR("Music play dir path is empty");
            return false;
        }
        else
        {
            for(const QFileInfo &file : MusicUtils::File::getFileListByDir(url, MusicFormats::supportMusicInputFilterFormats(), true))
            {
                TTK_LOGGER_INFO("Add play url path: " << file.absoluteFilePath());
                m_musicPlaylist->appendMedia(0, file.absoluteFilePath());
            }

            if(!m_musicPlaylist->isEmpty())
            {
                m_musicPlaylist->setCurrentIndex(0);
            }
        }
    }
    else if(parser.isSet(op3))
    {
        const QString &url = parser.value(op3);
        if(url.isEmpty())
        {
            TTK_LOGGER_ERROR("Music playlist path is empty");
            return false;
        }
        else
        {
            if(QFileInfo(url).suffix().toLower() != LST_FILE_PREFIX)
            {
                TTK_LOGGER_INFO("Music playlist format not support");
                return false;
            }

            MusicSongItems items;
            MusicTKPLConfigManager manager;
            if(!manager.readConfig(url))
            {
                TTK_LOGGER_ERROR("Music playlist read error");
                return false;
            }
            manager.readPlaylistData(items);

            for(const MusicSongItem &item : qAsConst(items))
            {
                for(const MusicSong &song : qAsConst(item.m_songs))
                {
                    TTK_LOGGER_INFO("Add play url path: " << song.getMusicPath());
                    m_musicPlaylist->appendMedia(0, song.getMusicPath());
                }
            }

            if(!m_musicPlaylist->isEmpty())
            {
                m_musicPlaylist->setCurrentIndex(0);
            }
        }
    }
    else
    {
        TTK_LOGGER_ERROR("Options error");
        return false;
    }

    TTK_LOGGER_INFO("\nMusic Files count: " << m_musicPlaylist->mediaCount() << "\n");

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
#else
    TTK_LOGGER_ERROR("Qt version less than 5.2 not support commend line");
#endif

    return app.exec();
}

void MusicConsoleModule::durationChanged(qint64 duration)
{
    print(0, duration);
}

void MusicConsoleModule::positionChanged(qint64 position)
{
    print(position, m_musicPlayer->duration());
}

void MusicConsoleModule::currentIndexChanged(int index)
{
    TTK_LOGGER_INFO("\nCurrent Play Indedx: " << index);
    QTimer::singleShot(MT_S2MS, this, SLOT(resetVolume()));
}

void MusicConsoleModule::musicStatePlay()
{
    if(m_musicPlaylist->isEmpty())
    {
        return;
    }

    if(m_musicPlayer->isPlaying())
    {
        m_musicPlayer->pause();
    }
    else
    {
        m_musicPlayer->play();
        m_musicPlayer->setVolume(m_volume);
    }
}

void MusicConsoleModule::musicPlayPrevious()
{
    if(m_musicPlaylist->isEmpty())
    {
        return;
    }

    if(m_musicPlaylist->playbackMode() == MusicObject::PM_PlayRandom)
    {
        m_musicPlaylist->setCurrentIndex();
    }
    else
    {
        m_musicPlayer->playPrevious();
    }

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleModule::musicPlayNext()
{
    if(m_musicPlaylist->isEmpty())
    {
        return;
    }

    if(m_musicPlaylist->playbackMode() == MusicObject::PM_PlayRandom)
    {
        m_musicPlaylist->setCurrentIndex();
    }
    else
    {
        m_musicPlayer->playNext();
    }

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleModule::resetVolume()
{
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleModule::musicActionVolumeSub()
{
    m_volume = m_musicPlayer->volume();
    m_volume -= 15;
    if(m_volume < 0)
    {
        m_volume = 0;
    }

    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleModule::musicActionVolumePlus()
{
    m_volume = m_musicPlayer->volume();
    m_volume += 15;
    if(m_volume > 100)
    {
        m_volume = 100;
    }

    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleModule::musicPlayOrder()
{
    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlayOrder);
    m_playbackMode = "Order";
}

void MusicConsoleModule::musicPlayRandom()
{
    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlayRandom);
    m_playbackMode = "Random";
}

void MusicConsoleModule::musicPlaylistLoop()
{
    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlaylistLoop);
    m_playbackMode = "ListLoop";
}

void MusicConsoleModule::musicPlayOneLoop()
{
    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlayOneLoop);
    m_playbackMode = "OneLoop";
}

void MusicConsoleModule::musicPlayItemOnce()
{
    m_musicPlaylist->setPlaybackMode(MusicObject::PM_PlayOnce);
    m_playbackMode = "Once";
}

void MusicConsoleModule::musicEnhancedOff()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::EnhancedOff);
    m_enhanced = "Off";
}

void MusicConsoleModule::musicEnhanced3D()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::Enhanced3D);
    m_enhanced = "3D";
}

void MusicConsoleModule::musicEnhancedNICAM()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::EnhancedNICAM);
    m_enhanced = "NICAM";
}

void MusicConsoleModule::musicEnhancedSubwoofer()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::EnhancedSubwoofer);
    m_enhanced = "Subwoofer";
}

void MusicConsoleModule::musicEnhancedVocal()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::EnhancedVocal);
    m_enhanced = "Vocal";
}

void MusicConsoleModule::print(qint64 position, qint64 duration)
{
    TTK_LOGGER_INFO(QString("Music Name: %1, Time:[%2/%3], Volume:%4, PlaybackMode:%5, Enhance:%6"))
                .arg(m_musicPlaylist->currentMediaPath())
                .arg(MusicTime::msecTime2LabelJustified(position))
                .arg(MusicTime::msecTime2LabelJustified(duration))
                .arg(m_musicPlayer->volume())
                .arg(m_playbackMode)
                .arg(m_enhanced);
}
