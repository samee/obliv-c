#include<waksman.h>
#include<stdlib.h>
#include<stdio.h> // XXX

size_t waksmanSwapCount(size_t n)
{
  if(n<=1) return 0;
  if(n==2) return 1;
  return n-1+waksmanSwapCount(n/2)+waksmanSwapCount((n+1)/2);
}

// Creates network. Output array lengths should be at least
// waksmanSwapCount(n). Returns this size waksmanSwapCount(n)
// If conventions here change, waksmanSwitches must also be changed
size_t waksmanNetwork(unsigned a[],unsigned b[],size_t n)
{
  size_t i,j=0,jd;
  if(n<=1) return 0;
  for(i=0;i<n-1;i+=2) 
  { a[j]=i;
    b[j]=i+1;
    j++;
  }
  // map1: 2*i -> i -> 2*i for i in [0,n/2]
  jd=waksmanNetwork(a+j,b+j,n/2);
  for(i=0;i<jd;++i) { a[i+j]*=2; b[i+j]*=2; }
  j+=jd;
  // map2: 2*i+1 -> i -> 2*i+1 for i in [0,n/2]
  //       n-1 ->  n/2  -> n-1
  jd=waksmanNetwork(a+j,b+j,(n+1)/2);
  for(i=0;i<jd;++i) 
  { if(a[i+j]!=n/2) a[i+j]=2*a[i+j]+1; else a[i+j]=n-1;
    if(b[i+j]!=n/2) b[i+j]=2*b[i+j]+1; else b[i+j]=n-1;
  }
  j+=jd;
  for(i=0;i<n-2;i+=2)
  { a[j]=i;
    b[j]=i+1;
    j++;
  }
  return j;
}

// Unfortunately, randomly choosing swaps does not seem to produce uniformly
// random permutation. So we do this off-the-protocol: map a given permutation
// to the corresponding waksmanNetwork switch.
// arr is a permutation of the numbers [0..n-1]
// output is of length waksmanSwapCount(n), tells us if gate i should swap
// If conventions here change, waksmanNetwork must also be changed
size_t waksmanSwitches(const unsigned arr[],unsigned n,bool output[])
{
  if(n<=1) return 0;
  unsigned *inv = malloc(n*sizeof(*arr));
  unsigned *arr1 = malloc((n/2)*sizeof(*arr));
  unsigned *arr2 = malloc(((n+1)/2)*sizeof(*arr));
  bool* outputTail = malloc(((n-1)/2)*sizeof(bool));
  bool* set = calloc(n,sizeof(bool));
  unsigned i,j,used=0,nxtj;
  for(i=0;i<n;++i) inv[arr[i]]=i;

  i=n;
  // output head length == n/2
  while(i-->0) if(!set[i])
  { j=i;
    do
    { unsigned prej=arr[j];
      set[j]=true;
      // go from prej to j through lower box
      if(prej/2<n/2) 
      { output[prej/2]=1-prej%2; // swap A if prej even
        used++;
      }
      arr2[j/2]=prej/2;
      if(j/2<(n-1)/2) outputTail[j/2]=1-j%2;  // swap B if j even

      if(n%2 && prej==n-1) break; // odd "open" case
      unsigned prenj=(prej^1);
      nxtj=inv[prenj];
      set[nxtj]=true;
      // go from prenj to nxtj through upper box
      // swap A already done (shared with above)
      arr1[nxtj/2]=prenj/2;
      if(nxtj/2<(n-1)/2) outputTail[nxtj/2]=nxtj%2; // swap B
      nxtj^=1;
      j=nxtj;
    }while(j/2!=i/2);
  }

  used+=waksmanSwitches(arr1,n/2,output+used);
  used+=waksmanSwitches(arr2,(n+1)/2,output+used);
  for(i=0;i<(n-1)/2;++i) output[used++]=outputTail[i];

  free(set);
  free(outputTail);
  free(arr1);
  free(arr2);
  free(inv);
  return used;
}

