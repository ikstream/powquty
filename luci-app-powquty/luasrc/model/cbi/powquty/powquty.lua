
m = Map("powquty", "General Configuration")
s = m:section(TypedSection, "powquty", "powquty", "Configuration")

s:tab("general", "General Configuration", translate("PowQuty General Configuration Tab"))
s:tab("event_log", "Event Log Configuration", translate("PowQuty Event Log Configuration Tab"))
s:tab("mqtt_conf", "MQTT Configuration", "")
s:tab("slack_conf", "Slack Configuration", "")
s:tab("mail_conf", "Email Configuration", "")

device_tty = s:taboption("general", Value, "device_tty", "Device tty", "The path to the tty device created by cdc-acm driver")
device_tty.datatype = "string"
device_tty.default = "/dev/ttyACM0"

mqtt_host = s:taboption("mqtt_conf", Value, "mqtt_host", "mqtt_host", "IP-address or URL to the MQTT broker who receives the publish messages of powqutd")
mqtt_host.datatype = "string"
mqtt_host.default = "localhost"

mqtt_topic = s:taboption("mqtt_conf", Value, "mqtt_topic", "mqtt_topic", "The topic under which powquty will publish the mesurement results. The Format is the following: device_uuid,timestamp(sec),timestamp(millisec),3,RMS_Voltage,RMS_Frequency,H3,H5,H7,H9,H11,H13,H15")
mqtt_topic.datatype = "string"
mqtt_topic.default = "devices/update"

dev_uuid = s:taboption("general", Value, "dev_uuid", "dev_uuid", "The dev_uuid sets the device name used in the MQTT-publish messages")
dev_uuid.datatype = "string"
dev_uuid.default = "BERTUB001"

powquty_path = s:taboption("general", Value, "powquty_path", "powquty_path", "The path of the output logfile.")
powquty_path.datatype = "string"
powquty_path.default = "/tmp/powquty.log"

powqutyd_print = s:taboption("general", Flag, "powqutyd_print", "powqutyd_print", "If activated, will print the results published to the MQTT broker to stdout")
powqutyd_print.rmempty = false
powqutyd_print.default = true

max_log_size_kb = s:taboption("general", Value, "max_log_size_kb", "max_log_size_kb", "Maximum size of log files in kB")
max_log_size_kb.datatype = "string"
max_log_size_kb.default = "4096"


-- m:chain("powquty_event_log")

-- m1 = Map("powquty_event_log", "Powquty Event Log")
-- local s1 = m:section(NamedSection, "powquty_event_log", "powquty event log" )
-- s1.title = translate("Event Log Configuration")

powquty_email = s:taboption("mail_conf", Flag, "powquty_email", "powquty_email", "If activated, powquty will send notifications via email")
powquty_email.rmempty = false
powquty_email.default = false

powquty_adress = s:taboption("mail_conf", Value, "powquty_adress", "powquty_adress", "Email adress to use")
powquty_adress.datatype = "string"
powquty_adress.default = ""

powquty_subject = s:taboption("mail_conf", Value, "powquty_subject", "powquty_subject", "Subject prefix")
powquty_subject.datatype = "string"
powquty_subject.default = "[EN50160-Event]"

mqtt_eventflag = s:taboption("mqtt_conf", Flag, "mqtt_eventflag", "mqtt_eventflag", "If activated, powquty will send mqtt notification on EN50160 Event")
mqtt_eventflag.rmempty = false
mqtt_eventflag.default = true
mqtt_eventflag.enabled = 1

mqtt_eventhost = s:taboption("mqtt_conf", Value, "mqtt_eventhost", "mqtt_eventhost", "IP-address or URL to the MQTT broker who receives the publish messages of powqutd")
mqtt_eventhost.datatype = "string"
mqtt_eventhost.default = "localhost"

mqtt_eventtopic = s:taboption("mqtt_conf", Value, "mqtt_eventtopic", "mqtt_eventtopic", "The topic powquty publishes EN50160 events to")
mqtt_eventtopic.datatype = "string"
mqtt_eventtopic.default = "device/en50160-event"

powquty_slack = s:taboption("slack_conf", Flag, "powquty_slack", "powquty_slack", "If activated, powquty will send notifications to a slack channel")
powquty_slack.rmempty = false
powquty_slack.default = false

slack_webhook = s:taboption("slack_conf", Value, "slack_webhook", "slack_webhook", "Incoming webhook for slack")
slack_webhook.datatype = "string"
slack_webhook.default = ""

slack_user = s:taboption("slack_conf", Value, "slack_user", "slack_user", "The user name to use for messages")
slack_user.datatype = "string"
slack_user.default = "PowQuty_novio"

slack_channel = s:taboption("slack_conf", Value, "slack_channel", "slack_channel", "Post to this channel")
slack_channel.datatype = "string"
slack_channel.default = "#general"

return m

