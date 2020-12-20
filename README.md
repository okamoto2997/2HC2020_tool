# 2HC2020_tool

日立北大コンの支援ツール

- `script/make_test_case.sh`: コンテストに合わせたテストケースを生成するスクリプト
  - `in/test_case_[day_type]_[id]_[seed].in` という形式で16個作る
  - Day Type の割合はコンテストと同じ
  - `in/` がある状態で実行すると "Are you sure?" とか聞かれる．`in` を作り直すことになるので，それでよければ`y`
- `script/run_all_test.sh`: テストケースを全実行するスクリプト
  - `in/` 以下のファイルをすべてテストケースだと思って実行する
  - `script/make_test_case.sh` でテストケースを作成していると思って動くが，別にgenerator_[AB]で作っていても問題なく実行してくれるはず
  - `result_[日時]/` を作ってその中に結果を格納する．ディレクトリがどんどん増えてくので適当なところで整理するといいはず
  - `result_[日時]/summary` に各テストケースの得点と総得点が格納されている．なんならこれだけでいいかもしれない

## 使い方

1. toolkit を問題ページからダウンロード -> 解凍
2. toolkit のディレクトリにいどうして，`script/`をぺたり
  - この時点で，toolkit以下には下記のディレクトリ・ファイルがいるはず(A問題のtoolkitの場合)
    - `script/`
    - `src/`
    - `Makefile`
    - `README_A_EN(short ver.).md`
    - `README_A_JP.md`
3. `script/make_test_case.sh` でテストケースを作成
4. `script/run_all_test.sh answer` で`answer`を利用してすべてのテストケースを実行
  - `answer`は解答コードの実行ファイル．スクリプト言語ならスクリプトのファイルそのものでいけるはず．コンパイル言語ならコンパイルした後の実行ファイルを指定．
