#include "gtest/gtest.h"

#include "zim/zim.h"
#include "zim/archive.h"
#include "../src/zimcheck/checks.h"


TEST(zimfilechecks, test_checksum)
{
    std::string fn = "data/zimfiles/wikibooks_be_all_nopic_2017-02.zim";

    zim::Archive archive(fn);
    ErrorLogger logger;

    test_checksum(archive, logger);

    ASSERT_TRUE(logger.overallStatus());
}

TEST(zimfilechecks, test_metadata)
{
    std::string fn = "data/zimfiles/wikibooks_be_all_nopic_2017-02.zim";

    zim::Archive archive(fn);
    ErrorLogger logger;

    test_metadata(archive, logger);

    ASSERT_TRUE(logger.overallStatus());
}

TEST(zimfilechecks, test_favicon)
{
    std::string fn = "data/zimfiles/wikibooks_be_all_nopic_2017-02.zim";

    zim::Archive archive(fn);
    ErrorLogger logger;

    test_favicon(archive, logger);

    ASSERT_TRUE(logger.overallStatus());
}

TEST(zimfilechecks, test_mainpage)
{
    std::string fn = "data/zimfiles/wikibooks_be_all_nopic_2017-02.zim";

    zim::Archive archive(fn);
    ErrorLogger logger;

    test_mainpage(archive, logger);

    ASSERT_TRUE(logger.overallStatus());
}

TEST(zimfilechecks, test_articles)
{
    std::string fn = "data/zimfiles/wikibooks_be_all_nopic_2017-02.zim";

    zim::Archive archive(fn);
    ErrorLogger logger;
    ProgressBar progress(1);
    EnabledTests all_checks; all_checks.enableAll();
    test_articles(archive, logger, progress, all_checks);

    ASSERT_TRUE(logger.overallStatus());
}

class CapturedStdout
{
  std::ostringstream buffer;
  std::streambuf* const sbuf;
public:
  CapturedStdout()
    : sbuf(std::cout.rdbuf())
  {
    std::cout.rdbuf(buffer.rdbuf());
  }

  CapturedStdout(const CapturedStdout&) = delete;

  ~CapturedStdout()
  {
    std::cout.rdbuf(sbuf);
  }

  operator std::string() const { return buffer.str(); }
};

class CapturedStderr
{
  std::ostringstream buffer;
  std::streambuf* const sbuf;
public:
  CapturedStderr()
    : sbuf(std::cerr.rdbuf())
  {
    std::cerr.rdbuf(buffer.rdbuf());
  }

  CapturedStderr(const CapturedStderr&) = delete;

  ~CapturedStderr()
  {
    std::cerr.rdbuf(sbuf);
  }

  operator std::string() const { return buffer.str(); }
};

int zimcheck (const std::vector<const char*>& args);

const std::string zimcheck_help_message(
  "\n"
  "zimcheck checks the quality of a ZIM file.\n\n"
  "Usage: zimcheck [options] zimfile\n"
  "options:\n"
  "-A , --all             run all tests. Default if no flags are given.\n"
  "-0 , --empty           Empty content\n"
  "-C , --checksum        Internal CheckSum Test\n"
  "-I , --integrity       Low-level correctness/integrity checks\n"
  "-M , --metadata        MetaData Entries\n"
  "-F , --favicon         Favicon\n"
  "-P , --main            Main page\n"
  "-R , --redundant       Redundant data check\n"
  "-U , --url_internal    URL check - Internal URLs\n"
  "-X , --url_external    URL check - External URLs\n"
  "-D , --details         Details of error\n"
  "-B , --progress        Print progress report\n"
  "-J , --json            Output in JSON format\n"
  "-H , --help            Displays Help\n"
  "-V , --version         Displays software version\n"
  "examples:\n"
  "zimcheck -A wikipedia.zim\n"
  "zimcheck --checksum --redundant wikipedia.zim\n"
  "zimcheck -F -R wikipedia.zim\n"
  "zimcheck -M --favicon wikipedia.zim\n"
);

TEST(zimcheck, help)
{
    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(-1, zimcheck({"zimcheck", "-h"}));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(-1, zimcheck({"zimcheck", "-H"}));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(-1, zimcheck({"zimcheck", "--help"}));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
}

TEST(zimcheck, version)
{
    const std::string zimcheck_version = "2.1.1";

    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(0, zimcheck({"zimcheck", "-v"}));
      ASSERT_EQ(zimcheck_version + "\n", std::string(zimcheck_output));
    }
    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(0, zimcheck({"zimcheck", "-V"}));
      ASSERT_EQ(zimcheck_version + "\n", std::string(zimcheck_output));
    }
    {
      CapturedStdout zimcheck_output;
      ASSERT_EQ(0, zimcheck({"zimcheck", "--version"}));
      ASSERT_EQ(zimcheck_version + "\n", std::string(zimcheck_output));
    }
}

TEST(zimcheck, nozimfile)
{
    const std::string expected_stderr = "No file provided as argument\n";
    {
      CapturedStdout zimcheck_output;
      CapturedStderr zimcheck_stderr;
      ASSERT_EQ(-1, zimcheck({"zimcheck"}));
      ASSERT_EQ(expected_stderr, std::string(zimcheck_stderr));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
}

const char GOOD_ZIMFILE[] = "data/zimfiles/good.zim";
const char POOR_ZIMFILE[] = "data/zimfiles/poor.zim";
const char BAD_CHECKSUM_ZIMFILE[] = "data/zimfiles/bad_checksum.zim";

using CmdLineImpl = std::vector<const char*>;
struct CmdLine : CmdLineImpl {
  CmdLine(const std::initializer_list<value_type>& il)
    : CmdLineImpl(il)
  {}
};

std::ostream& operator<<(std::ostream& out, const CmdLine& c)
{
  out << "Test context:\n";
  for ( const auto& a : c )
    out << " " << a;
  out << std::endl;
  return out;
}

const char EMPTY_STDERR[] = "";

void test_zimcheck_single_option(std::vector<const char*> optionAliases,
                                 const char* zimfile,
                                 int expected_exit_code,
                                 const std::string& expected_stdout,
                                 const std::string& expected_stderr)
{
    for ( const char* opt : optionAliases )
    {
        CapturedStdout zimcheck_output;
        CapturedStderr zimcheck_stderr;
        const CmdLine cmdline{"zimcheck", opt, zimfile};
        ASSERT_EQ(expected_exit_code, zimcheck(cmdline)) << cmdline;
        ASSERT_EQ(expected_stderr, std::string(zimcheck_stderr)) << cmdline;
        ASSERT_EQ(expected_stdout, std::string(zimcheck_output)) << cmdline;
    }
}

TEST(zimcheck, integrity_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Verifying ZIM-archive structure integrity..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-i", "-I", "--integrity"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, checksum_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Verifying Internal Checksum..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-c", "-C", "--checksum"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, metadata_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Searching for metadata entries..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-m", "-M", "--metadata"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, favicon_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Searching for Favicon..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-f", "-F", "--favicon"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, mainpage_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Searching for main page..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-p", "-P", "--main"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, article_content_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Verifying Articles' content..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {
          "-0", "--empty",              // Any of these options triggers
          "-u", "-U", "--url_internal", // checking of the article contents.
          "-x", "-X", "--url_external"  // For a good ZIM file there is no
        },                              // difference in the output.
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, redundant_articles_goodzimfile)
{
    const std::string expected_output(
        "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
        "[INFO] Verifying Articles' content..." "\n"
        "[INFO] Searching for redundant articles..." "\n"
        "  Verifying Similar Articles for redundancies..." "\n"
        "[INFO] Overall Test Status: Pass" "\n"
        "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-r", "-R", "--redundant"},
        GOOD_ZIMFILE,
        0,
        expected_output,
        EMPTY_STDERR
    );
}

const std::string ALL_CHECKS_OUTPUT_ON_GOODZIMFILE(
      "[INFO] Checking zim file data/zimfiles/good.zim" "\n"
      "[INFO] Verifying ZIM-archive structure integrity..." "\n"
      "[INFO] Avoiding redundant checksum test (already performed by the integrity check)." "\n"
      "[INFO] Searching for metadata entries..." "\n"
      "[INFO] Searching for Favicon..." "\n"
      "[INFO] Searching for main page..." "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[INFO] Searching for redundant articles..." "\n"
      "  Verifying Similar Articles for redundancies..." "\n"
      "[INFO] Overall Test Status: Pass" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
);

TEST(zimcheck, nooptions_goodzimfile)
{
    CapturedStdout zimcheck_output;
    ASSERT_EQ(0, zimcheck({"zimcheck", GOOD_ZIMFILE}));

    ASSERT_EQ(ALL_CHECKS_OUTPUT_ON_GOODZIMFILE, std::string(zimcheck_output));
}

TEST(zimcheck, all_checks_goodzimfile)
{
    test_zimcheck_single_option(
        {"-a", "-A", "--all"},
        GOOD_ZIMFILE,
        0,
        ALL_CHECKS_OUTPUT_ON_GOODZIMFILE,
        EMPTY_STDERR
    );
}

TEST(zimcheck, invalid_option)
{
    {
      CapturedStdout zimcheck_output;
      CapturedStderr zimcheck_stderr;
      ASSERT_EQ(1, zimcheck({"zimcheck", "-z", GOOD_ZIMFILE}));
      ASSERT_EQ("Unknown option `-z'\n", std::string(zimcheck_stderr));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
}

TEST(zimcheck, invalid_long_option)
{
    {
      CapturedStdout zimcheck_output;
      CapturedStderr zimcheck_stderr;
      ASSERT_EQ(1, zimcheck({"zimcheck", "--oops", GOOD_ZIMFILE}));
      ASSERT_EQ("Unknown option `--oops'\n", std::string(zimcheck_stderr));
      ASSERT_EQ(zimcheck_help_message, std::string(zimcheck_output));
    }
}

TEST(zimcheck, json_goodzimfile)
{
    CapturedStdout zimcheck_output;
    ASSERT_EQ(0, zimcheck({
      "zimcheck",
      "--json",
      "data/zimfiles/good.zim"
    }));

    ASSERT_EQ(
      "{"                                                         "\n"
      "  'zimcheck_version' : '2.1.1',"                           "\n"
      "  'file_name' : 'data/zimfiles/good.zim',"                 "\n"
      "  'file_uuid' : '00000000-0000-0000-0000-000000000000'"    "\n"
      "}" "\n"
      , std::string(zimcheck_output)
    );
}

TEST(zimcheck, bad_checksum)
{
    const std::string expected_output(
      "[INFO] Checking zim file data/zimfiles/bad_checksum.zim" "\n"
      "[INFO] Verifying Internal Checksum..." "\n"
      "  [ERROR] Wrong Checksum in ZIM archive" "\n"
      "[ERROR] Invalid checksum:" "\n"
      "  ZIM Archive Checksum in archive: 00000000000000000000000000000000" "\n"
      "" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-c", "-C", "--checksum"},
        BAD_CHECKSUM_ZIMFILE,
        1,
        expected_output,
        EMPTY_STDERR
    );
}

TEST(zimcheck, metadata_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Searching for metadata entries..." "\n"
      "[ERROR] Missing metadata entries:" "\n"
      "  Title" "\n"
      "  Description" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-m", "-M", "--metadata"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, favicon_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Searching for Favicon..." "\n"
      "[ERROR] Missing favicon:" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-f", "-F", "--favicon"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, mainpage_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Searching for main page..." "\n"
      "[ERROR] Missing mainpage:" "\n"
      "  Main Page Index stored in Archive Header: 4294967295" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-p", "-P", "--main"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, empty_items_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[ERROR] Empty articles:" "\n"
      "  Entry empty.html is empty" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-0", "--empty"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, internal_url_check_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[ERROR] Invalid internal links found:" "\n"
      "  The following links:" "\n"
      "- A/non_existent.html" "\n"
      "(/A/non_existent.html) were not found in article dangling_link.html" "\n"
      "  Found 1 empty links in article: empty_link.html" "\n"
      "  ../../oops.html is out of bounds. Article: outofbounds_link.html" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-u", "-U", "--url_internal"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, external_url_check_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[ERROR] Invalid external links found:" "\n"
      "  http://a.io/pic.png is an external dependence in article external_link.html" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-x", "-X", "--url_external"},
        POOR_ZIMFILE,
        1,
        expected_stdout,
        EMPTY_STDERR
    );
}

TEST(zimcheck, redundant_poorzimfile)
{
    const std::string expected_stdout(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[INFO] Searching for redundant articles..." "\n"
      "  Verifying Similar Articles for redundancies..." "\n"
      "[WARNING] Redundant data found:" "\n"
      "  article1.html and redundant_article.html" "\n"
      "[INFO] Overall Test Status: Pass" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
    );

    test_zimcheck_single_option(
        {"-r", "-R", "--redundant"},
        POOR_ZIMFILE,
        0,
        expected_stdout,
        EMPTY_STDERR
    );
}

const std::string ALL_CHECKS_OUTPUT_ON_POORZIMFILE(
      "[INFO] Checking zim file data/zimfiles/poor.zim" "\n"
      "[INFO] Verifying ZIM-archive structure integrity..." "\n"
      "[INFO] Avoiding redundant checksum test (already performed by the integrity check)." "\n"
      "[INFO] Searching for metadata entries..." "\n"
      "[INFO] Searching for Favicon..." "\n"
      "[INFO] Searching for main page..." "\n"
      "[INFO] Verifying Articles' content..." "\n"
      "[INFO] Searching for redundant articles..." "\n"
      "  Verifying Similar Articles for redundancies..." "\n"
      "[ERROR] Empty articles:" "\n"
      "  Entry empty.html is empty" "\n"
      "[ERROR] Missing metadata entries:" "\n"
      "  Title" "\n"
      "  Description" "\n"
      "[ERROR] Missing favicon:" "\n"
      "[ERROR] Missing mainpage:" "\n"
      "  Main Page Index stored in Archive Header: 4294967295" "\n"
      "[WARNING] Redundant data found:" "\n"
      "  article1.html and redundant_article.html" "\n"
      "[ERROR] Invalid internal links found:" "\n"
      "  The following links:" "\n"
      "- A/non_existent.html" "\n"
      "(/A/non_existent.html) were not found in article dangling_link.html" "\n"
      "  Found 1 empty links in article: empty_link.html" "\n"
      "  ../../oops.html is out of bounds. Article: outofbounds_link.html" "\n"
      "[ERROR] Invalid external links found:" "\n"
      "  http://a.io/pic.png is an external dependence in article external_link.html" "\n"
      "[INFO] Overall Test Status: Fail" "\n"
      "[INFO] Total time taken by zimcheck: 0 seconds." "\n"
);

TEST(zimcheck, nooptions_poorzimfile)
{
    CapturedStdout zimcheck_output;
    ASSERT_EQ(1, zimcheck({"zimcheck", POOR_ZIMFILE}));

    ASSERT_EQ(ALL_CHECKS_OUTPUT_ON_POORZIMFILE, std::string(zimcheck_output));
}

TEST(zimcheck, all_checks_poorzimfile)
{
    test_zimcheck_single_option(
        {"-a", "-A", "--all"},
        POOR_ZIMFILE,
        1,
        ALL_CHECKS_OUTPUT_ON_POORZIMFILE,
        EMPTY_STDERR
    );
}
