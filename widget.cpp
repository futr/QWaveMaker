#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 波ウィジェット初期化
    ui->inputWave->setEditable();
    ui->inputWave->setDrawStyle( WaveWidget::dsScale );
    ui->inputWave->setZeroMode( WaveWidget::zmCenter );
    ui->inputWave->setYScaleMode( WaveWidget::ymExpand );
    ui->inputWave->setExpandYLevel( 256 );
    ui->inputWave->setDrawCenter();
    ui->inputWave->setWave( QVector<qreal>( 100 ) );
    ui->inputWave->setWaveLine( QPen( Qt::blue, 3 ), Qt::white, WaveWidget::lsLine );

    ui->fourierAmp->setEditable();
    ui->fourierAmp->setDrawStyle( WaveWidget::dsScale );
    ui->fourierAmp->setZeroMode( WaveWidget::zmBottom );
    ui->fourierAmp->setYScaleMode( WaveWidget::ymExpand );
    ui->fourierAmp->setExpandYLevel( 256 );
    ui->fourierAmp->setWaveLine( QPen( Qt::green, 3 ), Qt::white, WaveWidget::lsBar );

    ui->fourierPhase->setEditable();
    ui->fourierPhase->setDrawStyle( WaveWidget::dsScale );
    ui->fourierPhase->setZeroMode( WaveWidget::zmCenter );
    ui->fourierPhase->setDrawCenter();
    ui->fourierPhase->setYScaleMode( WaveWidget::ymExpand );
    ui->fourierPhase->setExpandYLevel( 2 * M_PI );
    ui->fourierPhase->setWaveLine( QPen( Qt::red, 3 ), Qt::white, WaveWidget::lsBar );

    // 8bit unsigned
    QAudioFormat format;
    format.setChannelCount( 1 );
    format.setSampleRate( 44100 );
    format.setSampleType( QAudioFormat::UnSignedInt );
    format.setSampleSize( 8 );
    format.setCodec( "audio/pcm" );

    // この設定で再生できるか確認
    QAudioDeviceInfo info( QAudioDeviceInfo::defaultOutputDevice() );

    if ( !info.isFormatSupported( format ) ) {
        // サポートされていないので失敗
        QMessageBox::critical( this, "エラー", "オーディオ出力の初期化に失敗しました" );
    }

    // 出力バッファ開く
    waveFIFO.open( WaveDevice::ReadWrite );

    // QAudioOutput初期化
    audioOutpu = new QAudioOutput( format, this );

    // シグナル接続
    connect( ui->freqSpinBox,   SIGNAL(valueChanged(int)),   this,              SLOT(waveChange()) );
    connect( ui->freqDial,      SIGNAL(valueChanged(int)),   ui->freqSpinBox,   SLOT(setValue(int)) );
    connect( ui->freqSpinBox,   SIGNAL(valueChanged(int)),   ui->freqDial,      SLOT(setValue(int)) );
    connect( ui->waveTypeBox,   SIGNAL(currentIndexChanged(int)), this,         SLOT(waveChange()) );
    connect( ui->volumeSpinBox, SIGNAL(valueChanged(int)),   ui->volumeSlider,  SLOT(setValue(int)) );
    connect( ui->volumeSlider,  SIGNAL(valueChanged(int)),   ui->volumeSpinBox, SLOT(setValue(int)) );
    connect( ui->volumeSpinBox, SIGNAL(valueChanged(int)),   this,              SLOT(setVolume(int)) );
    connect( ui->inputWave,     SIGNAL(waveChanged()),       this,              SLOT(doFFT()) );
    connect( ui->inputWave,     SIGNAL(waveChanged()),       this,              SLOT(audioBuffering()) );
    connect( ui->fourierAmp,    SIGNAL(waveChangedByHand()), this,              SLOT(doIFFT()) );
    connect( ui->fourierPhase,  SIGNAL(waveChangedByHand()), this,              SLOT(doIFFT()) );
    connect( ui->biasSpinBox,   SIGNAL(valueChanged(int)),   this,              SLOT(waveChange()) );
    connect( ui->ampSpinBox,    SIGNAL(valueChanged(int)),   this,              SLOT(waveChange()) );

    // 波をとりあえず書く
    waveChange();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::waveChange( void )
{
    // 波の形が変更された
    QVector<qreal> inputData;
    int index;
    int i;
    qint32 frames;
    qreal amp  = ui->ampSpinBox->value();50;127;
    qreal bias = ui->biasSpinBox->value();0;

    // 波の酒類を取得
    index = ui->waveTypeBox->currentIndex();

    // 入力データーの個数を決定
    frames = audioOutpu->format().framesForDuration( 1.0 / ui->freqSpinBox->value() * 1000000 );

    inputData.resize( frames );

    // 波形指定に従って波形を作成
    if ( index == 0 ) {
        // 正弦波
        for ( i = 0; i < inputData.size(); i++ ) {
            inputData[i] = qSin( (qreal)i / inputData.size() * 2 * M_PI ) * amp + bias;
        }

        // 波を設定
        ui->inputWave->setWave( inputData );
    } else if ( index == 1 ) {
        // 方形波
        for ( i = 0; i < inputData.size(); i++ ) {
            if ( i < inputData.size() / 2 ) {
                inputData[i] = amp + bias;
            } else {
                inputData[i] = -amp + bias;
            }
        }

        // 波を設定
        ui->inputWave->setWave( inputData );
    } else if ( index == 2 ) {
        // のこぎり波
        for ( i = 0; i < inputData.size(); i++ ) {
            inputData[i] = ( (qreal)i / inputData.size() ) * amp * 2 + bias - amp;
        }

        // 波を設定
        ui->inputWave->setWave( inputData );
    } else if ( index == 3 ) {
        // 三角波
        qreal val;

        for ( i = 0; i < inputData.size(); i++ ) {
            if ( i < inputData.size() / 4 ) {
                val = ( (qreal)i / ( inputData.size() / 4 ) ) * amp + bias;
            } else if ( inputData.size() / 4 <= i && i < inputData.size() * 2 / 4 ) {
                val = amp - ( (qreal)( i - inputData.size() / 4 ) / ( inputData.size() / 4 ) ) * amp + bias;
            } else if ( inputData.size() * 2 / 4 <= i && i < inputData.size() * 3 / 4 ) {
                val = -( (qreal)( i - inputData.size() * 2 / 4 ) / ( inputData.size() / 4 ) ) * amp + bias;
            } else {
                val = -amp + ( (qreal)( i - inputData.size() * 3 / 4 ) / ( inputData.size() / 4 ) ) * amp + bias;
            }

            inputData[i] = val;
        }

        // 波を設定
        ui->inputWave->setWave( inputData );
    } else if ( index == 4 ) {
        // 手書き
        QVector<qreal> oldData;
        qreal scale;
        int pos;

        ui->inputWave->setEditable();

        // 波取得
        oldData = ui->inputWave->getWave();

        // 倍率
        scale = (qreal)inputData.size() / oldData.size();

        // 波を現在のデーター数に変換
        for ( i = 0; i < inputData.size(); i++ ) {
            pos = i / scale;

            // 古いデータの範囲内ならコピー
            if ( pos >= 0 && pos < oldData.size() ) {
                inputData[i] = oldData[pos];
            }
        }

        // 波設定
        ui->inputWave->setWave( inputData );
    } else if ( index == 5 ) {
        qreal val;

        for ( i = 0; i < inputData.size(); i++ ) {
            inputData[i] = ( (qreal)qrand() - RAND_MAX / 2 ) / ( RAND_MAX / 2 ) * amp + bias;
        }

        // 波を設定
        ui->inputWave->setWave( inputData );
    }
}

void Widget::audioBuffering( void )
{
    // バッファ内に1000ms秒分残るようにバッファリング
    int i;
    QByteArray output;

    // 入力をそのままコピー
    outputWave = ui->inputWave->getWave();

    // 出力データー作成
    output.resize( outputWave.size() );

    // 波形をUnSignedintに変換
    for ( i = 0; i < outputWave.size(); i++ ) {
        // 出力波形クリッピング
        if ( outputWave[i] > 0 ) {
            outputWave[i] = qMin( outputWave[i], 127.0 );
        } else {
            outputWave[i] = qMax( outputWave[i], -127.0 );
        }

        output[i] = outputWave[i] + 127;
    }

    // 送出
    waveFIFO.write( output );
}

void Widget::setVolume( int vol )
{
    // ボリューム設定
    audioOutpu->setVolume( vol / 100.0 );
}

void Widget::on_stopButton_clicked()
{
    // 再生停止
    audioOutpu->stop();
}

void Widget::on_playButton_clicked()
{
    // 再生開始
    setVolume( ui->volumeSpinBox->value() );

    outputWave = ui->inputWave->getWave();

    // 波設定
    audioBuffering();

    // 開始
    audioOutpu->start( &waveFIFO );
}

void Widget::doFFT()
{
    // FFT実行
    int i;
    fftw_complex *input;
    fftw_complex *output;
    fftw_plan fftPlan;
    QVector<qreal> inputData;
    QVector<qreal> phaseData;
    QVector<qreal> ampData;
    QVector<QPointF> fftAmpPhase;

    // 波形取得
    inputData = ui->inputWave->getWave();

    // FFT用配列確保
    input  = (fftw_complex *)fftw_malloc( sizeof(fftw_complex) * inputData.size() );
    output = (fftw_complex *)fftw_malloc( sizeof(fftw_complex) * inputData.size() );

    // 入力の幅を保存
    numData = inputData.size();

    // 波をFFT用配列に入力
    for ( i = 0; i < inputData.size(); i++ ) {
        input[i][0] = inputData[i];
        input[i][1] = 0;
    }

    // FFTプラン作成
    fftPlan = fftw_plan_dft_1d( inputData.size(), input, output, FFTW_FORWARD, FFTW_ESTIMATE );

    // FFT実行
    fftw_execute( fftPlan );

    // 結果をコピー
    fftAmpPhase.resize( inputData.size() );
    ampData.resize( inputData.size() / 2 + 1 );
    phaseData.resize( inputData.size() / 2 + 1 );

    for ( i = 0; i < inputData.size(); i++ ) {
        fftAmpPhase[i].setX( qSqrt( output[i][0] * output[i][0] + output[i][1] * output[i][1] ) );

        // エルミート性を考えて後半の虚部を反転　わる2+1 no index
        if ( i > inputData.size() / 2 ) {
            // 反転
            fftAmpPhase[i].setY( qAtan2( -output[i][1], output[i][0] ) );
        } else {
            fftAmpPhase[i].setY( qAtan2( output[i][1], output[i][0] ) );
        }
    }

    for ( i = 0; i < ampData.size(); i++ ) {
        ampData[i]   = fftAmpPhase[i].x();
        phaseData[i] = fftAmpPhase[i].y();
    }

    // FFT結果出力
    ui->fourierAmp->setExpandYLevel( qSqrt( qPow( 127 / qSqrt( 2 ), 2 ) * numData * numData ) / qSqrt( 2 ) );
    ui->fourierAmp->setWave( ampData );
    ui->fourierPhase->setWave( phaseData );

    // FFT用データー解放
    fftw_free( input );
    fftw_free( output );
}

void Widget::doIFFT()
{
    // 逆FFTを行う
    int i;
    int sub;
    int pos;
    fftw_complex *input;
    fftw_complex *output;
    fftw_plan fftPlan;
    QVector<qreal> inputData;
    QVector<qreal> phaseData;
    QVector<qreal> ampData;
    QVector<QPointF> fftAmpPhase;

    // 波形取得
    inputData = ui->inputWave->getWave();

    // FFT用配列確保
    input  = (fftw_complex *)fftw_malloc( sizeof(fftw_complex) * numData );
    output = (fftw_complex *)fftw_malloc( sizeof(fftw_complex) * numData );

    // 振幅・位相取得
    phaseData = ui->fourierPhase->getWave();
    ampData   = ui->fourierAmp->getWave();

    // もともときすうか偶数かで現残量を決める
    if ( numData % 2 ) {
        sub = 1;
    } else {
        sub = 2;
    }

    // 波をIFFT用配列に入力
    for ( i = 0; i < numData; i++ ) {
        if ( i < numData / 2 + 1 ) {
            input[i][0] = ampData[i] * qCos( phaseData[i] );
            input[i][1] = ampData[i] * qSin( phaseData[i] );
        } else {
            pos = ( numData / 2 + 1 ) - ( i - ( numData / 2 + 1 ) ) - sub;
            input[i][0] = ampData[pos] * qCos( phaseData[pos] );
            input[i][1] = -ampData[pos] * qSin( phaseData[pos] );
        }
    }

    // FFTプラン作成
    fftPlan = fftw_plan_dft_1d( numData, input, output, FFTW_BACKWARD, FFTW_ESTIMATE );

    // FFT実行
    fftw_execute( fftPlan );

    // 結果をコピー
    inputData.resize( numData );

    for ( i = 0; i < inputData.size(); i++ ) {
        inputData[i] = output[i][0] / inputData.size();
    }

    // IFFT結果出力
    ui->inputWave->setWave( inputData );

    // FFT用データー解放
    fftw_free( input );
    fftw_free( output );
}
