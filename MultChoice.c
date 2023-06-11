#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct MC MC;
struct MC {
    uint8_t Amnt;
    uint8_t CorrectAnswer;
    char *Question;
    char **Answers;
};

static uint16_t
GetStringlen(char* in)
{
  uint16_t i;
    for (i = 0; in[i]; i++);
  return i;
}

MC*
ReadFile(char *file, uint16_t *nbQuestions)
{
    *nbQuestions = 100;
    MC* Questions = malloc(sizeof(MC)*(*nbQuestions));
    FILE *fd;
    char *Res = NULL;
    size_t len = 0;
    ssize_t nlines;
    fd = fopen(file, "r");
    if (fd == NULL){
      printf("failed to open file");
        return NULL;
    }
  
  uint16_t index = -1;
  uint8_t nbanswers=0;
  /* we can make this faster by removing the copy process and pass a new pointer each time here, 
   * but removing first char would require reading it anyway -> just skip printing first char
   */
  while ((nlines = getline(&Res, &len, fd) != -1)){// 
    //printf("%s", Res);
    uint16_t len = GetStringlen(Res);

    if (Res[0] == '-') {// start incorrect answers with - (45)
      Questions[index].Answers[nbanswers] = malloc(sizeof(char)*len+1);
      //Questions[index].Answers[Questions[index].Amnt]++;
      Questions[index].Amnt++;
      for (uint16_t i=1; Res[i] != 0; i++){
	      Questions[index].Answers[nbanswers][i-1] = Res[i];// don't copy +
      } 
      nbanswers++;
    //printf("IncAnswer: %s\n", Questions[index].Answers[nbanswers]);

    } else if (Res[0] == '+') {// start correct answer with + (43)
      Questions[index].Answers[nbanswers] = malloc(sizeof(char)*len+1);
      //Questions[index].Answers[Questions[index].Amnt]++;
      Questions[index].Amnt++;
      for (uint16_t i=1; Res[i] != 0; i++){
	      Questions[index].Answers[nbanswers][i-1] = Res[i];// don't copy +
	} 
      Questions[index].CorrectAnswer = nbanswers++;
      //printf("correctanswer: %u\n", Questions[index].CorrectAnswer);
        //printf("Answer: %s\n", Questions[index].Answers[nbanswers]);

    } else if(Res[0] == '?') {// start question with ? (63)
      index++;
      if (index)
          if (!(Questions[index-1].Answers = realloc(Questions[index-1].Answers, sizeof(char*)*nbanswers))){
              printf("mallocError\n");
              return NULL;
          }
      nbanswers = 0;
      Questions[index].CorrectAnswer = -1;
      if ( !(Questions[index].Question = malloc(sizeof(char)*len+1))){
          printf("malloc fail\n");
          return NULL;
      }
      if (!Questions[index].Question)
        printf("allocation failed");
      Questions[index].Answers = (char**) malloc(sizeof(char*)*8);// max number of options = 8 -> add check to nbQuestions to increase to inf.
      uint16_t i=1;
      for (; Res[i]; i++){
        Questions[index].Question[i-1] = Res[i];// don't copy ?
      }
      Questions[index].Question[i] = 0;
      //printf("Question: %s\n", Questions[index].Question);
    }
    //printf("AnswerAmount: %u", Questions[index].Amnt);
  }
  fclose(fd);

  if (!(Questions[index].Answers = realloc(Questions[index].Answers, sizeof(char*)*nbanswers))){
      printf("mallocError\n");
      return NULL;
  }
  if (!(Questions = realloc(Questions, sizeof(MC)*(index+1)))){// shrink memory
      printf("mallocError\n");
      return NULL;
  }
  *nbQuestions = index;
  return Questions;
}

uint8_t
AskQuestion(MC *Question, uint16_t i)
{
  char *Value;
  printf("\033[01;35m%u: \033[38;5;6m%s\033[0m", i, Question->Question);
  for (uint8_t i = 0; i < Question->Amnt; i++) {
    printf("\033[38;5;3m%d: %s\033[0m", i, Question->Answers[i]);
  }
  scanf("%ms", &Value);
  if (atoi(Value) != Question->CorrectAnswer){
    printf("\033[38;5;1mInCorrect\n\033[38;5;2mCorrect: %d\033[0m\n\n", Question->CorrectAnswer);
    return 0;
  } else {
    printf("\033[38;5;2mCorrect\033[0m\n\n");
    return 1;
  }
}

int
main(int argc, char * argv[])
{
  if (argc != 2)
    return 1;
  uint16_t nbQuestions = 0;
  MC * Questions = ReadFile(argv[1], &nbQuestions);
  if (!Questions)
    return 1;
  uint16_t nbCorrect = 0;
  for (uint16_t i = 0; i < nbQuestions+1; i++){
    nbCorrect += AskQuestion(&(Questions[i]), i);
  }
  printf("\033[0mEndScore: %u/%u\n", nbCorrect, nbQuestions);
  return 0;
  
}
