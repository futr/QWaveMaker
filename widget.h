#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioInput>
#include <QBuffer>
#include <QMessageBox>
#include <QtMath>
#include <QDataStream>
#include <QDebug>
#include "wavedevice.h"
#include "fftw3.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void waveChange( void );
    void audioBuffering( void );
    void setVolume( int vol );
    void doFFT( void );
    void doIFFT( void );
    void readInputAudioData( void );
    void on_stopButton_clicked();

    void on_playButton_clicked();

    void on_startRecordButton_clicked();

    void on_stopRecordButton_clicked();

private:
    Ui::Widget *ui;

    QAudioOutput *audioOutpu;
    QAudioInput *audioInput;
    QIODevice *inputDevice;
    WaveDevice waveFIFO;
    int numData;
    qint64 readInputPos;

    QVector<qreal> outputWave;
};

#endif // WIDGET_H
