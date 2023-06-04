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
    if (fd == NULL)
        return NULL;
  
  uint16_t index = -1;
  uint8_t nbanswers=0;
  while ((nlines = getline(&Res, &len, fd) != -1)){
    uint16_t len = GetStringlen(Res);

    if (Res[0] == 45) {// start incorrect answers with -
      Questions[index].Answers[Questions[index].Amnt]++;
      for (unsigned char i=0; Res[i+1] != 0; i++){
	      Questions[index].Answers[nbanswers][i+1] = Res[i];// don't copy +
      } 
      nbanswers++;
    } else if (Res[0] == 43) {// start correct answer with +
      Questions[index].Answers[Questions[index].Amnt]++;
      for (unsigned char i=0; Res[i+1] != 0; i++){
	      Questions[index].Answers[nbanswers][i+1] = Res[i];// don't copy +
	} 
      Questions[index].CorrectAnswer = nbanswers++;
    } else if(Res[0] == 63) {// start question with ?
      index++;
      nbanswers = 0;
      Questions[index].Question = malloc(sizeof(char)*len+1);
      Questions[index].Answers = (char**) malloc(sizeof(char*)*4);
      for (unsigned char i=0; Res[i+1] != 0; i++){
	      Questions[index].Question[i+1] = Res[i];// don't copy ?
      }
    }
  }
  fclose(fd);
  *nbQuestions = index;
  return Questions;
}

void
AskQuestion(MC *Question)
{
  uint8_t *Value;
  printf("%s\n", Question->Question);
  for (uint8_t i = 0; i < Question->Amnt; i++) {
    printf("%d: %s\n", i, Question->Answers[i]);
  }
  scanf("%ms", &Value);
  if (*Value != Question->CorrectAnswer)
    printf("InCorrect\n");
  else 
    printf("Correct\n");
  return;
}

int
main(int argc, char * argv[])
{
  uint16_t nbQuestions = 0;
  MC * Questions = ReadFile(argv[1], &nbQuestions);
  for (uint16_t i = 0; i < nbQuestions; i++)
    AskQuestion(&(Questions[i]));
  return 0;
  
}
