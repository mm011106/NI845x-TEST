 NI845x-TEST


### NI-8451 を試してみました。
32bit送信を想定して4バイトのデータを連続して投げることを試しています。

1MBPSの設定です。極性などはサンプルプログラムのままなので、細かく追っていません。

1ms間隔で32bitのチャンクを10回送信しています。



__全体__

![tek00013](https://user-images.githubusercontent.com/9587359/49413490-78a3af80-f7b3-11e8-9228-fa44b7b363e7.png)

時間間隔は結構いい加減です。


__32bitチャンクの拡大__

![tek00015](https://user-images.githubusercontent.com/9587359/49779862-6e535980-fd4f-11e8-885a-3d98e7a1a9b5.png)


8bitの間が結構空いています。これが問題かどうかは検討の必要があります。

この後100kspsに設定し直して確認しましたが、8bitの間の間隔はほぼわからなくなるくらい綺麗になっています。これなら全く問題なしかともいます。
