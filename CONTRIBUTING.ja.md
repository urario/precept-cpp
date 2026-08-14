# Precept への貢献

これは [CONTRIBUTING.md](CONTRIBUTING.md) の日本語訳です。英語版が normative であり、内容が食い違う
場合は英語版が優先されます。

## なぜ貢献するか

Precept は、関数の中に埋もれた `size() >= 16` のような意味的な前提条件を、シグネチャに書ける小さな型
に変えるライブラリです。それによって何が得られ、逆に何にはならないと決めているかは
[Project Charter](knowledge/vision/project-charter.md) にあります。

貢献しやすい理由は 2 つです。

* **スコープが小さい。** v0.1 は span の API 4 つで、フレームワークではありません。
* **誰かの頭の中で決まっていることがない。** あなたの変更がレビューされる基準はすべて
  [knowledge bundle](knowledge/index.md) に書かれていて、このページからリンクされています。

## 言語

Issue、Pull Request、レビュー、ディスカッションは原則 **日本語** です。英語も大歓迎で、誰にも切り替え
を求めません。

リポジトリの成果物 — README、コードコメント、public API ドキュメント、識別子、サンプル、
user-facing text、knowledge bundle — は **英語** で書きます。方針の全文は
[Documentation Rules](knowledge/rules/documentation.md) にあります。

## まずここから

```sh
git clone https://github.com/urario/precept-cpp.git
cd precept-cpp
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

初回の configure では、CMake が pin された GoogleTest を取得するためネットワークアクセスが必要です。

Visual Studio のような multi-configuration generator では、ビルドとテストで同じ構成を指定します。

```sh
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

ライブラリのみの構成では GoogleTest を取得もビルドもしません。

```sh
cmake -S . -B build-no-tests -DBUILD_TESTING=OFF
cmake --build build-no-tests
```

### knowledge check

`tools/check_knowledge.py` は knowledge bundle を検査し、他のテストと同じく CTest 配下で走ります —
ただし Python 3 と PyYAML が使える場合に限ります。どちらかが無いと、CMake は configure 時にその旨を
伝え、knowledge のテストを登録しないだけなので、[まずここから](#まずここから) の素の `ctest` は
bundle を一度も検査しないまま成功することがあります。knowledge を編集しながら単体で走らせるには:

```sh
python -m pip install pyyaml==6.0.3
python tools/check_knowledge.py knowledge
```

CI と同じ保証 — knowledge のテストが黙ってスキップされず実際に走ること — を得るには、PyYAML を
インストールしたうえで configure します。

```sh
python -m pip install pyyaml==6.0.3
cmake -S . -B build -DBUILD_TESTING=ON -DPRECEPT_REQUIRE_KNOWLEDGE_CHECK=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

これにより、interpreter または PyYAML が無い状態は黙ったスキップではなく configure エラーになります。
どちらも開発専用で、ライブラリの要件になることはありません。

### フォーマット

`.clang-format` がソースのスタイルを定義します。対象はリポジトリ内のすべての C++ ファイル —
public header、テスト、examples — です。確認・適用します。

```sh
clang-format --dry-run --Werror include/precept/span/*.hpp tests/*.cpp tests/negative/*.cpp examples/*.cpp
clang-format -i include/precept/span/*.hpp tests/*.cpp tests/negative/*.cpp examples/*.cpp
```

Markdown 内のコード例には `clang-format` が届かないので、編集するときは同じ brace / indent スタイルに
手で合わせてください。

clang-tidy は必須のゲートです（[ADR-0007](knowledge/decisions/adr-0007-versioning-compatibility-and-support.md) 参照）。CI への組み込みと `.clang-tidy` 設定は
[issue #27](https://github.com/urario/precept-cpp/issues/27) で追跡しており、それが入るまでは
決まったローカル実行コマンドはありません。

## コードを書く前に

1. [knowledge index](knowledge/index.md) を読む。
2. その変更に関係する ADR / Rule / Contract を読む。

書き直しを防ぐのはこのステップです。knowledge bundle は設計判断の source of truth であり、レビューが
参照するのもここです。

## マージされる条件

* **public API** は [API Admission Rules](knowledge/architecture/api-admission-rules.md) の 8 つを
  すべて満たすこと。1 つでも満たさないなら、後で直す課題ではなく提案を作り直す理由です。新しい API は
  実装前に Issue で提案してください。受け入れられた API の形は
  [Design Principles](knowledge/architecture/design-principles.md) が決めます。
* **スコープ** は v0.1 の範囲、つまり span のサイズ前提条件に収めます
  （[ADR-0005](knowledge/decisions/adr-0005-v0-1-span-scope.md)）。その外側のアイデアは Issue として
  歓迎します。
* **基本方針** を維持すること: C++20、header-only、consumer 依存ゼロ
  （[ADR-0001](knowledge/decisions/adr-0001-cpp20.md)、
  [ADR-0002](knowledge/decisions/adr-0002-header-only.md)）。開発専用ツールが consumer の要件になる
  ことはありません。
* **ソースファイル** は [Coding Rules](knowledge/rules/coding.md) の SPDX ライセンスヘッダーで始まり、
  `clang-format` が通ること。
* **CI が green** であること: Linux GCC / Linux Clang / Windows MSVC、いずれも knowledge check 込み。

## テスト

CTest が共通の実行入口です。性質ごとに、それを保証できる最小の仕組みに割り当てます。

* 実行時の振る舞い → GoogleTest
* コンパイル時の性質 → `static_assert`
* positive compile test → 通常の CMake ビルドターゲット
* negative compile test → public API の制約について、拒否されることの検証が必要な場合のみ
* example が正しいままであること → `examples/` のプログラム。同じ `ctest` 実行でビルド・実行されます

全体の切り分けは [Test Strategy](knowledge/testing/test-strategy.md) にあります。

public API を変更するときは [`examples/`](examples/) がコンパイルでき、内容も実態に合っている状態を
保ちます。example はドキュメントであってカバレッジを足す場所ではありません。assert する価値のある
性質は GoogleTest のケースに書きます。

## knowledge と ADR

設計知識が実際に変わったときに knowledge を更新します。contract も rule も決定も変えない純粋な
リファクタリングでは knowledge の編集は不要で、Pull Request に「不要 — 理由」と書けばそれで十分な回答
です。

ADR を書くのは、書かなければ将来の貢献者がその選択肢を却下した理由を再導出する羽目になるときです。
ADR は Issue / Pull Request / レビューで既に到達した決定を記録するものであり、そこで決定を下す場所では
ありません。規約は [Documentation Rules](knowledge/rules/documentation.md) にあります。

## 未決の問いは Issue へ

変更にあたってどこにも記録されていない設計判断が必要になったら、Pull Request の中で決めずに Issue に
出してください。決まっていない設計判断は、決着するまで Issue と Pull Request 上の議論にあり、既に
決まったことを保持するのが knowledge bundle です。

## AI 支援での貢献

AI の利用は当たり前の開発手段として想定しています — ローカルで Codex、クラウドで Claude Code。どちらも
必須ではなく、agent 専用のワークフローもありません。上のコマンドが人にも agent にも共通の source of
truth です。

agent 固有の要件は 2 つだけです。設計判断を勝手に作らないこと、provenance を推測で埋めず正直に記録する
こと。いずれも
[AI-Assisted Development Rules](knowledge/rules/ai-assisted-development.md) にあります。agent の入口は
[AGENTS.md](AGENTS.md) です。

## Pull Request

小さく保ってください。方向性の相談をしたいなら、早い段階で draft として出すのも歓迎です。

Pull Request template が尋ねるのは、関連 Issue、What / Why、public API の変更、テスト、ドキュメント、
**knowledge / ADR 更新の要否と理由**、breaking change です。本文は日本語でも英語でも構いません。

`CONTRIBUTING.md` を変更した場合は、同じ Pull Request で
[CONTRIBUTING.ja.md](CONTRIBUTING.ja.md)（このファイル）も更新してください。

## ライセンス

Precept は [Apache License 2.0](LICENSE) で提供されます。明示的に別段の意思表示をしない限り、Precept へ
取り込まれることを意図して提出された貢献物は、ライセンス第 5 条のとおり Apache-2.0 の下でライセンスされます。
第 5 条は、ライセンサーとの間で別途締結した契約を上書きしません。本プロジェクトは現時点で独自の CLA や DCO を
必要としていません。ライセンス選択の理由は
[ADR-0003](knowledge/decisions/adr-0003-apache-2-0.md) にあります。
