#include <stdio.h>
#include <math.h>
#include <float.h>

#ifndef __FISHER_EXACT_TEST__
#define __FISHER_EXACT_TEST__
 
double factorInc(int chi11, int chi12, int chi21, int chi22)
{
  double factor_inc;
  factor_inc = (double)chi12 * chi21;
  factor_inc /= (double)(chi11 + 1) * (chi22 + 1);
  return factor_inc;
}

double factorDec(int chi11, int chi12, int chi21, int chi22)
{
  double factor_dec = (double)chi11 * chi22;
  factor_dec /= (double)(chi21 + 1) * (chi12 + 1);
  return factor_dec;
}
 
double gammln(double xx)
{
  static double cof[6] = { 76.18009172947146, -86.50532032941677, 
			  24.01409824083091, -1.231739572450155,
                          0.1208650973866179e-2, -0.5395239384953e-5 };
  double x, tmp, ser;
  int j;
  x = xx - 1.0;
  tmp = x + 5.5;
  tmp -= (x + 0.5) * log(tmp);
  ser = 1.0;
  for (j = 0; j <= 5; j++) {
    x += 1.0;
    ser += cof[j] / x;
  }
  return -tmp + log(2.50662827465 * ser);
}
 
double factln0(int n)
{
  static double pi=3.1415926536; 
 
  if (n < 0) {
     return 0.0;
  }
  if (n <= 1) return 0.0;
  return 0.5*log((2.0*n+1./3.)*pi)+n*(log(1.0*n)-1.0);
}
 
double factln(int n)
{
  static double a[101];
  if (n < 0) {
     return 0.0;
  }
  if (n <= 1) return 0.0;
  if (n <= 100) return a[n] ? a[n] : (a[n] = gammln((double)(n + 1.0)));
  else return gammln((double)(n + 1.0));
}
 
double binomialln(int n, int k)
{
  return (factln(n) - factln(k) - factln(n - k));
 
}
 
double calc_hypergeom(int chi11, int chi12, int chi21, int chi22)
{
   static double choose0;//, choose1, choose2, choose3;
   static double b1, b2, b3;
   static int total;
   total = chi11 + chi12 + chi21 + chi22;
   b1 = binomialln(chi11 + chi12, chi11);
   b2 = binomialln(chi21 + chi22, chi21);
   b3 = binomialln(total, chi11 + chi21);
   choose0=exp(b1+b2-b3);
   return (choose0);
}
 
double fisher_exact_tiss(int chi11, int chi12, int chi21, int chi22)
{
   register int co_occ, total_libs;
   static int min_co_occ, max_co_occ;
   static int gene_a, gene_b;
   static double factor_inc, factor_dec;
   static double base_p, curr_p;
   int sign=1;
   co_occ = chi11;
   gene_a = chi11 + chi12;
   gene_b = chi11 + chi21;
   total_libs = chi11 + chi12 + chi21 + chi22;
   min_co_occ = 0;
   if (gene_a + gene_b > total_libs) {
      min_co_occ = gene_a + gene_b - total_libs;
   }
   if (gene_a < gene_b) { max_co_occ = gene_a; }
   else { max_co_occ = gene_b; }
   base_p = calc_hypergeom(chi11, chi12, chi21, chi22);
   if (co_occ == max_co_occ || co_occ == min_co_occ) 
     {
       if(co_occ == max_co_occ) {
	 sign=1;
       }
       else {
	 sign=-1;
       }
     }
   else {
      factor_inc = factorInc(chi11, chi12, chi21, chi22);
      factor_dec = factorDec(chi11, chi12, chi21, chi22);
      curr_p = base_p;
      if (factor_dec < factor_inc) {  sign=-1;
         do {
            curr_p *= factor_dec;
            base_p += curr_p;
            co_occ--;
            chi11--; chi22--;
            chi12++; chi21++;
            factor_dec = factorDec(chi11, chi12, chi21, chi22);
         } while (co_occ > min_co_occ);
      }
      else if (factor_inc < factor_dec) { sign=1;
         do {
            curr_p *= factor_inc;
            base_p += curr_p;
            co_occ++;
            chi11++; chi22++;
            chi12--; chi21--;
            factor_inc = factorInc(chi11, chi12, chi21, chi22);
         } while (co_occ < max_co_occ);
      }
      else {
         base_p = 1.0;
      }
   }
   return base_p*sign;
}

#endif
