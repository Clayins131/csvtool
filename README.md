# csvtool - Lightweight CSV Data Analyzer in C++

A simple and efficient command-line tool for analyzing large-scale CSV files using C++. Supports filtering, sorting, statistical analysis, and exporting results.

## 🔧 Features

- Stream reading for large CSV files
- Basic data operations: max/min, sorting, grouping, averaging
- Export processed results to a new CSV
- Command-line interface
- Written in C++17 using STL

## 📦 Build

### With g++

```bash
g++ -std=c++17 -O2 -o csvtool src/csvtool.cpp
```

### With CMake

```bash
mkdir build && cd build
cmake ..
make
```

## 🚀 Usage

```bash
./csvtool <filename.csv> [options]
```

### Options

| Option         | Description                                  |
|----------------|----------------------------------------------|
| `-max <col>`   | Show max and min of a numeric column         |
| `-sort <col>`  | Sort the CSV by a column (asc by default)    |
| `-sort <col> desc` | Sort descending                        |

### Example

```bash
./csvtool data.csv -max Salary
./csvtool data.csv -sort Age desc
```

## 📄 License

This project is licensed under the MIT License.
