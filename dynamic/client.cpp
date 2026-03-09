#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

struct ParseException : std::runtime_error, rapidjson::ParseResult {
  ParseException(rapidjson::ParseErrorCode code, const char *msg, size_t offset)
      : std::runtime_error(msg), rapidjson::ParseResult(code, offset) {}
};

template <typename T> class blocking_queue {
  std::queue<T> q;
  std::mutex m;
  std::condition_variable_any cond;
  bool done = false;

public:
  bool pop(T &popto) {
    std::unique_lock<std::mutex> lock(m);

    cond.wait(lock, [&]() { return done || !(q.empty()); });

    if (q.empty()) {
      return false;
    }

    popto = q.front();
    q.pop();

    return true;
  }

  void push(const T &pushit) {
    std::lock_guard<std::mutex> lock(m);
    q.push(pushit);
    cond.notify_one();
  }

  bool isitdone() { return q.empty() && done; }

  void all_done() {
    std::lock_guard<std::mutex> lock(m);
    done = true;
    cond.notify_all();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(m);
    return q.empty();
  }
};

#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset)                           \
  throw ParseException(code, #code, offset)

#include <chrono>
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

bool debug = false;

// Updated service URL
const string SERVICE_URL =
    "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";
const int MAX_THREADS = 50;

// Function to HTTP ecnode parts of URLs. for instance, replace spaces with
// '%20' for URLs
string url_encode(CURL *curl, string input) {
  char *out = curl_easy_escape(curl, input.c_str(), input.size());
  string s = out;
  curl_free(out);
  return s;
}

// Callback function for writing response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     string *output) {
  size_t totalSize = size * nmemb;
  output->append((char *)contents, totalSize);
  return totalSize;
}

// Function to fetch neighbors using libcurl with debugging
string fetch_neighbors(CURL *curl, const string &node) {

  string url = SERVICE_URL + url_encode(curl, node);
  string response;

  if (debug)
    cout << "Sending request to: " << url << endl;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Verbose Logging

  // Set a User-Agent header to avoid potential blocking by the server
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    cerr << "CURL error: " << curl_easy_strerror(res) << endl;
  } else {
    if (debug)
      cout << "CURL request successful!" << endl;
  }

  // Cleanup
  curl_slist_free_all(headers);

  if (debug)
    cout << "Response received: " << response << endl; // Debug log

  return (res == CURLE_OK) ? response : "{}";
}

// Function to parse JSON and extract neighbors
vector<string> get_neighbors(const string &json_str) {
  vector<string> neighbors;
  try {
    Document doc;
    doc.Parse(json_str.c_str());

    if (doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
      for (const auto &neighbor : doc["neighbors"].GetArray())
        neighbors.push_back(neighbor.GetString());
    }
  } catch (const ParseException &e) {
    std::cerr << "Error while parsing JSON: " << json_str << std::endl;
    throw e;
  }
  return neighbors;
}

// BFS Traversal Function
vector<string> bfs(CURL *curl, const string &start, int depth) {
  struct WorkItem {
    std::string node;
    int depth;
  };
  std::mutex visit;
  std::mutex result_m;

  blocking_queue<WorkItem> q;
  unordered_set<string> visited;
  std::vector<string> result;

  q.push({start, 0});
  visited.insert(start);
  result.push_back(start);

  std::atomic<int> workers{0};
  std::vector<std::thread> threads;

  for (int i = 0; i < MAX_THREADS; i++) {

    threads.emplace_back([&]() {
      CURL *curl = curl_easy_init();
      WorkItem item;

      while (q.pop(item)) {
        workers.fetch_add(1);

        try {
          if (item.depth < depth) {
            std::vector<string> neighbors =
                get_neighbors(fetch_neighbors(curl, item.node));

            for (const auto neighbor : neighbors) {
              bool enqueue = false;

              visit.lock();
              auto inserted = visited.insert(neighbor);
              visit.unlock();

              if (inserted.second) {
                enqueue = true;
              }

              if (enqueue) {
                result_m.lock();
                result.push_back(neighbor);
                result_m.unlock();
                q.push({neighbor, item.depth + 1});
              }
            }
          }
        } catch (const exception &e) {
          cerr << "Worker error while processing \"" << item.node
               << "\": " << e.what() << endl;
        }
        int active = workers.fetch_sub(1) - 1;

        if (q.empty() && active == 0) {
          q.all_done();
        }
      }
      curl_easy_cleanup(curl);
    });
  }

  for (auto &t : threads) {
    t.join();
  }
  return result;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <node_name> <depth>\n";
    return 1;
  }

  string start_node = argv[1]; // example "Tom%20Hanks"
  int depth;
  try {
    depth = stoi(argv[2]);
  } catch (const exception &e) {
    cerr << "Error: Depth must be an integer.\n";
    return 1;
  }

  CURL *curl = curl_easy_init();
  if (!curl) {
    cerr << "Failed to initialize CURL" << endl;
    return -1;
  }

  const auto start{std::chrono::steady_clock::now()};

  for (const auto &node : bfs(curl, start_node, depth))
    cout << "- " << node << "\n";

  const auto finish{std::chrono::steady_clock::now()};
  const std::chrono::duration<double> elapsed_seconds{finish - start};
  std::cout << "Time to crawl: " << elapsed_seconds.count() << "s\n";

  curl_easy_cleanup(curl);

  return 0;
}
