/*******************************************/
/*** Copyright from 2016 vbextreme       ***/
/*** License view file in this directory ***/
/*******************************************/

#ifndef __OPTEX_H__
#define __OPTEX_H__

typedef enum{ ARGDEF_NOARG, ARGDEF_SIGNED, ARGDEF_UNSIGNED, ARGDEF_DOUBLE, ARGDEF_STR} argdef_e;

typedef struct argdef{
	int hasset;
	char vshort;
	char* vlong;
	argdef_e typeParam;
	void* autoset;
	char* descript;
}argdef_s;

int opt_parse(argdef_s* args, char** argv, int argc);
void opt_errno(int* ac, int* sub);
void opt_usage(argdef_s* args, char* argv0);


#endif

