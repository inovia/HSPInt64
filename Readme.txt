☆ hspint64

■ 免責事項

・本ソフトは無料で使うことができます
・付属のソースコードのライセンスはOpenHSPのライセンスに準拠します
・本ソフトを利用した事によるいかなる損害も作者は一切の責任を負いません
・配布、転載、改造は無断かつ自由にして構いません（大歓迎）
・追記・新規追加したソースコード汚くてごめんなさい
・たぶんバグがあります。
・hspint64のソースコードはgithubにて管理しています。
https://github.com/inovia/HSPInt64

■ 対応機能

int()
lpeek()
lpoke
varptr()
dupptr()
callfunc()

のなどの64bit対応版

■ 連絡先 

イノビア(inovia)
https://twitter.com/hk1v
https://hsp.moe/
https://github.com/inovia/

■ 謝辞

本ソフトは OpenHSP/HSP3SDK を使用しております。
http://www.onionsoft.net/hsp/openhsp/

上大氏のサイトを参考にしました。
http://prograpark.ninja-web.net/HSP/index.html

■ バージョン

　 3.5b2  - 1.00     - 初期バージョン（製作時間：計5時間ほど）
   3.6b4  - 1.01     - 
・不具合の修正
・可変長引数形式のcallfunc風関数を追加 [cfunc64]
　（配列で引数データを渡さないので直感的）
・float型、UTF-16型変数型を追加
　（Win32APIやDXライブラリを扱う向け）
   3.6b5  - 1.02     - 
・qpeek,qpokeの不具合修正
・libptr64関数追加
   3.6b5  - 1.03     - 
・コールバック関数に対応
   3.6b5  - 1.04     - 
・不具合を修正
　・hspint64.as の記述ミスにより、Callback系が使えなくなっていた
　・UTF-16型文字列変数のメモリ回り不具合を修正
　・コールバック関数のfloat値が渡らない不具合を修正