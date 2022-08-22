# Nohの文法について

Nohのコードは複数の関数からなっています。関数は、main関数が存在するならばmain関数が、存在しないならば引数がない関数の中で一番上にあるものが最初に実行されます。
関数の中には文が複数存在する形になっています。

# 型について
## Num型
数。
```
print(123);
var x = 10;
print(x);

print(0123); → エラー
```
## Str型
文字列型。エスケープ文字も使える。
```
print("hoge");
var x = "fuga";
print(x);
print("Hello\nworld");
```
### 今は無いが、将来的に追加したい機能
- 文字列をTuple型として扱う
```
var x = "hoge";
print(x(0)); → "h"
```

## Tuple型
Num型、Str型を複数入れられる。
```
var x = [1, 2, 3, "hoge"];
print(x(0));
print(x(1));
print(x(2));
print(x(3));
```
### 今は無いが、将来的に追加したい機能
- 二次元以上のTupleを作れるようにする
```
var x = [[1, 2], [3, 4]];
print(x(0)(0));
print(x(0)(1));
print(x(1)(0));
print(x(1)(1));
```

## 今は無いが、将来的に追加したい型
- 配列
```
var x: Array(int) = [1, 2, 3];
var y: Array(int) = [1, "hoge"]; → エラー
```
- 細かい整数型
```
var a: Num32 = 1;
var b: Num64 = 2;
var c: uNum32 = 3;
var d: uNum64 = 4;
var e: Float32 = 5.0;
var f: Float64 = 6.0;
```
- Bool型
```
var x: Bool = true;
if x {
	print("OK");
}
```
# 演算子について
複数の演算子があります。
## 単項演算子
- `!`（否定）
- `-`（単項マイナス）
- `~`（ビット反転）
## 二項演算子
- `+`
- `-`
- `*`
- `/`(余り切り捨ての割り算)
- `%`(剰余演算)
- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`
- `&&`
- `||`

## 今は無いが、将来的に追加したい型
- べき乗演算(`^` or `**`)

# 関数について
```
fn main() {
	print("Hello, world!");
}
```
- 引数をつけることもできる
```
fn add(a, b) {
	return a + b;
}
```
## 今は無いが、将来的に追加したい機能
- 型注釈
```
fn add(a: Num, b: Num) -> Num {
	return a + b;
}
```
みたいに書けるようにしたい


# 文について

if文、while文、for文、(再)代入文、関数呼び出しなどが存在します。他にbreak、continue、exit、returnなどのキーワードも使用できます。

## if文
```
if a == 100 {
	print("OK");
}
```
else節をつけることもできます。
```
if a >= 50 {
	print("Yes");
} else {
	print("No");
}
```

## 今は無いが、将来的に追加したい機能
- elif節
```
if a >= 100 {
	print("A");
} elif a >= 50 {
	print("B");
} else {
	print("C");
}
```
- 式としてのif
```
var result = if a >= 100 { "Yes" } else { "No" };
```
- 後置if
```
whlie a <= 100 {
	break if a % 2 == 0;
	a = a + 1;
}
```
```
fn main() {
	var foo = 0;
	var bar = 0;
	return if foo == bar;
}
```

# while文
```
while a <= 100 {
	print(a);
	a = a + 1;
}
```
breakやcontinueなどは他の言語と同じように使える。
# for文
```
for i in 0..10 {
	print(i);
}
```
breakやcontinueなどは他の言語と同じように使える。
## 今はないが、将来的に追加したい機能
- 配列の中身を参照するfor文
```
for i in [0, 1, 2] {
	print(i);
}
```

# (再)代入文
```
var x = 0;
```
再度定義をすることはできないが、再代入ができる
```
var x = 0;
x = 1;
```
再代入のときに違う型の値を代入しようとすると、エラーになる
```
var x = 0;
x = "hoge"; → エラー
```
for、whileの中でスコープが存在する。
```
var x = 0;
for i in 0..5 {
	var x = i;
	print(x);
}
print(x);

→ 0 1 2 3 4 0
```
```
var x = 0;
for i in 0..5 {
	x = i;
	print(x);
}
print(x);

→ 0 1 2 3 4 4
```

## 今はないが、将来的に追加したい機能

- 型注釈
```
var x: Num = 0;
var x: Str = 0; → エラー
```

# ビルトイン関数
## print
コンソールに改行あり出力をする。
```
print(123);
print("hoge");
var x = 10;
print(x);
```
## scanNum
コンソールの入力を受け取って代入する。代入先の変数がNum型でないとエラー。
```
var x = 0;
scanNum(x);
```
## scanStr
コンソールの入力を受け取って代入する。代入先の変数がStr型でないとエラー。
```
var x = "";
scanStr(x);
```