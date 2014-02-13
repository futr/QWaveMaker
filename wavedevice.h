/*
 * 波を出力してくれるデバイス
 *
 *
 */

#ifndef WAVEDEVICE_H
#define WAVEDEVICE_H

#include <QIODevice>
#include <QVector>
#include <QByteArray>
#include <QMutex>

class WaveDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit WaveDevice(QObject *parent = 0);

    void setWave();

protected:
    virtual qint64 readData( char *data, qint64 maxlen );
    virtual qint64 writeData( const char *data, qint64 len );
    virtual bool isSequential( void ) const;
    virtual qint64 bytesAvailable( void ) const;

signals:

public slots:

private:
    QByteArray waveData;
    qint64 bufferPos;
    QMutex mutex;
};

#endif // WAVEDEVICE_H
