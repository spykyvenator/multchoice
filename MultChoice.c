#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>// same size as typedeffing here

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

static FILE*
ReadFile(char *file)
{
    FILE *fd;
    fd = fopen(file, "r");
    if (fd == NULL){
      printf("failed to open file");
        return NULL;
    } else 
      return fd;
}

static inline MC*
getQuestions (FILE* fd, uint16_t *nbQuestions)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t nlines;
  *nbQuestions = 300;
  MC* Questions = malloc(sizeof(MC)*(*nbQuestions));
  
  uint16_t index = -1;
  uint8_t nbanswers=0;
  /* we can make this faster by removing the copy process and pass a new pointer each time here, 
   * but removing first char would require reading it anyway -> just skip printing first char
   */
  while ((nlines = getline(&line, &len, fd) != -1)){// 
    uint16_t len = GetStringlen(line);

    if (line[0] == '-') {// start incorrect answers with - (45)
      Questions[index].Answers[nbanswers] = malloc(sizeof(char)*len+1);
      //Questions[index].Answers[Questions[index].Amnt]++;
      Questions[index].Amnt++;
      for (uint16_t i=1; line[i] != 0; i++){
	      Questions[index].Answers[nbanswers][i-1] = line[i];// don't copy +
      } 
      nbanswers++;

    } else if (line[0] == '+') {// start correct answer with + (43)
      Questions[index].Answers[nbanswers] = malloc(sizeof(char)*len+1);
      //Questions[index].Answers[Questions[index].Amnt]++;
      Questions[index].Amnt++;
      for (uint16_t i=1; line[i] != 0; i++){
	      Questions[index].Answers[nbanswers][i-1] = line[i];// don't copy +
	} 
      Questions[index].CorrectAnswer = nbanswers++;

    } else if(line[0] == '?') {// start question with ? (63)
      index++;
      if (index)
          if (!(Questions[index-1].Answers = realloc(Questions[index-1].Answers, sizeof(char*)*nbanswers))){
              puts("mallocError");
              return NULL;
          }
      nbanswers = 0;
      Questions[index].CorrectAnswer = -1;
      if ( !(Questions[index].Question = malloc(sizeof(char)*len+1))){
          puts("malloc fail");
          return NULL;
      }
      if (!Questions[index].Question)
        puts("allocation failed");
      Questions[index].Answers = (char**) malloc(sizeof(char*)*8);// max number of options = 8 -> add check to nbQuestions to increase to inf.
      uint16_t i=1;
      for (; line[i]; i++){
        Questions[index].Question[i-1] = line[i];// don't copy ?
      }
      Questions[index].Question[i] = 0;
    }
    free(line);
    line = NULL;
  }
  fclose(fd);

  if (!(Questions[index].Answers = realloc(Questions[index].Answers, sizeof(char*)*nbanswers))){
      puts("mallocError\n");
      return NULL;
  }

  if (!(Questions = realloc(Questions, sizeof(MC)*(index+1)))){// shrink memory
      puts("mallocError\n");
      return NULL;
  }

  *nbQuestions = index;
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
    printf("\033[38;5;3m%d: %s\033[0m", i, Question->Answers[i]);
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
  printf("\033[01;35m%u: \033[38;5;6m%s\033[0m", i, Question->Question);
  if (!nbA) {
    printAnswers(Question);
  } else {
    Question->CorrectAnswer = (uint8_t) rand() / (RAND_MAX / (nbA) + 1);
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

int
main(int argc, char * argv[])
{
  uint8_t cfg = 0;
  uint16_t nbQuestions = 0;
  uint16_t nbCorrect = 0;
  uint8_t i, nbA = 0;
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

  FILE *fd = ReadFile(argv[argc-1]);
  MC *Questions = getQuestions(fd, &nbQuestions);

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
    
  printf("\033[0mEndScore: %u/%u\n", nbCorrect, nbQuestions+1);
  return 0;
  
}
