#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

struct Metric{
    int value;
    double timestamp;
    char metricLabel[100];
};

struct Metric* createMetric(char* metricName, int metricVal, double sensorTs);

void destroyMetric(struct Metric*);

json_object* createMetricJson(struct Metric* metrics[], int);
