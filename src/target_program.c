#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t process_id = getpid();
    printf("Current Process ID: %d\n", process_id);
    fflush(stdout);

    volatile int health = 100;

    while (1) {
        printf("\nHealth value: %d\nHealth memory address: %p\n",
               health, (void *)&health);
        printf("Enter new health value: ");
        fflush(stdout);

        if (scanf("%d", (int *)&health) != 1) break;
    }

    return 0;
}