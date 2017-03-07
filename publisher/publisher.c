#include "weather.h"

dds_entity_t ppant;
dds_entity_t topic_temp;
dds_entity_t topic_humidity;
dds_entity_t publisher;
dds_entity_t writer_temp;
dds_entity_t writer_humidity;

void *thread_temp(void *argu)
{
    int error,i;
    Weather_Sensor sample;

    printf("thread_temp create\n");
    dds_thread_init("thread_temp");

    sample.type = "Temperature";
    sample.city = "Taipei";

    for(i=0;i<10;i++) {
        sample.value = 20.0 + 0.1*i;
	
        printf("Temperature in %s : %f\n",sample.city,sample.value);

        dds_write(writer_temp,&sample);
        dds_sleepfor(DDS_MSECS(500));
    }
   
    error = dds_instance_dispose (writer_temp, &sample);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_thread_fini();
    pthread_exit(NULL);  
}

void *thread_humidity(void *argu)
{
    int error,i;
    Weather_Sensor sample;

    printf("thread_humidity create\n");
    dds_thread_init("thread_humidity");

    sample.type = "Humidity";
    sample.city = "Taipei";

    for(i=0;i<10;i++) {
        sample.value = 0.5 + 0.01*i;

        printf("Humidity in %s : %f\n",sample.city,sample.value);

        dds_write(writer_humidity,&sample);
        dds_sleepfor(DDS_MSECS(500));
    }
   
    error = dds_instance_dispose (writer_humidity, &sample);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_thread_fini();
    pthread_exit(NULL);  
}

int main (int argc, char ** argv)
{
    int error,i;   
    dds_entity_t e;
    dds_qos_t * qos;
    dds_waitset_t ws;
    dds_condition_t cond_temp;
    dds_condition_t cond_humidity;
    dds_attach_t wsresults[2];
    dds_duration_t timeout = DDS_SECS(10);
    uint32_t status;
    pthread_t thread_id_temp = 0;
    pthread_t thread_id_humidity = 0;

    error = dds_init (argc, argv);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_participant_create (&ppant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_topic_create (ppant, &topic_temp, &Weather_Sensor_desc, "temperature", NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_topic_create (ppant, &topic_humidity, &Weather_Sensor_desc, "humidity", NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    error = dds_publisher_create (ppant, &publisher, NULL, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    qos = dds_qos_create ();
    dds_qset_durability (qos, DDS_DURABILITY_VOLATILE);
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));

    error = dds_writer_create (publisher, &writer_temp, topic_temp, qos, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
 
    error = dds_writer_create (publisher, &writer_humidity, topic_humidity, qos, NULL);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	
	dds_qos_delete (qos);

    ws = dds_waitset_create ();
    dds_status_set_enabled(writer_temp,DDS_PUBLICATION_MATCHED_STATUS);
    dds_status_set_enabled(writer_humidity,DDS_PUBLICATION_MATCHED_STATUS);
	
    cond_temp = dds_statuscondition_get (writer_temp);
	error = dds_waitset_attach (ws, cond_temp, writer_temp);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
	
    cond_humidity = dds_statuscondition_get (writer_humidity);
    error = dds_waitset_attach (ws, cond_humidity, writer_humidity);
    DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    printf("Waiting publication matched for 10 secs\n");
    do {
        error = dds_waitset_wait (ws, wsresults, 2, timeout);
        DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
   
        if(error==0) {
            printf("time out exit\n");
	        break;
        }

        for(i=0;i<error;i++) {
            e = wsresults[i];
            error = dds_status_take (e, &status, DDS_PUBLICATION_MATCHED_STATUS);
            DDS_ERR_CHECK (error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);          

            if(status & DDS_PUBLICATION_MATCHED_STATUS ) {
               if(e==writer_temp) {
	               dds_status_set_enabled(writer_temp,0);
                   pthread_create(&thread_id_temp,NULL,&thread_temp,NULL);
               }
               else {
                   dds_status_set_enabled(writer_humidity,0);
                   pthread_create(&thread_id_humidity,NULL,&thread_humidity,NULL);
               }
            }

        }
    } while(!thread_id_temp || !thread_id_humidity);

    printf("Waiting thread\n");
    pthread_join(thread_id_temp,NULL);
    pthread_join(thread_id_humidity,NULL);

    dds_waitset_detach (ws, cond_temp);
    dds_waitset_detach (ws, cond_humidity);
    dds_waitset_delete (ws);

    dds_entity_delete (ppant);
    dds_fini ();
}
