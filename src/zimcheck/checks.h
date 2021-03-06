#ifndef _ZIM_TOOL_ZIMFILECHECKS_H_
#define _ZIM_TOOL_ZIMFILECHECKS_H_

#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../progress.h"

namespace zim {
  class Archive;
}

enum StatusCode : int {
   PASS = 0,
   FAIL = 1,
   EXCEPTION = 2
};

enum class LogTag { ERROR, WARNING };

// Specialization of std::hash needed for our unordered_map. Can be removed in c++14
namespace std {
  template <> struct hash<LogTag> {
    size_t operator() (const LogTag &t) const { return size_t(t); }
  };
}

static std::unordered_map<LogTag, std::string> tagToStr{ {LogTag::ERROR,     "ERROR"},
                                                         {LogTag::WARNING,   "WARNING"}};

enum class TestType {
    CHECKSUM,
    INTEGRITY,
    EMPTY,
    METADATA,
    FAVICON,
    MAIN_PAGE,
    REDUNDANT,
    URL_INTERNAL,
    URL_EXTERNAL,
    MIME,
    OTHER
};

// Specialization of std::hash needed for our unordered_map. Can be removed in c++14
namespace std {
  template <> struct hash<TestType> {
    size_t operator() (const TestType &t) const { return size_t(t); }
  };
}

static std::unordered_map<TestType, std::pair<LogTag, std::string>> errormapping = {
    { TestType::CHECKSUM,      {LogTag::ERROR, "Invalid checksum"}},
    { TestType::INTEGRITY,     {LogTag::ERROR, "Invalid low-level structure"}},
    { TestType::EMPTY,         {LogTag::ERROR, "Empty articles"}},
    { TestType::METADATA,      {LogTag::ERROR, "Missing metadata entries"}},
    { TestType::FAVICON,       {LogTag::ERROR, "Missing favicon"}},
    { TestType::MAIN_PAGE,     {LogTag::ERROR, "Missing mainpage"}},
    { TestType::REDUNDANT,     {LogTag::WARNING, "Redundant data found"}},
    { TestType::URL_INTERNAL,  {LogTag::ERROR, "Invalid internal links found"}},
    { TestType::URL_EXTERNAL,  {LogTag::ERROR, "Invalid external links found"}},
    { TestType::MIME,       {LogTag::ERROR, "Incoherent mimeType found"}},
    { TestType::OTHER,      {LogTag::ERROR, "Other errors found"}}
};

class ErrorLogger {
  private:
    std::unordered_map<TestType, std::vector<std::string>> reportMsgs;
    std::unordered_map<TestType, bool> testStatus;

  public:
    ErrorLogger()
    {
        for (const auto &m : errormapping) {
            testStatus[m.first] = true;
        }
    }

    void setTestResult(TestType type, bool status) {
        testStatus[type] = status;
    }

    void addReportMsg(TestType type, const std::string& message) {
        reportMsgs[type].push_back(message);
    }

    void report(bool error_details) const {
        for (auto testmsg : reportMsgs) {
                auto &p = errormapping[testmsg.first];
                std::cout << "[" + tagToStr[p.first] + "] " << p.second << ":" << std::endl;
                for (auto& msg: testmsg.second) {
                    std::cout << "  " << msg << std::endl;
                }
        }
    }

    inline bool overalStatus() const {
        return std::all_of(testStatus.begin(), testStatus.end(),
                           [](std::pair<TestType, bool> e){
                                    if (errormapping[e.first].first == LogTag::ERROR)
                                    {
                                        return e.second; //return the test status result
                                    }
                                    return true;
                            });
    }
};


void test_checksum(zim::Archive& archive, ErrorLogger& reporter);
void test_integrity(const std::string& filename, ErrorLogger& reporter);
void test_metadata(const zim::Archive& archive, ErrorLogger& reporter);
void test_favicon(const zim::Archive& archive, ErrorLogger& reporter);
void test_mainpage(const zim::Archive& archive, ErrorLogger& reporter);
void test_articles(const zim::Archive& archive, ErrorLogger& reporter, ProgressBar progress,
                   bool redundant_data, bool url_check, bool url_check_external, bool empty_check);

#endif
