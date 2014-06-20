//mutualfriends file

//#prama once
//the maximum length of a name
#define MAXL 11
//the maximum number of friends
#define MAXN 512

typedef char* string;
typedef struct protocolIO{
  char mine[MAXN][MAXL];
  int size;
  char common[2*MAXN][MAXL];
  int commonSize;
}protocolIO;

const char* mySide();

void mutualFriends(void* args);

void sortMutual(void* args);
