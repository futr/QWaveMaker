#include "wavedevice.h"

WaveDevice::WaveDevice(QObject *parent) :
    QIODevice(parent),
    bufferPos( 0 )
{
}

qint64 WaveDevice::readData( char *data, qint64 maxlen )
{
    // 波形データーを読み込み
    int i;

    // シリアライズ
    mutex.lock();

    // waveDataがなければ失敗
    if ( waveData.size() == 0 ) {
        return 0;
    }

    // 指定された分だけ渡す
    for ( i = 0; i < maxlen; i++ ) {
        data[i] = waveData.constData()[(int)bufferPos];
        // data[i] = waveData.constData()[(int)bufferPos] + (qreal)qrand() / RAND_MAX * 100;

        // ぐるぐるまわす
        bufferPos++;

        if ( bufferPos >= waveData.size() ) {
            bufferPos = 0;
        }
    }

    mutex.unlock();

    // 全部送った
    return maxlen;
}

qint64 WaveDevice::writeData( const char *data, qint64 len )
{
    // 波形データー書き込み

    // シリアライズ
    mutex.lock();

    // コピー
    waveData = QByteArray( data, len );

    // 位置をクリア
    bufferPos = 0;

    mutex.unlock();

    return len;
}

bool WaveDevice::isSequential( void ) const
{
    // シーケンシャルデバイスです
    return true;
}

qint64 WaveDevice::bytesAvailable( void ) const
{
    // 読み込めるバイト数

    // 1波長分しかよめない
    return waveData.size() + QIODevice::bytesAvailable();
}
