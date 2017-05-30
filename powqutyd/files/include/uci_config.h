/*
 * uci_config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef UCI_CONFIG_H_
#define UCI_CONFIG_H_

#define MAX_LENGTH	32
#define WEBHOOK_LENGTH 	512

struct powquty_conf {

	char device_tty[32];
	char mqtt_host[32];
	char mqtt_topic[32];
	char dev_uuid[32];
	char powquty_path[32];
	int powqutyd_print;
	long max_log_size_kb;

#ifndef NO_MQTT
	int event_flag;
	char eventhost[MAX_LENGTH];
	char eventtopic[MAX_LENGTH];
#endif

#ifndef NO_SLACK
	int powquty_slack;
	char slack_webhook[WEBHOOK_LENGTH];
	char slack_user[MAX_LENGTH];
	char slack_channel[MAX_LENGTH];
#endif

#ifndef NO_MAIL
	int powquty_email;
	char powquty_adress[MAX_LENGTH];
	char powquty_subject[MAX_LENGTH];
#endif

/*	option device_tty '/dev/ttyACM0'
		option mqtt_host 'localhost'
		option mqtt_topic 'devices/update'
		option dev_uuid 'BERTUB001'
		option powqutyd_print '1'
		*/
};

int uci_config_powquty(struct powquty_conf*);

#endif /* CONFIG_H_ */
