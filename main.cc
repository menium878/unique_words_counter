#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <sstream>

std::set<std::string> global_words;
std::mutex words_mutex;


void process_chunk(const std::string& chunk) {
    std::istringstream ss(chunk);
    std::string word;


    std::set<std::string> local_words;

    while (ss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        local_words.insert(word);
    }

    std::lock_guard<std::mutex> guard(words_mutex);
    global_words.insert(local_words.begin(), local_words.end());
}

void process_file(const std::string& filename, int num_threads) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filename << '\n';
        return;
    }

    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::streamsize chunk_size = file_size / num_threads;

    std::vector<std::thread> threads;
    std::string chunk;

    for (int i = 0; i < num_threads; ++i) {
        std::string buffer;
        buffer.reserve(chunk_size);

        if (i != num_threads - 1) {
            buffer.resize(chunk_size);
            file.read(&buffer[0], chunk_size);

            char c;
            while (file.get(c) && c != ' ') {
                buffer += c;
            }
        } else {
            buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }

        threads.emplace_back(process_chunk, buffer);
    }

    for (auto& t : threads) {
        t.join();
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Please provide file as argument: " << argv[0] << " <filename>" << '\n';
        return 1;
    }

    std::string filename = argv[1];
    int num_threads = std::thread::hardware_concurrency();


    process_file(filename, num_threads);


    std::cout << global_words.size() << '\n';

    return 0;
}

