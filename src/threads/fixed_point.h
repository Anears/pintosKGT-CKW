#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#define F (1<<14)
#define INT_MAX ((1<<31)-1)
#define INT_MIN (-(1<<31))

#define int_to_fp(n) ((n)*F)
#define fp_to_int_round(x) (((x)>=0?((x)+F/2):((x)-F/2))/F)
#define fp_to_int(x) ((x)/F)
#define add_fp(x,y) ((x)+(y))
#define add_mixed(x,n) ((x)+int_to_fp(n))
#define sub_fp(x,y) ((x)-(y))
#define sub_mixed(x,n) ((x)-int_to_fp(n))
#define mult_fp(x,y) ((int64_t)(x)*(y)/F)
#define mult_mixed(x,n) ((x)*(n))
#define div_fp(x,y) ((int64_t)(x)*F/(y))
#define div_mixed(x,n) ((x)/(n))

/*
int int_to_fp(int n);
int fp_to_int_round(int x);
int fp_to_int(int x);
int add_fp(int x,int y);
int add_mixed(int x,int n);
int sub_fp(int x,int y);
int sub_mixed(int x,int n);
int mult_fp(int x,int y);
int mult_mixed(int x,int y);
int div_fp(int x,int y);
int div_mixed(int x,int n);
*/

#endif