#ifndef PPC_CONFIG_H
#define PPC_CONFIG_H

#ifndef PPC_PROG_NAME
#define PPC_PROG_NAME "prometheus-procspawn-collector"
#endif

#ifndef PPC_COLLECTOR_DIRECTORY
#define PPC_COLLECTOR_DIRECTORY "."
#endif

#ifndef PPC_COLLECTOR_FILE_NAME_SUFFIX
#define PPC_COLLECTOR_FILE_NAME_SUFFIX ".prom"
#endif

#ifndef PPC_METRIC_PREFIX
#define PPC_METRIC_PREFIX "procspawn_"
#endif

#ifndef PPC_JOB_LABEL_NAME
#define PPC_JOB_LABEL_NAME "collector_job"
#endif

#endif // ifndef PPC_CONFIG_H
