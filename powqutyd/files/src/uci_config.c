/*
 * config.c
 *
 *  Created on: Aug 15, 2016
 *      Author: neez
 */

#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include "uci_config.h"

#define ON 1
#define OFF 0

/** analogous to uci_lookup_option_string from uci.h, returns -1 when not found */
static int uci_lookup_option_int(struct uci_context *uci, struct uci_section *s,
		const char *name) {
	const char* str = uci_lookup_option_string(uci, s, name);
	return str == NULL ? -1 : atoi(str);
}

static long uci_lookup_option_long(struct uci_context *uci,
				   struct uci_section *s,
				   const char *name) {
	const char *str = uci_lookup_option_string(uci, s, name);
	return str == NULL ? -1 : atol(str);
}

int uci_config_powquty(struct powquty_conf* conf) {
	struct uci_context* uci;
	struct uci_package* p;
	struct uci_element* e;
	const char* str;

	/* general powquty config */
	char default_device_tty[MAX_LENGTH] = "/dev/ttyACM0";
	char default_powquty_path[MAX_LENGTH] = "/tmp/powquty.log";
	char default_dev_uuid[MAX_LENGTH] = "BERTUB001";
	int default_powqutyd_print = ON;
	long default_max_log_size_kb = 4096;

	/* regular MQTT handling */
	char default_mqtt_host[MAX_LENGTH] = "localhost";
	char default_mqtt_topic[MAX_LENGTH] = "devices/update";

#ifndef NO_MQTT
	/* MQTT event handling */
	int default_event_flag = ON;
	char default_mqtt_eventhost[MAX_LENGTH] = "localhost";
	char default_mqtt_eventtopic[MAX_LENGTH] = "device/en50160-event";
#endif /* NO_MQTT */

#ifndef NO_SLACK
	/* Slack Config */
	int default_powquty_slack = OFF;
	char default_slack_webhook[WEBHOOK_LENGTH] = "";
	char default_slack_user[MAX_LENGTH] = "PowQuty_novio";
	char default_slack_channel[MAX_LENGTH] = "#general";
#endif /* NO_SLACK */

#ifndef NO_MAIL
	/* email configuration */
	int default_powquty_email = OFF;
	char default_powquty_adress[MAX_LENGTH] = "";
	char default_powquty_subject[MAX_LENGTH] = "[EN50160-Event]";
#endif /* NO_MAIL */

	uci = uci_alloc_context();
	if (uci == NULL)
		return 0;

	if (uci_load(uci, "powquty", &p)) {
		uci_free_context(uci);
		return 0;
	}

	uci_foreach_element(&p->sections, e)
	{
		struct uci_section *s = uci_to_section(e);
		if (strcmp(s->type, "powquty") == 0) {
			/* general */
			strcpy(conf->device_tty, default_device_tty);
			strcpy(conf->dev_uuid, default_dev_uuid);
			strcpy(conf->powquty_path, default_powquty_path);
			conf->powqutyd_print = default_powqutyd_print;
			conf->max_log_size_kb = default_max_log_size_kb;

			/* mqtt */
			strcpy(conf->mqtt_host, default_mqtt_host);
			strcpy(conf->mqtt_topic, default_mqtt_topic);
#ifndef NO_MQTT
			conf->mqtt_event_flag = default_event_flag;
			strcpy(conf->mqtt_eventhost, default_mqtt_eventhost);
			strcpy(conf->mqtt_eventtopic, default_mqtt_eventtopic);
#endif /* NO_MQTT */

#ifndef NO_SLACK
			/* slack */
			conf->powquty_slack = default_powquty_slack;
			strcpy(conf->slack_webhook, default_slack_webhook);
			strcpy(conf->slack_user, default_slack_user);
			strcpy(conf->slack_channel, default_slack_channel);
#endif /* NO_SLACK */

#ifndef NO_MAIL
			/* email */
			conf->powquty_email = default_powquty_email;
			strcpy(conf->powquty_adress, default_powquty_adress);
			strcpy(conf->powquty_subject, default_powquty_subject);
#endif /* NO_EMAIL */

			/* general */
			/* uuid */
			str = uci_lookup_option_string(uci, s, "dev_uuid");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH) {
				continue;
			}
			strcpy(conf->dev_uuid, str);
			printf("looking up dev_uuid: currently ==> %s\n",
				conf->dev_uuid);

			/* device_tty */
			str = uci_lookup_option_string(uci, s, "device_tty");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH) {
				continue;
			}
			strcpy(conf->device_tty, str);
			printf("looking up device_tty currently ==> %s\n",
				conf->device_tty);

			/* powquty_path */
			str = uci_lookup_option_string(uci, s, "powquty_path");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH) {
				continue;
			}
			strcpy(conf->powquty_path, str);
			printf("looking up powquty_path currently ==> %s\n",
				conf->powquty_path);

			/* powquty_print */
			conf->powqutyd_print = uci_lookup_option_int(uci, s,
					"powqutyd_print");

			/* max_log_size_kb */
			conf->max_log_size_kb = uci_lookup_option_long(uci, s,
					"max_log_size_kb");

			/* MQTT */
			/* mqtt_host */
			str = uci_lookup_option_string(uci, s, "mqtt_host");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH) {
				continue;
			}
			strcpy(conf->mqtt_host, str);
			printf("looking up mqtt_host: currently ==> %s\n",
				conf->mqtt_host);

			/* mqtt_topic */
			str = uci_lookup_option_string(uci, s, "mqtt_topic");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH) {
				continue;
			}
			strcpy(conf->mqtt_topic, str);
			printf("looking up mqtt_topic: currently ==> %s\n",
				conf->mqtt_topic);

#ifndef NO_MQTT
			/* mqtt_eventhost */
			str = uci_lookup_option_string(uci, s,
						       "mqtt_eventhost");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->mqtt_eventhost, str);
			printf("looking up mqtt_eventhost: currently ==> %s\n",
				conf->mqtt_eventhost);

			/* mqtt_eventtopic */
			str = uci_lookup_option_string(uci, s,
						       "mqtt_eventtopic");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->mqtt_eventtopic, str);
			printf("looking up mqtt_eventtopic: currently ==> %s\n",
				conf->mqtt_eventtopic);

			/* mqtt_event_flag */
			conf->mqtt_event_flag = uci_lookup_option_int(uci, s,
					"mqtt_event_flag");
#endif /* NO_MQTT */

#ifndef NO_SLACK
			/* slack_webhook */
			str = uci_lookup_option_string(uci, s,
						       "slack_webhook");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->slack_webhook, str);
			printf("looking up slack_webhook: currently ==> %s\n",
				conf->slack_webhook);

			/* slack channel */
			str = uci_lookup_option_string(uci, s,
						       "slack_channel");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->slack_channel, str);
			printf("looking up slack_channel: currently ==> %s\n",
				conf->slack_channel);

			/* slack_user */
			str = uci_lookup_option_string(uci, s,
						       "slack_user");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->slack_user, str);
			printf("looking up slack_user: currently ==> %s\n",
				conf->slack_user);

			/* powquty_slack */
			conf->powquty_slack = uci_lookup_option_int(uci, s,
					      "powquty_slack");
#endif /* NO_SLACK */

#ifndef NO_MAIL
			/* powquty_adress */
			str = uci_lookup_option_string(uci, s,
						       "powquty_adress");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->powquty_adress, str);
			printf("looking up powquty_adress: currently ==> %s\n",
				conf->powquty_adress);

			/* powquty_subject */
			str = uci_lookup_option_string(uci, s,
						       "powquty_subject");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH)
				continue;

			strcpy(conf->powquty_subject, str);
			printf("looking up powquty_subject: currently %s\n",
				conf->powquty_subject);
			conf->powquty_email = uci_lookup_option_int(uci, s,
					      "powquty_email");
#endif /* NO_MAIL */
		}
	}

	uci_unload(uci, p);
	uci_free_context(uci);
	return 0;
}
