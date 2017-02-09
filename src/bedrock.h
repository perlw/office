#ifndef __BEDROCK_H__
#define __BEDROCK_H__

int bedrock_init();
void bedrock_kill();

void bedrock_swap();
void bedrock_poll();
int bedrock_should_close();

#endif // __BEDROCK_H__
