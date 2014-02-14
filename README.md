QWaveMaker
==========

一周期分の波を生成して音として鳴らすGUIソフトです．

ビルド方法
---------

* 必要環境
    * Qt5以上推奨
    * libfftw3

Unix系OSではlibfftw3が入っていれば恐らくそのままビルド可能です．  
QtCreatorでproを開いてビルド・実行してください．

Windowsではソースコードと同じディレクトリに，fftw3.hとlibfftw3-3.aを用意し，
実行ファイルのカレントディレクトリにlibfftw3-3.dllを用意すればビルド可能です．
