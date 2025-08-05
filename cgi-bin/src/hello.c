#include <stdio.h>


int main(int ac, char **av, char **envp)
{
    printf("content-lenght: text/plain\n\n");

    (void)ac;
    (void)av;
    int i = 0;
    printf("Environment Variables : \n");
    while(envp[i])
    {
        printf("%d :: %s\n",i, envp[i]);
        i++;
    }
    return 0;

}