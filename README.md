# VTUフォーマット
## 概要
- VTUフォーマットの機能の１つとして，Bezier曲線を用いた形状の表現が可能である．このプロジェクト内にあるvtu.hをインクルードすることで手軽にBezier曲線を用いた形状の表現が可能となる．　

## 使い方
- vtu.hの中身はなるべく川畑さんが作成したxdmf3.hの使い方に沿うように作成している．
- xdmf3とは異なる点は，gridに関する情報がないことである．代わりに，pvdファイルを作成することで，複数のvtuファイルをまとめて扱うことができる．

|関数名|xdmf3.h|vtu.h|
|:--|:--:|:--:|
|set_geometry|〇|〇|
|set_topology|〇|〇|
|add_attribute|〇|〇|
|add_grid|〇|×|
|pvd_writer|×|〇|
- vtu.hが対応している形状は以下の通りである．
  - ポイント（TopologyType::Vertex）
  - Bezier曲線（TopologyType::BezierCurve）
  - Bezier曲面（TopologyType::BezierQuadrilateral）
  - Bezier体積（TopologyType::BezierHexahedron）

- 使い方の流れとしては，
```c++
vtu::PVDWriter pvd_writer(...);
vtu::VTUWriter writer(...); //ASCII形式かBinary(Appended)形式かを指定する．
writer.set_geometry(...); //Bezier制御点の物理空間の座標を指定する．
writer.set_topology(...); //要素数，曲線，曲面，体積のどれか，次数を指定する．
writer.add_attribute(...);//Bezier制御点の物理量を指定する．（外力，成長量，応力，重みなど）
writer.write();//vtuファイルを出力する．
pvd_writer.write(...);//pvdファイルを出力する．
```
## class vtu::Writerについて
- vtu.hの中で定義されているクラスである．
```c++
Writer(const fs::path& filename, Format format)
```
- 第一引数には出力するvtuファイルのパスを指定する．ファイル名だけでなく，ディレクトリも含めて指定する必要がある．
- 第二引数には出力するvtuファイルの形式を指定する．ASCII形式かBinary(Appended)形式かを指定する．
```c++
  vtu::Format::ASCII
```
```c++
  vtu::Format::Appended
```
- ３つの関数
`set_geometry`
`set_topology`
`add_attribute`
を呼び出すことで，vtuファイルに必要な情報を格納する．
情報を全て格納した後に，write関数を呼び出すことで，vtuファイルを出力する．

## set_geometryについて
- Writerのメンバ関数である．
```c++
  void set_geometry(const double* pos, size_t num_points)
```
- 第一引数にはBezier化した制御点の座標が格納された配列の先頭ポインタが渡される．
もし，Double3型といった独自のクラスの配列の場合は，double型の配列にキャストする必要がある．
[x1, y1, z1, x2, y2, z2, ...]のような形で格納されている必要がある．
もし，[x1, x2, x3, ..., y1, y2, y3, ..., z1, z2, z3, ...]のような形で格納されている場合は，vtu.hの中身を変更するか，output用の配列だけでも[x1, y1, z1, x2, y2, z2, ...]のような形で作成することを推奨する．
- 第二引数にはBezier化した制御点の総数が渡される．一般的には，
```c++
  num_points = nen * num_elements;
```
- Bezier化した制御点というのは，ある要素（ノットスパン）に対して，ノットインサーションを最大数行うことで得られる制御点の座標のことである．
これにより，ある要素はBezeier化した制御点のBezier関数によって表現することができる．
しかし，ノットインサーションを毎回行う必要はない．
一般的に，ある要素のBezier化した制御点$\bm{P}_e$は，要素のBezier Extraction Operator $\bm{C}_e$とその要素の$IEN$と制御点の座標の配列$\bm{P}$を用いて，以下のように表現することができる．
$$
\bm{P}_{e}[I] = \bm{C}_e^{T}[I][J] \bm{P}[IEN[I]]
$$

## set_topologyについて
- Writerのメンバ関数である．
```c++
  void set_topology(size_t num_cells, TopologyType type, const int* p)
```
- 第一引数には要素の総数が渡される．
- 第二引数には要素の種類が渡される．
TopologyTypeは以下の通りである．
  ```c++
  vtu::TopologyType::Vertex
  ```
  ```c++
  vtu::TopologyType::BezierCurve
  ```
  ```c++
  vtu::TopologyType::BezierQuadrilateral
  ```
  ```c++
  vtu::TopologyType::BezierHexahedron
  ```
- 第三引数には要素の次数が渡される．
これは，int型の配列で要素数は３である．
これも必ずしもint型である必要はないが，独自クラスを使用している場合はint型にキャストする必要がある．

## add_attributeについて
- Writerのメンバ関数である．
```c++
  void add_attribute(const T* data, size_t num_items, AttributeType type, AttributeCenter center, const std::string& name)
```
- 第一引数にはBezier化した制御点の物理量が格納された配列の先頭ポインタが渡される．
もし，Double3型といった独自のクラスの配列の場合は，double型の配列にキャストする必要がある．
[x1, y1, z1, x2, y2, z2, ...]のような形で格納されている必要がある．
もし，[x1, x2, x3, ..., y1, y2, y3, ..., z1, z2, z3, ...]のような形で格納されている場合は，vtu.hの中身を変更するか，output用の配列だけでも[x1, y1, z1, x2, y2, z2, ...]のような形で作成することを推奨する．
- 第二引数にはBezier化した制御点の総数が渡される．一般的には，
```c++
  num_items = nen * num_elements;
```
- 第三引数には物理量の種類を指定する．
```c++
  vtu::AttributeType::Scalar
  ```
  ```c++
  vtu::AttributeType::Vector
  ```
  ```c++
  vtu::AttributeType::Tensor6
  ```
  ```c++
  vtu::AttributeType::Tensor9
  ```
  ```c++
  vtu::AttributeType::Matrix
  ```
  データがその点において何種類の物理量を持つかによって，Scalar, Vector, Tensor6, Tensor9, Matrixのいずれかを指定する．
  順に，1次元，3次元，6次元，9次元である．Matrixは任意の次元を指定できるが，vtu.hの中身を変更する必要がある．
- 第四引数には物理量がCellのどこに定義されているかを指定する．
```c++
  vtu::AttributeCenter::Node
  ```
  ```c++
  vtu::AttributeCenter::Cell
  ```
のどちらかだが，基本的にはNodeでよい．
- 第五引数にはその物理量をなんという名前で出力するかを指定する．xdmf3.hはスペースが許容されているが，vtu.hはスペースが許されていないので，パスカルケースかキャメルケースで指定することを推奨する．

- この関数を呼び出した後は，データが格納されたポインタの値が変わってもよい．
出力するデータの種類の数だけメモリを確保するといったことはアホらしいのでやめるように．

## class vtu::PvdWriterについて
- vtu.hの中で定義されているクラスである．
```c++
PvdWriter(const fs::path& pvd_filepath)
```
- 第一引数には出力するpvdファイルのパスを指定する．ファイル名だけでなく，ディレクトリも含めて指定する必要がある．

- メンバ関数のwrite関数を呼び出すことで，pvdファイルを出力する．
```c++
  void write(double time, int patch_id, const std::string& vtu_filename)
```
- 第一引数には時間を指定する．
- 第二引数にはパッチIDを指定する．パッチが一つの場合は０でよい．
- 第三引数には出力するvtuファイルの名前を指定する．

