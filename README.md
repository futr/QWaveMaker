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

WindowsとMac用のlibfftw3を添付しているので，これらの環境ではそのままビルド可能です．  
Windowsの場合，動作にlibfftw3-3.dllが必要となります．
DLLはソースツリーのfftw内に添付しているのでこれをコピーして利用してください．

Windowsは8.1と7，Macは10.9での動作を確認しています．

このアプリケーションはGPLv2です．

