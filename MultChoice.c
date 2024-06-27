#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>

typedef struct MC MC;
struct MC {
    uint8_t Amnt, CorrectAnswer;
    char *Question;
    char **Answers;
};

static inline uint16_t
GetStringlen(char* in)
{
  uint16_t i;
    for (i = 0; in[i]; i++);
  return i;
}

static void*
mempcpy2 (void* dst, const void* in, uint64_t len)
{
  size_t i = 0;
  for (; i < len; i++)
    *((char*)dst+i) = *((char*)in+i);
  return dst+i;
}

static void*
append (void **in, uint16_t *crnt, uint16_t *len, size_t size) 
{
  if (!(*in)){
    if (*len < 1)
      *len = 4;
    *crnt = 0;
    *in =  calloc(*len, size);
  } else if (*crnt == *len) {
    *len *= 2;
    void *tmp = (void*) calloc(*len, size);
    if (!tmp)
      return NULL;
    mempcpy2(tmp, *in, size**crnt);

    free(*in);
    *in = tmp;
  }

  uint16_t index = (*crnt)++;
  return *in + index*size;
}

static inline void 
shuffle(MC *array, size_t n)
{
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          MC t;
          t.Amnt = array[j].Amnt;
          t.CorrectAnswer = array[j].CorrectAnswer;
          t.Question = array[j].Question;
          t.Answers = array[j].Answers;

          array[j].Amnt = array[i].Amnt;
          array[j].CorrectAnswer = array[i].CorrectAnswer;
          array[j].Question = array[i].Question;
          array[j].Answers = array[i].Answers;

          array[i].Amnt = t.Amnt;
          array[i].CorrectAnswer = t.CorrectAnswer;
          array[i].Question = t.Question;
          array[i].Answers = t.Answers;
        }
    }
}

static char*
ReadFile(char *file, uint32_t *length)
{
    FILE *f = fopen(file, "r");
    char * buffer = NULL;
    if (f) {
        fseek (f, 0, SEEK_END);
        *length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (*length);
        if (!buffer || !fread (buffer, 1, *length, f)) return NULL;
        fclose (f);
        }
    return buffer;
}

static void*
emalloc(size_t size)
{
  void *res = malloc(size);
  if (!res) {
    puts("malloc fail");
    exit(1);
  }
  return res;
}


static inline MC*
getQuestions (char *fc, const size_t length, uint16_t *nbQuestions)
{
  *nbQuestions = 50;
  MC* Questions = emalloc(sizeof(MC)*(*nbQuestions));
  //MC* Questions = NULL;
  uint16_t index = -1;
  uint16_t nbanswers;
  char ** ans; uint8_t linestart = 1;

  for (size_t i = 0; i < length; i++) {
    switch (fc[i]) {
        case '?':
          if (!linestart) {
            break;
          }
          linestart = 0;
          //append((void**) &Questions, &index, nbQuestions, sizeof(MC));
          index++;
          nbanswers = 0;
          Questions[index].CorrectAnswer = -1;
          Questions[index].Question = &fc[i+1];
          Questions[index].Amnt = 0;
          Questions[index].Answers = (char**) emalloc(sizeof(char**)*8);
          fc[i] = '\0';
          break;
        case '+':
        case '-':
          if (!linestart) {
            break;
          }
          linestart = 0;
          if (fc[i] == '+')
            Questions[index].CorrectAnswer = Questions[index].Amnt;
          //ans = (char**) append((void**) &(Question->Answers), (uint16_t*) &(Question->Amnt), &nbanswers, sizeof(char**));
          //*ans = &(fc[i+1]);
          Questions[index].Answers[Questions[index].Amnt++] = &(fc[i+1]);
          fc[i] = '\0';
          break;
        case '\n':
          if (i == length-1 || fc[i+1] == '?' || fc[i+1] == '+' || fc[i+1] == '-')
            linestart = 1;
          fc[i] = '\0';
          break;
    }
  }
  Questions = (MC*) realloc(Questions, sizeof(MC)*(index+1));
  *nbQuestions = index - 1;
  return Questions;
}

static inline uint8_t
checkResponse(MC *Question, uint8_t response)
{
  if (response != Question->CorrectAnswer){
    printf("\033[38;5;1mInCorrect\n\033[38;5;2mCorrect: %d\033[0m\n\n", Question->CorrectAnswer);
    return 0;
  } else {
    puts("\033[38;5;2mCorrect\033[0m\n\n");
    return 1;
  }
}

static inline void
printAnswers(MC *Question)
{
  for (uint8_t i = 0; i < Question->Amnt; i++) 
    printf("\033[38;5;3m%d: %s\033[0m\n", i, Question->Answers[i]);
}

static inline void
printAnswersRand(MC *Question, uint8_t nbA, uint8_t nbQ, MC *Questions)
{
  for (uint8_t i = 0; i < nbA; i++) {
    if (i == Question->CorrectAnswer) {
      printf("\033[38;5;3m%d: %s\033[0m", i, Question->Answers[Question->CorrectAnswer]);
    } else {
      size_t randQuestion = rand() / (RAND_MAX / (nbQ) + 1);
      size_t randAns = rand() / (RAND_MAX / (Questions[randQuestion].Amnt) + 1);
      printf("\033[38;5;3m%d: %s\033[0m", i, Questions[randQuestion].Answers[randAns]);
    }
  }
}

static inline uint8_t
AskQuestion(MC *Question, uint16_t i, uint8_t nbA, uint8_t nbQ, MC *Questions)
{
  unsigned char Value;
  printf("\033[01;35m%u: \033[38;5;6m%s\033[0m\n", i, Question->Question);
  if (!nbA) {
    printAnswers(Question);
  } else {
    Question->CorrectAnswer = (uint8_t) rand() / (RAND_MAX / (nbA)-1);
    printAnswersRand(Question, nbA, nbQ, Questions);
  }
  while (scanf("%hhd", &Value) != 1)  {
    if (scanf("%c", &Value))// flush
      puts("enter value between 0 and 255");
  }
  return checkResponse(Question, Value);
}

void
help()
{
    puts("\033[38;5;44m-r NUM\n\t\033[0muse NUM answers from random other questions");
    puts("\033[38;5;44m-s\n\t\033[0mshuffle the questions before asking them");
    exit(0);
}

void
setSeed()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand((unsigned int) (tv.tv_sec + tv.tv_usec));
}

void
freeMC(MC *Questions, uint16_t nbQuestions)
{
  for (uint16_t i = 0; i < nbQuestions; i++){
    free(Questions[i].Answers);
  }
  free(Questions);
}

int
main(int argc, char * argv[])
{
  uint8_t cfg = 0;
  uint16_t nbQuestions = 0;
  uint16_t nbCorrect = 0;
  uint8_t i = 0, nbA = 0;
  uint32_t len;

  for (; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 's')
            cfg ^= 1;
        else if (argv[i][0] == '-' && argv[i][1] == 'r')
            nbA = (uint8_t) atoi(argv[++i]);
        else if (argv[i][0] == '-' && argv[i][1] == 'h')
            help();
        else if (argv[i][0] == '-')
            exit(1);
  }

  if (argc == 1)
    return 1;

  char *fc = ReadFile(argv[argc-1], &len);
  MC *Questions = getQuestions(fc, len, &nbQuestions);

  if (cfg & 1) {
    setSeed();
    shuffle(Questions, nbQuestions+1);
  }

  if (!Questions)
    return 1;

  if (nbA)
    setSeed();

  for (uint16_t i = 0; i < nbQuestions+1; i++)
    nbCorrect += AskQuestion(&(Questions[i]), i, nbA, nbQuestions+1, Questions);
  freeMC(Questions, nbQuestions+1);
  free(fc);
    
  printf("\033[0mEndScore: %u/%u\n", nbCorrect, nbQuestions+1);
  return 0;
}
