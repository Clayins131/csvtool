#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#ifdef __cpp_lib_execution
#include <execution>
#endif

class CSVRow {
public:
    std::vector<std::string> data;

    void readNextRow(std::istream& str) {
        std::string line;
        std::getline(str, line);
        std::stringstream lineStream(line);
        std::string cell;

        data.clear();
        while (std::getline(lineStream, cell, ',')) {
            data.push_back(cell);
        }
    }

    const std::string& operator[](std::size_t index) const {
        return data[index];
    }

    std::size_t size() const {
        return data.size();
    }
};

class CSVReader {
    std::ifstream file;
    std::string headerLine;
    std::vector<std::string> headers;

public:
    CSVReader(const std::string& filename) {
        file.open(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file");
        }
        std::getline(file, headerLine);
        std::stringstream ss(headerLine);
        std::string header;
        while (std::getline(ss, header, ',')) {
            headers.push_back(header);
        }
    }

    std::vector<std::string> getHeaders() const {
        return headers;
    }

    bool readRow(CSVRow& row) {
        if (!file.eof()) {
            row.readNextRow(file);
            return !row.data.empty();
        }
        return false;
    }

    void reset() {
        file.clear();
        file.seekg(0, std::ios::beg);
        std::getline(file, headerLine);
    }
};

class CSVAnalyzer {
    CSVReader& reader;
    std::vector<std::string> headers;

public:
    CSVAnalyzer(CSVReader& r) : reader(r), headers(r.getHeaders()) {}

    std::vector<CSVRow> filter(std::function<bool(const CSVRow&)> predicate) {
        std::vector<CSVRow> result;
        CSVRow row;
        reader.reset();
        while (reader.readRow(row)) {
            if (predicate(row)) {
                result.push_back(row);
            }
        }
        return result;
    }

    void groupByAndAverage(const std::string& groupCol, const std::string& valueCol) {
        int groupIdx = getColumnIndex(groupCol);
        int valueIdx = getColumnIndex(valueCol);
        std::map<std::string, std::vector<double>> groups;

        CSVRow row;
        reader.reset();
        while (reader.readRow(row)) {
            std::string key = row[groupIdx];
            double val = std::stod(row[valueIdx]);
            groups[key].push_back(val);
        }

        for (const auto& [key, values] : groups) {
            double avg = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
            std::cout << key << ": " << avg << "\n";
        }
    }

    void getMaxMin(const std::string& columnName, double& maxVal, double& minVal) {
        int colIdx = getColumnIndex(columnName);
        CSVRow row;
        reader.reset();
        bool first = true;
        while (reader.readRow(row)) {
            double val = std::stod(row[colIdx]);
            if (first) {
                maxVal = minVal = val;
                first = false;
            } else {
                if (val > maxVal) maxVal = val;
                if (val < minVal) minVal = val;
            }
        }
    }

    std::vector<CSVRow> sortByColumn(const std::string& columnName, bool descending = false) {
        int colIdx = getColumnIndex(columnName);
        std::vector<CSVRow> allRows;
        CSVRow row;
        reader.reset();

        while (reader.readRow(row)) {
            allRows.push_back(row);
        }

#ifdef __cpp_lib_execution
        std::sort(std::execution::par, allRows.begin(), allRows.end(), [&](const CSVRow& a, const CSVRow& b) {
#else
        std::sort(allRows.begin(), allRows.end(), [&](const CSVRow& a, const CSVRow& b) {
#endif
            double valA = std::stod(a[colIdx]);
            double valB = std::stod(b[colIdx]);
            return descending ? valA > valB : valA < valB;
        });

        return allRows;
    }

    void exportToCSV(const std::string& filename, const std::vector<CSVRow>& rows) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) throw std::runtime_error("Failed to write to file");

        for (size_t i = 0; i < headers.size(); ++i) {
            outFile << headers[i];
            if (i != headers.size() - 1) outFile << ",";
        }
        outFile << "\n";

        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                outFile << row[i];
                if (i != row.size() - 1) outFile << ",";
            }
            outFile << "\n";
        }
    }

private:
    int getColumnIndex(const std::string& name) const {
        for (size_t i = 0; i < headers.size(); ++i) {
            if (headers[i] == name) return i;
        }
        throw std::runtime_error("Column not found: " + name);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: csvtool <filename.csv> [options]\n";
        return 1;
    }

    try {
        std::string filename = argv[1];
        CSVReader reader(filename);
        CSVAnalyzer analyzer(reader);

        if (argc >= 4 && std::string(argv[2]) == "-max") {
            double maxVal, minVal;
            analyzer.getMaxMin(argv[3], maxVal, minVal);
            std::cout << "Max " << argv[3] << ": " << maxVal << ", Min: " << minVal << "\n";
        } else if (argc >= 4 && std::string(argv[2]) == "-sort") {
            bool desc = (argc >= 5 && std::string(argv[4]) == "desc");
            auto sorted = analyzer.sortByColumn(argv[3], desc);
            analyzer.exportToCSV("sorted_output.csv", sorted);
            std::cout << "Sorted data exported to sorted_output.csv\n";
        } else {
            std::cout << "No operation specified or unsupported args.\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
