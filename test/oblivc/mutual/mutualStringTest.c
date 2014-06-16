#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

void cmp(char* s1, char* s2){
  
}
int main(int argc, char* argv[]){
  char* s1 = "Bob";
  char s2[6] = "Joe";
  char s3[10] = "Ricky";
  char* s4;
  char s5[10];
  char ss1[20][20];
  s4 = s3;//valid for pointer = array
  //s3 = s2; //illegal because s3 is an array and s2 is a pointer
  //s2 = s3; //illegal views s3 as a pointer and s2 as an array
  //can use strcpy for these!!!
  s3[1] ='!';
  s4 = "Bobby!!!";
  //s4[2] = '!'; //SEG FAULT
  s1 = s4; //valid for pointer to pointer
  //s2 = s1; //illegal! views s1 as a pointer
  //s2 = *s1;//illegal! views s1 as a char
  //*s1 = *s2;//creates a segfault
  //s5 = s2; //illegal!
  strcpy(s5, s2);//valid for array to array IFFFFF Array has enough space
  //s4 =s2;
  strcpy(s2, s4);//Valid for array to pointer if array has enough space
  int i;
  for(i=0; s4[i]!='\0'; i++)
    s2[i] = s4[i]; //Does the same thing as the cpy
  strcpy(ss1[0], s1);
  ss1[0][2]='1';
  printf("%s %s %s\n", s1, s2, ss1[0]);
  i = 0;
  strcpy(s3, "Boby");
  strcpy(ss1[1], s3);
  bool b = true;
  int c = 17;
  char s6[20] = "Bo1by";
  strcpy(ss1[1], s6);
  strcpy(ss1[0], s6);
  cmp(ss1[0], s3);
  int i1 = -1;
  int i0 = -1;
  for(i=0; i<20; i++){
    if(ss1[0][i]!=ss1[1][i] && i0 == -1 && i1 == -1){
      b = false;
    }
    if(ss1[0][i]=='\0' && i0 == -1)
      i0 = i;
    if(ss1[1][i]=='\0' && i1 == -1)
      i1 = i;
  }
  if(i1!=i0)
    b = false;
  printf("%d %d %d\n", i,c, b);
  printf("%s %s \n", ss1[0], ss1[1]);
  return 0;
}
