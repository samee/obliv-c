/* C program for merge sort */
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
 
/* Function to merge the two haves arr[l..m] and arr[m+1..r] of array arr[] */
void merge(char arr[45][100], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* create temp arrays */
    char L[n1][100], R[n2][100];
 
    /* Copy data to temp arrays L[] and R[] */
    for(i = 0; i < n1; i++)
        strcpy(L[i], arr[l+i]);
    for(j = 0; j < n2; j++)
        strcpy(R[j], arr[m+1+j]);
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (strcmp(L[i],R[j]) == -1)
        {
            strcpy(arr[k], L[i]);
            i++;
        }
        else
        {
            strcpy(arr[k], R[j]);
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there are any */
    while (i < n1)
    {
        strcpy(arr[k], L[i]);
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there are any */
    while (j < n2)
    {
        strcpy(arr[k], R[j]);
        j++;
        k++;
    }
}
 
/* l is for left index and r is right index of the sub-array
  of arr to be sorted */
void mergeSort(char arr[45][100], int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2; //Same as (l+r)/2, but avoids overflow for large l and h
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);		//line 67
    }
}
 
/* Driver program to test above functions */
main()
{
	char arr[2][100];
	strcpy(arr[0], "bbb");
	strcpy(arr[1], "aaa");
//	printf("%s", arr[0]);
//	printf("%s", arr[1]);
	mergeSort(arr, 0, 1);
	printf("%s", arr[0]);
	printf("%s", arr[1]);
	
//    int arr[] = {12, 11, 13, 5, 6, 7};
//    int arr_size = sizeof(arr)/sizeof(arr[0]);
 
//    printf("Given array is \n");
//    printArray(arr, arr_size);
 
//    mergeSort(arr, 0, arr_size - 1);
 
//    printf("\nSorted array is \n");
//    printArray(arr, arr_size);
   return 0;
}
