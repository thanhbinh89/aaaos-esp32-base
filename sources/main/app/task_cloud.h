#ifndef __APP_CLOUD_H__
#define __APP_CLOUD_H__

extern void TaskCloudEntry(void *);
extern void sendStaticMsgToServer(const char *topic, const char *data);
extern void sendDynamicMsgToServer(char *topic, char *data, int dataLen);

#endif
