#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "thirdparty/sds/sds.h"

#include "src/config.h"
#include "src/error.h"
#include "src/label.h"
#include "src/nanotime.h"
#include "src/path.h"
#include "src/signal.h"

extern char **environ;

static _Noreturn void
usage() {
  fputs("usage: " PPC_PROG_NAME " [flags] name cmd [arg ...]\n", stderr);
  exit(2);
}

#define PPC_OPT_DIRECTORY 1
#define PPC_OPT_LABEL     2

static struct option longopts[] = {
    {"directory", required_argument, NULL, PPC_OPT_DIRECTORY}, //
    {"label", required_argument, NULL, PPC_OPT_LABEL},         //
    {NULL, 0, NULL, 0}                                         //
};

struct ppc_args {
  const char * name;
  int          argc;
  char *const *argv;
  const char * directory;
  ppc_labelset labelset;
};

#define PPC_ARGV_SHIFT(N) \
  do {                    \
    argc -= (N);          \
    argv += (N);          \
  } while (0)

// dies upon any error
static void
parse_args(int argc, char *argv[], struct ppc_args *res) {
  memset(res, '\0', sizeof(struct ppc_args));
  res->directory = PPC_COLLECTOR_DIRECTORY;
  res->labelset  = ppc_labelset_alloc();

  int ch;
  while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
    switch (ch) {
    case PPC_OPT_DIRECTORY:
      if (optarg[0] != '\0') res->directory = optarg;
      break;

    case PPC_OPT_LABEL: {
      sds              bufrepr = sdsempty();
      struct ppc_label label   = ppc_label_split(optarg);

      if (!ppc_label_is_valid_name(label.name)) {
        bufrepr = sdscatrepr(bufrepr, label.name, sdslen(label.name));
        fprintf(stderr, PPC_PROG_NAME ": invalid label name at index %d: %s\n", optind - 1,
                bufrepr);
        sdsfree(bufrepr);
        usage();
      }

      if (!ppc_label_is_valid_value(label.value)) {
        bufrepr = sdscatrepr(bufrepr, label.value, sdslen(label.value));
        fprintf(stderr, PPC_PROG_NAME ": invalid label value at index %d: %s\n", optind - 1,
                bufrepr);
        sdsfree(bufrepr);
        usage();
      }

      res->labelset = ppc_labelset_append(res->labelset, label);
      break;
    }

    default:
      usage();
    }
  }

  PPC_ARGV_SHIFT(optind);
  if (argc < 1) usage();
  res->name = argv[0];

  PPC_ARGV_SHIFT(1);
  if (argc < 1) usage();
  res->argc = argc;
  res->argv = argv;

  res->labelset = ppc_labelset_append(res->labelset, ppc_label_new(PPC_JOB_LABEL_NAME, res->name));
}

static sds
metric_file_name(const char *restrict directory, const char *restrict job_name, sds buf) {
  size_t ld = strlen(directory);
  if (ld) {
    buf = sdscatlen(buf, directory, ld);
  } else {
    buf = sdscatlen(buf, PPC_COLLECTOR_DIRECTORY, sizeof(PPC_COLLECTOR_DIRECTORY) - 1);
  }
  if (!ld || (ld && directory[ld - 1] != '/')) { buf = sdscatlen(buf, "/", 1); }
  buf = sdscatlen(buf, PPC_METRIC_PREFIX, sizeof(PPC_METRIC_PREFIX) - 1);
  buf = sdscat(buf, job_name);
  buf = sdscatlen(buf, PPC_COLLECTOR_FILE_NAME_SUFFIX, sizeof(PPC_COLLECTOR_FILE_NAME_SUFFIX) - 1);
  return buf;
}

struct proc_args {
  int          argc;
  char *const *argv;
};

struct proc_result {
  int      status;
  nanotime started;
  nanotime runtime;
};

static int sigs_wait[] = {
    SIGCHLD,
    // remaining signals are forwarded to child:
    SIGHUP,
    SIGINT,
    SIGQUIT,
    SIGTERM,
    SIGUSR1,
    SIGUSR2,
};

// dies upon any error
static int
wait_child(pid_t child_pid) {
  sigset_t mask;
  sigemptyset(&mask);
  sigset_add(&mask, sigs_wait, sizeof(sigs_wait) / sizeof(int));
  sigprocmask_block_by_sigset(&mask);

  errno = 0;
  if (sigaction(SIGCHLD, // SIG_IGN'd by default
                &(const struct sigaction){
                    .sa_handler = handle_signal_dummy,
                    .sa_flags   = SA_NOCLDSTOP,
                },
                NULL))
    die("sigaction:");

  int rc = 0;
  for (;;) {
    int errnum, sig;
    if ((errnum = sigwait(&mask, &sig))) die_errnum(errnum, "sigwait:");

    int status;
    switch (sig) {
    case SIGCHLD:
      if (waitpid(child_pid, &status, 0) == -1) die("wait:");
      if (WIFEXITED(status)) rc = WEXITSTATUS(status);
      if (WIFSIGNALED(status)) rc = 128 + WTERMSIG(status);
      goto done;
    default:
      if (kill(child_pid, sig)) die("kill:");
    }
  }
done:
  return rc;
}

static bool
path_includes_slash(const char *path) {
  for (const char *c = path; *c; c++) {
    if (*c == '/') return true;
  }
  return false;
}

static int
spawn(pid_t *restrict pid,                                     //
      const char *restrict path,                               //
      const posix_spawn_file_actions_t *restrict file_actions, //
      const posix_spawnattr_t *restrict attrp,                 //
      char *const argv[restrict],                              //
      char *const envp[restrict]) {
  if (path_includes_slash(path)) return posix_spawn(pid, path, file_actions, attrp, argv, envp);
  return posix_spawnp(pid, path, file_actions, attrp, argv, envp);
}

// dies upon any error
static void
spawn_wait(const struct proc_args *args, struct proc_result *res) {
  int               rc, errnum;
  sigset_t          mask_all;
  posix_spawnattr_t attr;
  struct timespec   t1_mono, t2_mono, t1_wall;

  sigfillset(&mask_all);
  if ((errnum = posix_spawnattr_init(&attr))) die_errnum(errnum, "posix_spawnattr_init:");
  if ((errnum = posix_spawnattr_setsigdefault(&attr, &mask_all)))
    die_errnum(errnum, "posix_spawnattr_setsigdefault:");
  if ((errnum = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF)))
    die_errnum(errnum, "posix_spawnattr_setflags:");

  errno = 0;
  if (clock_gettime(CLOCK_REALTIME, &t1_wall)) die("clock_gettime:");
  errno = 0;
  if (clock_gettime(CLOCK_MONOTONIC, &t1_mono)) die("clock_gettime:");

  pid_t pid;
  if ((errnum = spawn(&pid, args->argv[0], NULL, &attr, args->argv, environ)))
    die_errnum(errnum, "spawn: %s:", args->argv[0]);
  rc = wait_child(pid);

  errno = 0;
  if (clock_gettime(CLOCK_MONOTONIC, &t2_mono)) die("clock_gettime:");

  posix_spawnattr_destroy(&attr);

  res->status  = rc;
  res->started = nanotime_from_timespec(&t1_wall);
  res->runtime = nanotime_from_timespec(&t2_mono) - nanotime_from_timespec(&t1_mono);
}

int
main(int argc, char *argv[]) {
  struct ppc_args args;
  parse_args(argc, argv, &args);

  struct proc_result res;
  spawn_wait(&(struct proc_args){.argc = args.argc, .argv = args.argv}, &res);

  sds labelbuf = ppc_labelset_format(args.labelset, sdsnewcap(128));
  sds fname    = metric_file_name(args.directory, args.name, sdsnewcap(64));
  sds tname    = tmp_file_name(fname, sdsnewcap(sdslen(fname) + 8));

  errno   = 0;
  FILE *f = fopen(tname, "w");
  if (f == NULL) die("fopen: %s:", tname);
  fprintf(f, "# TYPE " PPC_METRIC_PREFIX "last_exit_status gauge\n");
  fprintf(f, PPC_METRIC_PREFIX "last_exit_status%s %d\n", labelbuf, res.status);
  fprintf(f, "# TYPE " PPC_METRIC_PREFIX "last_duration_seconds gauge\n");
  fprintf(f, PPC_METRIC_PREFIX "last_duration_seconds%s %.9f\n", labelbuf,
          nanotime_seconds(res.runtime));
  fprintf(f, "# TYPE " PPC_METRIC_PREFIX "last_started_timestamp_seconds gauge\n");
  fprintf(f, PPC_METRIC_PREFIX "last_started_timestamp_seconds%s %.9f\n", labelbuf,
          nanotime_seconds(res.started));
  errno = 0;
  if (fclose(f)) die("fclose:");
  errno = 0;
  if (rename(tname, fname)) {
    unlink(tname);
    die("rename: %s", fname);
  }

  exit(res.status);
}
