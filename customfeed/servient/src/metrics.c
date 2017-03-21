#include "metrics.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

struct Metric* createMetric(char* metricName, int metricVal, double sensorTs)
{
    struct Metric* metric = (struct Metric*) malloc(sizeof(struct Metric));
    //struct Metric *metric = malloc(sizeof(struct Metric));
    strcpy(metric->metricLabel, metricName);
    metric->value = metricVal;
    metric->timestamp = sensorTs;
    return metric;
}

void destroyMetric(struct Metric* metric)
{
    free(metric);
}

json_object* createMetricJson(struct Metric* metricList[], int size)
{
    int metricListSize = size;
    int i;
    json_object *metricArray = json_object_new_array();
    json_object *metricJson = json_object_new_object();
    for(i=0; i < metricListSize; i++){
        json_object * jobj = json_object_new_object();
        json_object * jobj_ts = json_object_new_object();
        json_object_object_add(jobj, "key", json_object_new_string(metricList[i]->metricLabel));
        json_object_object_add(jobj, "value", json_object_new_double(metricList[i]->value));
        json_object_object_add(jobj_ts, "millis", json_object_new_double(metricList[i]->timestamp));
        json_object_object_add(jobj, "sensorTs", jobj_ts);
        json_object_array_add(metricArray,jobj);
    }
    json_object_object_add(metricJson,"monitoringMetricList", metricArray);
    return metricJson;
}
