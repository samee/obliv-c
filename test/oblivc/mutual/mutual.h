//#prama once
//the maximum length of a name
#define MAXL 10
//the maximum number of friends
#define MAXN 60

typedef char* string;
typedef struct protocolIO{
  char mine[MAXN][MAXL];
  int size;
  char common[MAXN][MAXL];
  int commonSize;
}protocolIO;

const char* mySide();

void mutualFriends(void* args);

void sortMutual(void* args);
