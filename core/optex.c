/*******************************************/
/*** Copyright from 2016 vbextreme       ***/
/*** License view file in this directory ***/
/*******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optex.h"

static int errno_argc;
static int errno_subarg;

static void opt_reset(argdef_s* args){
	size_t el = 0;
	for(; args[el].vshort; ++el){
		args[el].hasset = 0;
	}
}

static int opt_type(char* opt){
	if( opt[0] == '-' ){
		if( opt[1] == '-' ){
			return 2;
		}
		return 1;
	}
	return 0;
}

static int opt_find_long(argdef_s* args, char* opt){
	int el = 0;
	for(; args[el].vshort; ++el){
		if( !strcmp(args[el].vlong, opt) ){
			return el;
		}
	}
	return -1;
}

static int opt_find_short(argdef_s* args, char opt){
	int el = 0;
	for(; args[el].vshort; ++el){
		if( args[el].vshort == opt ){
			return el;
		}
	}
	return -1;
}

static int opt_as_signed(long* ptr, char* argv){
	char* en;
	*ptr = strtol(argv, &en, 10);
	return ( *en != 0 ) ? -1 : 1;
}

static int opt_as_unsigned(unsigned long* ptr, char* argv){
	char* en;
	*ptr = strtoul(argv, &en, 10);
	return ( *en != 0 ) ? -1 : 1;
}

static int opt_as_double(double* ptr, char* argv){
	char* en;
	*ptr = strtod(argv, &en);
	return ( *en != 0 ) ? -1 : 1;
}

static int opt_arg_set(argdef_s* arg, int itarg, char** argv, int argc){
	arg->hasset = 1;
	switch( arg->typeParam ){
		case ARGDEF_NOARG:
		return 0;
		
		case ARGDEF_SIGNED:
			if( itarg >= argc ) return -1;
		return opt_as_signed(arg->autoset, argv[itarg]);
		
		case ARGDEF_UNSIGNED:
			if( itarg >= argc ) return -1;
		return opt_as_unsigned(arg->autoset, argv[itarg]);
		
		case ARGDEF_DOUBLE:
			if( itarg >= argc ) return -1;
		return opt_as_double(arg->autoset, argv[itarg]);

		case ARGDEF_STR:
			if( itarg >= argc ) return -1;
			arg->autoset = argv[itarg];
		return 1;
	}
	return -1;
}

static int opt_parse_short(argdef_s* args, int itarg, char** argv, int argc){
	int skipper = 1;
	char* opt = argv[itarg];
	for(++opt; *opt; ++opt ){
		int el = opt_find_short(args, *opt);
		if( el < 0 ){
			errno_argc = itarg;
			errno_subarg = opt - argv[itarg];
			return -1;
		}

		int ns = opt_arg_set(&args[el], itarg + skipper, argv, argc);
		if( ns < 0 ){
			errno_argc = itarg + skipper;
			errno_subarg = -1;
			return -1;
		}

		skipper += ns;
	}
	return skipper;
}

static int opt_parse_long(argdef_s* args, int itarg, char** argv, int argc){
	int skipper = 1;

	int el = opt_find_long(args, &argv[itarg][2]);
	if( el < 0 ){
		errno_argc = itarg;
		errno_subarg = -1;
		return -1;
	}

	int ns = opt_arg_set(&args[el], itarg + skipper, argv, argc);
	if( ns < 0 ){
		errno_argc = itarg + skipper;
		errno_subarg = -1;
		return -1;
	}

	skipper += ns;
	return skipper;
}

int opt_parse(argdef_s* args, char** argv, int argc){
	int it;
	
	opt_reset(args);
	
	it = 1;
	while( it < argc ){
		int sk;

		switch( opt_type(argv[it]) ){
			case 0: return it;
			
			case 1:
				sk = opt_parse_short(args, it, argv, argc);
			break;

			case 2:
				sk = opt_parse_long(args, it, argv, argc);
			break;

			default: return -1;
		}
		it += sk;
	}

	return it;
}

void opt_errno(int* ac, int* sub){
	*ac = errno_argc;
	*sub = errno_subarg;
}

void opt_usage(argdef_s* args, char* argv0){
	printf("usage %s\n", argv0);
	
	size_t el;
	for(el = 0; args[el].vshort; ++el){
		printf("\t-%c --%s\n\t\t", args[el].vshort, args[el].vlong);
		puts(args[el].descript);
	}
}















