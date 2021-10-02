#include "src/label.h"

#include "thirdparty/utest/utest.h"

UTEST(label, value_escape_backslash) {
  sds buf = ppc_label_escape_value("foo\\bar", sdsempty());
  EXPECT_STREQ("foo\\\\bar", buf);
  sdsfree(buf);
}

UTEST(label, value_escape_double_quote) {
  sds buf = ppc_label_escape_value("foo\"bar", sdsempty());
  EXPECT_STREQ("foo\\\"bar", buf);
  sdsfree(buf);
}

UTEST(label, value_escape_newline) {
  sds buf = ppc_label_escape_value("foo\nbar", sdsempty());
  EXPECT_STREQ("foo\\nbar", buf);
  sdsfree(buf);
}

UTEST(label, split) {
  struct ppc_label l = ppc_label_split("foo=bar");

  EXPECT_STREQ("foo", l.name);
  EXPECT_STREQ("bar", l.value);

  ppc_label_free(l);
}

UTEST(label, split_value_contains_equals) {
  struct ppc_label l = ppc_label_split("foo=bar=baz");

  EXPECT_STREQ("foo", l.name);
  EXPECT_STREQ("bar=baz", l.value);

  ppc_label_free(l);
}

UTEST(label, split_missing_value) {
  struct ppc_label l = ppc_label_split("foo=");

  EXPECT_STREQ("foo", l.name);
  EXPECT_STREQ("", l.value);

  ppc_label_free(l);
}

UTEST(label, split_missing_separator) {
  struct ppc_label l = ppc_label_split("foo");

  EXPECT_STREQ("foo", l.name);
  EXPECT_STREQ("", l.value);

  ppc_label_free(l);
}

UTEST(label, append_zero) {
  sds          buf = sdsempty();
  ppc_labelset ls  = ppc_labelset_alloc();

  buf = ppc_labelset_format(ls, buf);

  EXPECT_STREQ("{}", buf);

  ppc_labelset_free_all(ls);
  sdsfree(buf);
}

UTEST(label, append_one) {
  sds          buf = sdsempty();
  ppc_labelset ls  = ppc_labelset_alloc();

  ls  = ppc_labelset_append(ls, ppc_label_new("foo", "bar"));
  buf = ppc_labelset_format(ls, buf);

  EXPECT_STREQ("{foo=\"bar\"}", buf);

  ppc_labelset_free_all(ls);
  sdsfree(buf);
}

UTEST(label, append_many) {
  sds          buf = sdsempty();
  ppc_labelset ls  = ppc_labelset_alloc();

  ls  = ppc_labelset_append(ls, ppc_label_new("svy", "wvd"));
  ls  = ppc_labelset_append(ls, ppc_label_new("epj", "ydj"));
  ls  = ppc_labelset_append(ls, ppc_label_new("doy", "vbc"));
  ls  = ppc_labelset_append(ls, ppc_label_new("ahr", "otn"));
  ls  = ppc_labelset_append(ls, ppc_label_new("qbs", "tnm"));
  ls  = ppc_labelset_append(ls, ppc_label_new("hcr", "clk"));
  ls  = ppc_labelset_append(ls, ppc_label_new("tog", "zpb"));
  ls  = ppc_labelset_append(ls, ppc_label_new("hgu", "syz"));
  ls  = ppc_labelset_append(ls, ppc_label_new("sfw", "yga"));
  ls  = ppc_labelset_append(ls, ppc_label_new("szl", "npo"));
  ls  = ppc_labelset_append(ls, ppc_label_new("qnd", "ioe"));
  ls  = ppc_labelset_append(ls, ppc_label_new("mlp", "bnc"));
  ls  = ppc_labelset_append(ls, ppc_label_new("buz", "yaz"));
  ls  = ppc_labelset_append(ls, ppc_label_new("eab", "uih"));
  ls  = ppc_labelset_append(ls, ppc_label_new("ani", "hwy"));
  ls  = ppc_labelset_append(ls, ppc_label_new("ahy", "fya"));
  ls  = ppc_labelset_append(ls, ppc_label_new("lnj", "dzc"));
  ls  = ppc_labelset_append(ls, ppc_label_new("wgq", "guc"));
  ls  = ppc_labelset_append(ls, ppc_label_new("reb", "bhp"));
  ls  = ppc_labelset_append(ls, ppc_label_new("bes", "auf"));
  buf = ppc_labelset_format(ls, buf);

  EXPECT_STREQ(
      "{ahr=\"otn\",ahy=\"fya\",ani=\"hwy\",bes=\"auf\",buz=\"yaz\",doy=\"vbc\",eab=\"uih\",epj="
      "\"ydj\",hcr=\"clk\",hgu=\"syz\",lnj=\"dzc\",mlp=\"bnc\",qbs=\"tnm\",qnd=\"ioe\",reb=\"bhp\","
      "sfw=\"yga\",svy=\"wvd\",szl=\"npo\",tog=\"zpb\",wgq=\"guc\"}",
      buf);

  ppc_labelset_free_all(ls);
  sdsfree(buf);
}

UTEST_MAIN();
