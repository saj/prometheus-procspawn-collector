#include "src/path.h"

#include "thirdparty/utest/utest.h"

#define INPUT    utest_fixture->input
#define OUTPUT   utest_fixture->output
#define EXPECTED utest_fixture->expected

struct case_tmp_file_name {
  sds         input;
  sds         output;
  const char *expected;
};

UTEST_F_SETUP(case_tmp_file_name) {
  INPUT  = sdsnewcap(32);
  OUTPUT = sdsnewcap(32);
}

UTEST_F_TEARDOWN(case_tmp_file_name) {
  OUTPUT = tmp_file_name(INPUT, OUTPUT);
  EXPECT_STREQ(EXPECTED, OUTPUT);
  sdsfree(INPUT);
  sdsfree(OUTPUT);
}

UTEST_F(case_tmp_file_name, bare) {
  INPUT    = sdscat(INPUT, "hello");
  EXPECTED = ".hello~tmp";
}

UTEST_F(case_tmp_file_name, dotbare) {
  INPUT    = sdscat(INPUT, ".hello");
  EXPECTED = ".hello~tmp";
}

UTEST_F(case_tmp_file_name, cwd) {
  INPUT    = sdscat(INPUT, "./hello");
  EXPECTED = ".hello~tmp";
}

UTEST_F(case_tmp_file_name, cwddot) {
  INPUT    = sdscat(INPUT, "./.hello");
  EXPECTED = ".hello~tmp";
}

UTEST_F(case_tmp_file_name, bare_ext) {
  INPUT    = sdscat(INPUT, "hello.ext");
  EXPECTED = ".hello.ext~tmp";
}

UTEST_F(case_tmp_file_name, shallow) {
  INPUT    = sdscat(INPUT, "/hello");
  EXPECTED = "/.hello~tmp";
}

UTEST_F(case_tmp_file_name, deep) {
  INPUT    = sdscat(INPUT, "/foo/bar/hello");
  EXPECTED = "/foo/bar/.hello~tmp";
}

UTEST_F(case_tmp_file_name, parent) {
  INPUT    = sdscat(INPUT, "../hello");
  EXPECTED = "../.hello~tmp";
}

UTEST_F(case_tmp_file_name, empty) {
  INPUT    = sdscat(INPUT, "");
  EXPECTED = "";
}

UTEST_F(case_tmp_file_name, root) {
  INPUT    = sdscat(INPUT, "/");
  EXPECTED = "/";
}

UTEST_F(case_tmp_file_name, dot) {
  INPUT    = sdscat(INPUT, ".");
  EXPECTED = ".";
}

UTEST_F(case_tmp_file_name, dotdot) {
  INPUT    = sdscat(INPUT, "..");
  EXPECTED = "..";
}

UTEST_MAIN();
