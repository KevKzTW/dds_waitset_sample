#include "weather.h"

#define MAX_SAMPLES 1

void read_data(dds_entity_t reader)
{
    int ret,i;
    static Weather_Sensor sample[MAX_SAMPLES];
    void * samples[MAX_SAMPLES];
    dds_sample_info_t info[MAX_SAMPLES];
    uint32_t mask = DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;

    for(i=0;i<MAX_SAMPLES;i++)
        samples[i] = &sample[i];

    ret = dds_take(reader, samples, MAX_SAMPLES, info, mask);
    if (ret >= 0) {
        for(i=0;i<ret;i++) {
	        printf("%s in %s : %f\n",sample[i].type,sample[i].city,sample[i].value);
	    }
    }
}

void check_reader_status(dds_entity_t reader,uint32_t status,const char* sensor_name)
{
    if(status & DDS_SUBSCRIPTION_MATCHED_STATUS) {
        dds_subscription_matched_status_t matched_status;
        dds_get_subscription_matched_status(reader,&matched_status);
        if(matched_status.total_count_change)
            printf("find %s sensor\n",sensor_name);
        else
            printf("lost %s sensor\n",sensor_name);
    }
    if(status & DDS_DATA_AVAILABLE_STATUS) 
        read_data(reader);
}

int main (int argc, char ** argv)
{
    int error,i;  
    dds_entity_t ppant;
    dds_entity_t topic_temp;
    dds_entity_t topic_humidity;
    dds_entity_t subscriber;
    dds_entity_t reader_temp;
    dds_entity_t reader_humidity; 
    dds_entity_t e;
    dds_qos_t * qos;
    dds_waitset_t ws;
    dds_condition_t cond_temp;
    dds_condition_t cond_humidity;
    dds_attach_t wsresults[2];
    dds_duration_t timeout = DDS_SECS(3);
    uint32_t status;

    error = dds_init (argc, argv);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_participant_create (&ppant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_topic_create (ppant, &topic_temp, &Weather_Sensor_desc, "temperature", NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_topic_create (ppant, &topic_humidity, &Weather_Sensor_desc, "humidity", NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_subscriber_create (ppant, &subscriber, NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    qos = dds_qos_create ();
    dds_qset_durability (qos, DDS_DURABILITY_VOLATILE);
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));

    error = dds_reader_create (subscriber, &reader_temp, topic_temp, qos, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
 
    error = dds_reader_create (subscriber, &reader_humidity, topic_humidity, qos, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	
	dds_qos_delete (qos);

    ws = dds_waitset_create ();
    dds_status_set_enabled(reader_temp,DDS_DATA_AVAILABLE_STATUS|DDS_SUBSCRIPTION_MATCHED_STATUS);
    dds_status_set_enabled(reader_humidity,DDS_DATA_AVAILABLE_STATUS|DDS_SUBSCRIPTION_MATCHED_STATUS);
	
    cond_temp = dds_statuscondition_get (reader_temp);
	error = dds_waitset_attach (ws, cond_temp, reader_temp);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	
    cond_humidity = dds_statuscondition_get (reader_humidity);
    error = dds_waitset_attach (ws, cond_humidity, reader_humidity);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    printf("Waiting event for 3 secs\n");
    do {
        error = dds_waitset_wait (ws, wsresults, 2, timeout);
        DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
   
        if(error==0) {
            printf("time out exit\n");
	    break;
        }
   
        for(i=0;i<error;i++) {
            e = wsresults[i];
            error = dds_status_take (e, &status, DDS_DATA_AVAILABLE_STATUS|DDS_SUBSCRIPTION_MATCHED_STATUS);
            DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

            if(e==reader_temp)
                check_reader_status(e,status,"temperature");
            else
                check_reader_status(e,status,"humidity");
        }
    } while(1);
  
    dds_waitset_detach (ws, cond_temp);
    dds_waitset_detach (ws, cond_humidity);
    dds_waitset_delete (ws);

    dds_entity_delete (ppant);
    dds_fini ();
}
